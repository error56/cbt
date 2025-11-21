#include "BasicBlock.h"

#include <ranges>

#include "spdlog/spdlog.h"

bool BasicBlock::IsTrivial() const noexcept {
  for (const auto& instr : *instructions_) {
    const auto& decoded = instr.instruction;

    // Trivial basic block can NOT modify memory in any way
    for (uint8_t i = 0; i < decoded.operand_count; ++i) {
      const auto& op = instr.operands[i];
      if (op.type == ZYDIS_OPERAND_TYPE_MEMORY) {
        if (((op.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) != 0) ||
            op.mem.base == ZYDIS_REGISTER_RIP) {
          return false;
        }
      }
    }

    // We can't emulate instructions that change processor mode or are
    // privileged.
    if ((decoded.attributes & ZYDIS_ATTRIB_IS_PRIVILEGED) != 0) {
      return false;
    }

    // Instructions can operate only on general purpose registers
    for (uint8_t i = 0; i < decoded.operand_count_visible; ++i) {
      const auto& op = instr.operands[i];
      if ((op.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) != 0) {
        if (op.type == ZYDIS_OPERAND_TYPE_REGISTER) {
          const auto reg = op.reg.value;

          // Can't modify special registers directly (like RIP or FLAGS)
          if (ZydisRegisterGetClass(reg) == ZYDIS_REGCLASS_FLAGS ||
              ZydisRegisterGetClass(reg) == ZYDIS_REGCLASS_IP ||
              ZydisRegisterGetClass(reg) == ZYDIS_REGCLASS_SEGMENT ||
              ZydisRegisterGetClass(reg) == ZYDIS_REGCLASS_CONTROL ||
              ZydisRegisterGetClass(reg) == ZYDIS_REGCLASS_DEBUG ||
              ZydisRegisterGetClass(reg) == ZYDIS_REGCLASS_TEST ||
              ZydisRegisterGetClass(reg) == ZYDIS_REGCLASS_BOUND) {
            return false;
          }
        }
      }
    }

    // Block various mnemonics by name
    if (kBlockedMnemonicsInTrivialBlock.test(
            static_cast<size_t>(decoded.mnemonic))) {
      return false;
    }

    // Atomic operations are not supported yet.
    if ((decoded.attributes & ZYDIS_ATTRIB_HAS_LOCK) != 0) {
      return false;
    }

    // Instructions with REP prefix are not supported yet.
    if ((decoded.attributes & (ZYDIS_ATTRIB_HAS_REP | ZYDIS_ATTRIB_HAS_REPE |
                               ZYDIS_ATTRIB_HAS_REPNE)) != 0) {
      return false;
    }
  }

  return true;
}

bool BasicBlock::RequiresSegmentSwitch() const noexcept {
  for (const auto& instruction : *instructions_) {
    for (auto i = 0; i < instruction.instruction.operand_count; i++) {
      const auto& operand = instruction.operands[i];

      if (operand.reg.value == ZYDIS_REGISTER_FS ||
          operand.reg.value == ZYDIS_REGISTER_GS) {
        return true;
      }

      if (operand.ptr.segment == ZYDIS_REGISTER_FS ||
          operand.ptr.segment == ZYDIS_REGISTER_GS) {
        return true;
      }

      if (operand.mem.segment == ZYDIS_REGISTER_FS ||
          operand.mem.segment == ZYDIS_REGISTER_GS ||
          operand.mem.base == ZYDIS_REGISTER_FS ||
          operand.mem.base == ZYDIS_REGISTER_GS) {
        return true;
      }
    }
  }

  return false;
}

bool BasicBlock::IsEmpty() const noexcept { return instructions_->size() < 2; }

DecodedInstruction& BasicBlock::LastInstruction() const noexcept {
  return instructions_->at(instructions_->size() - 1);
}

ContextSwitchStubInfo BasicBlock::AllocateContextSwitch(
    const std::shared_ptr<BasicBlockAllocator>& allocator,
    const zasm::Serializer& context_switch_stub) const noexcept {
  const auto allocation_size =
      (instructions_->size() * ZYDIS_MAX_INSTRUCTION_LENGTH) +
      (kBasicBlockAdditionalInstructionCount * ZYDIS_MAX_INSTRUCTION_LENGTH);

  auto* memory =
      allocator->AllocateMemoryForTranslatedBasicBlock(allocation_size);
  size_t offset = 0;

  // @TODO: Currently we perform instruction encoding for all instructions,
  //        even not changed ones - can be optimized.
  //
  // Basic block vector contains all instructions in basic block; we don't
  // want to execute last instruction (such as jmp or call), but emulate them,
  // so need to iterate over view of vector without last element
  auto instructions_to_execute =
      *instructions_ | std::views::take(instructions_->size() - 1);
  for (const auto& [instruction, operands, original_instruction_address, size] :
       instructions_to_execute) {
    ZydisEncoderRequest request;
    ZydisEncoderDecodedInstructionToEncoderRequest(
        &instruction, operands.data(), instruction.operand_count_visible,
        &request);

    // Fix operands
    const auto translated_instruction_address =
        reinterpret_cast<uint64_t>(static_cast<uint8_t*>(memory) + offset);
    if (!FixOperands(request, original_instruction_address,
                     translated_instruction_address)) {
      // If we can't fix the operands, consider it as a panic and critical
      // failure; if we execute the program without fixed references to the
      // memory it can break emulator state, or even achieve RCE.
      spdlog::error("Cannot fix operands at {}", original_instruction_address);

      exit(1);
    }

    std::array<uint8_t, ZYDIS_MAX_INSTRUCTION_LENGTH> encoded_instruction{};
    ZyanUSize encoded_length = encoded_instruction.size();

    if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(
            &request, encoded_instruction.data(), &encoded_length))) {
      spdlog::error("Cannot encode instruction");

      exit(2);
    }

    // Copy encoded instruction into the memory
    std::memcpy(static_cast<uint8_t*>(memory) + offset, &encoded_instruction,
                encoded_length);

    offset += encoded_length;
  }

  const auto stub = ContextSwitchStubInfo{
      .code = reinterpret_cast<ContextSwitchFunction>(memory),
      .last_instruction_address = static_cast<uint8_t*>(memory) + offset};

  std::memcpy(static_cast<uint8_t*>(memory) + offset,
              context_switch_stub.getCode(), context_switch_stub.getCodeSize());

  return stub;
}

bool BasicBlock::FixOperands(
    ZydisEncoderRequest& request, const uint64_t original_instruction_address,
    const uint64_t translated_instruction_address) noexcept {
  for (int i = 0; i < request.operand_count; i++) {
    auto& operand = request.operands[i];

    if (operand.type == ZYDIS_OPERAND_TYPE_MEMORY &&
        operand.mem.base == ZYDIS_REGISTER_RIP) {
      // [rip + something]

      // Relative addressing in x86 works with use of base, index, scale and
      // displacement.
      //
      // Effective address can be calculated using the formula:
      // effective_address = base + (index * scale) + displacement
      const auto mem = operand.mem;
      const auto effective_address =
          static_cast<ZyanI64>(mem.index * mem.scale) + mem.displacement;

      const auto reference_address_in_binary =
          original_instruction_address + effective_address;
      const auto difference =
          static_cast<int64_t>(reference_address_in_binary) -
          static_cast<int64_t>(translated_instruction_address);

      operand.mem.base = ZYDIS_REGISTER_RIP;
      operand.mem.index = ZYDIS_REGISTER_NONE;
      operand.mem.scale = 0;
      operand.mem.displacement = difference;
    }
  }

  return true;
}

std::shared_ptr<std::vector<DecodedInstruction>> BasicBlock::GetInstructions()
    const noexcept {
  return instructions_;
}
