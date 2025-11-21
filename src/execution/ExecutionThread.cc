#include "ExecutionThread.h"

#include <asm/prctl.h>
#include <dirent.h>

#include <cstddef>
#include <random>

#include "EmulatorMap.h"
#include "StackInitializer.h"
#include "zasm/program/program.hpp"
#include "zasm/serialization/serializer.hpp"
#include "zasm/x86/assembler.hpp"

#undef LOG_ASM_INSTRUCTIONS
#undef IDA_PLUGIN_LOGGING

#ifdef LOG_BB_STATS
static uint64_t kUniqueBasicBlockCount = 0;
static uint64_t kUniqueMergedBasicBlockCount = 0;
static uint64_t kMergedBasicBlockExecutionCount = 0;
static uint64_t kBasicBlockCacheLookupCount = 0;
static uint64_t kBasicBlockCacheHitCount = 0;
static uint64_t kBasicBlockCacheMissCount = 0;
#endif

ExecutionThread::ExecutionThread(
    std::shared_ptr<ElfLoader> binary, uint64_t starting_instruction_address,
    const std::shared_ptr<HighLevelEmulationContext>& hlec,
    const std::shared_ptr<TranslationContext>& translation_context)
    : binary_(std::move(binary)),
      generator_(processor_state_),
      current_instruction_address_(starting_instruction_address),
      hlec_(hlec),
      translation_context_(translation_context) {
  for (const auto& [opcode, ctr] : kInstructionEmulatorMap) {
    instruction_emulators_.insert(
        std::make_pair(opcode, ctr(processor_state_.guest_state)));
  }

  for (const auto& [syscall, ctr] : kSyscallEmulatorMap) {
    syscall_emulators_.insert(std::make_pair(syscall, ctr(hlec_)));
  }
}

void ExecutionThread::Run() {
  SetupContextSwitchStub();
  if (!InitializeThreadContext()) {
    spdlog::error("Cannot initialize thread context");

    return;
  }

  if (current_instruction_address_ == 0) {
    current_instruction_address_ =
        binary_->GetBinaryBase() + binary_->GetEntryPoint();
  }

  while (true) {
    if (should_exit_) {
      break;
    }

    const auto cache_entry = GetTranslationCacheEntryFromCurrentBasicBlock();
    if (cache_entry->is_trivial && cache_entry->is_not_empty) {
      if (!EmulateTrivialBasicBlock(
              *cache_entry->instructions->GetInstructions())) {
        // @TODO: Technically, if error during emulation occurs, we can use old
        // way of context switch and emulating it as normal block, but need to
        // remember about modified state by emulated instructions till the
        // invalid one.

        spdlog::error("EmulateTrivialBasicBlock failed");
        return;
      }
    } else {
      // Not trivial

      // Try to merge if possible
      // It modifies cache_entry directly, so if merging is successful, we will
      // execute modified merged block.
      if (!cache_entry->merging_info.merging_performed &&
          cache_entry->merging_info.merging_available &&
          cache_entry->is_not_empty) {
        if (TryToMergeBasicBlock(cache_entry)) {
          // spdlog::info("A");
        }
      }

      // If basic block is not empty, then just execute it on the real processor
      if (cache_entry->is_not_empty) {
        // Context switch to guest code

        // Push address of prepared stub onto the guest stack (it will be used
        // by ret instruction)
        PushToGuestStack(
            reinterpret_cast<uint64_t>(cache_entry->context_switch.code));

        // Perform FS/GS segment switch if basic block uses access to these
        // registers. It is done by performing two syscalls, which is very
        // expensive in terms of CPU utilization, so we don't want to do this
        // every time we perform a context switch to the guest code.
        if (cache_entry->requires_segment_switch) {
          this->PerformSegmentSwitch(true);
        }

        // Run guest code
        reinterpret_cast<ContextSwitchFunction>(
            generator_.context_switch_from_translator_to_guest)();

        // We've returned from guest code now, so do a quick cleanup and emulate
        // instruction that broke basic block.
        if (cache_entry->requires_segment_switch) {
          this->PerformSegmentSwitch(false);
        }
      }
    }

    auto& last_instruction = cache_entry->last_instruction;
    // Handle instruction that broke basic block.
    if (!instruction_emulators_[last_instruction.instruction.mnemonic]
             .get()
             ->EmulateInstruction(current_instruction_address_,
                                  last_instruction, this, call_stack_)) {
      spdlog::error("EmulateInstruction failed");

      return;
    }
  }
}

uint64_t ExecutionThread::GetJumpTarget(
    const DecodedInstruction& instruction) const noexcept {
  uint64_t target = 0;

  if (instruction.operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
    target = current_instruction_address_ + instruction.operands[0].imm.value.s;
  } else if (instruction.operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY) {
    // [rip + offset]
    assert(instruction.operands[0].mem.base == ZYDIS_REGISTER_RIP);

    const auto addr =
        current_instruction_address_ + instruction.operands[0].mem.disp.value;
    const auto value = *reinterpret_cast<uint64_t*>(addr);
    target = value;
  } else if (instruction.operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
    target = InstructionEmulator::GetRegisterValue(
        processor_state_.guest_state, instruction.operands[0].reg.value);
  }

  return target;
}

bool ExecutionThread::TryToMergeBasicBlock(
    const std::shared_ptr<TranslationCacheEntry>& entry) noexcept {
  assert(entry->last_instruction.instruction.mnemonic == ZYDIS_MNEMONIC_JMP);

  // If JMP and not merged

  // Add jump to the end
  const auto jump_target = GetJumpTarget(entry->last_instruction);

  // If next basic block is not translated yet, give up.
  //
  // @TODO: Recursive translation?
  if (!translation_context_->translation_cache->contains(jump_target)) {
    return false;
  }

  // If next basic block is translated, change JMP instruction
  const auto next_cache_entry =
      translation_context_->translation_cache->at(jump_target);

  auto modification_pointer =
      reinterpret_cast<uint64_t>(entry->merging_info.jump_address);
  // Set opcode after the last valid instruction to relative jmp 32
  *reinterpret_cast<uint8_t*>(modification_pointer) =
      kRelativeJumpOpcode;  // rel jmp 32
  modification_pointer++;

  // After opcode write the offset of the starting instruction of the next basic
  // block
  *reinterpret_cast<uint32_t*>(modification_pointer) =
      reinterpret_cast<uint64_t>(next_cache_entry->context_switch.code) -
      (modification_pointer + 4);

  // Mark it as fixed
  entry->merging_info.merging_performed = true;
  entry->instruction_address_at_the_end =
      next_cache_entry->instruction_address_at_the_end;
  entry->last_instruction = next_cache_entry->last_instruction;
  current_instruction_address_ =
      next_cache_entry->instruction_address_at_the_end;

  // If the next basic block uses fs/gs segments, make sure that previous block
  // will switch segments
  if (next_cache_entry->requires_segment_switch) {
    entry->requires_segment_switch = true;
  }

  return true;
}

std::shared_ptr<TranslationCacheEntry>
ExecutionThread::GetTranslationCacheEntryFromCurrentBasicBlock() noexcept {
  if (translation_context_->translation_cache->contains(
          current_instruction_address_)) {
    // If basic block was previously translated by translator and is
    // currently in the translation cache, then derive all needed values from
    // the TranslationCacheEntry.

    const auto entry = translation_context_->translation_cache->at(
        current_instruction_address_);

    current_instruction_address_ = entry->instruction_address_at_the_end;

    return entry;
  }

  // If the basic block is encountered for the first time, extract the
  // instructions allocate context switch and insert the data to the
  // translation cache.
  const auto basic_block_start = current_instruction_address_;
  auto basic_block = decoder_.ExtractBasicBlock(current_instruction_address_);
  const auto basic_block_size =
      current_instruction_address_ - basic_block_start;

  const auto context_switch = basic_block->AllocateContextSwitch(
      translation_context_->basic_block_allocator, context_switch_stub_);
  const auto merging_available =
      ZYDIS_MNEMONIC_JMP == basic_block->LastInstruction().instruction.mnemonic;

  const auto entry =
      std::make_shared<TranslationCacheEntry>(TranslationCacheEntry{
          .context_switch = context_switch,
          .basic_block_size = basic_block_size,
          .requires_segment_switch = basic_block->RequiresSegmentSwitch(),
          .last_instruction = basic_block->LastInstruction(),
          .is_not_empty = !basic_block->IsEmpty(),
          .is_trivial = basic_block->IsTrivial(),
          .instructions = basic_block,
          .merging_info =
              MergingInfo{
                  .merging_available = merging_available,
                  .merging_performed = false,
                  .instruction_address_at_the_end =
                      current_instruction_address_,
                  .jump_address = context_switch.last_instruction_address},
          .instruction_address_at_the_end = current_instruction_address_});

  translation_context_->translation_cache->insert(
      std::make_pair(basic_block_start, entry));

  return entry;
}

bool ExecutionThread::InitializeThreadContext() noexcept {
  // Allocate some stack memory
  try {
    stack_ = new uint8_t[kStackSize];
  } catch (std::bad_alloc&) {
    return false;
  }

  // Set RSP to (end of the stack - one qword)
  processor_state_.guest_state.rsp = reinterpret_cast<uint64_t>(
      static_cast<uint8_t*>(stack_) + kStackSize - sizeof(uint64_t));

  // Set reserved bits
  processor_state_.guest_state.rflags = kRflagsCpl3 | kRflagsReserved;

  // rdi argc, rsi argv, rdx envp

  const auto stack = StackInitializer(hlec_->args, hlec_->env);
  stack.Initialize(processor_state_.guest_state, binary_->GetMappedFileBase());

  return true;
}

template <typename T>
  requires Numeric<T>
void ExecutionThread::PushToGuestStack(T value) noexcept {
  // Make sure we don't get over allocated memory
  assert(processor_state_.guest_state.rsp - sizeof(T) >
         reinterpret_cast<uint64_t>(stack_));

  // Create some space for value
  processor_state_.guest_state.rsp -= sizeof(T);

  // Write the value onto the stack
  *reinterpret_cast<T*>(processor_state_.guest_state.rsp) = value;
}

void ExecutionThread::PopFromGuestStack(const uint8_t size) noexcept {
  assert(size == 1 || size == 2 || size == 4 || size == 8);

  processor_state_.guest_state.rsp += size;
}

ProcessorRegisters& ExecutionThread::GetGuestRegisters() noexcept {
  return processor_state_.guest_state;
}

ProcessorState& ExecutionThread::GetProcessorState() noexcept {
  return processor_state_;
}

__attribute__((always_inline)) void ExecutionThread::PerformSegmentSwitch(
    const bool entry_to_emulation) noexcept {
  if (entry_to_emulation) {
    uint64_t emulator_fs = 0;
    uint64_t emulator_gs = 0;

    syscall(SYS_arch_prctl, ARCH_GET_FS, &emulator_fs);
    syscall(SYS_arch_prctl, ARCH_GET_GS, &emulator_gs);

    processor_state_.emulator_state.fs = emulator_fs;
    processor_state_.emulator_state.gs = emulator_gs;

    syscall(SYS_arch_prctl, ARCH_SET_FS, processor_state_.guest_state.fs);
    syscall(SYS_arch_prctl, ARCH_SET_GS, processor_state_.guest_state.gs);
  } else {
    uint64_t guest_fs = 0;
    uint64_t guest_gs = 0;

    syscall(SYS_arch_prctl, ARCH_GET_FS, &guest_fs);
    syscall(SYS_arch_prctl, ARCH_GET_GS, &guest_gs);

    processor_state_.guest_state.fs = guest_fs;
    processor_state_.guest_state.gs = guest_gs;

    syscall(SYS_arch_prctl, ARCH_SET_FS, processor_state_.emulator_state.fs);
    syscall(SYS_arch_prctl, ARCH_SET_GS, processor_state_.emulator_state.gs);
  }
}

[[nodiscard]] bool ExecutionThread::EmulateTrivialBasicBlock(
    std::vector<DecodedInstruction>& instructions) noexcept {
  for (auto& instr : instructions) {
    auto& decoded = instr.instruction;

    // We don't have instruction emulator for this instruction, which means
    // it's either instruction breaking basic block (and we need to return
    // from this function to the main translator loop) or something went
    // wrong, and we should fall back into normal emulation.
    if (std::ranges::find(kExitInstructions, decoded.mnemonic) !=
        kExitInstructions.end()) {
      return true;
    }
    // spdlog::warn("Emulating {}...",
    // ZydisMnemonicGetString(decoded.mnemonic));

    if (!instruction_emulators_.contains(decoded.mnemonic)) {
      spdlog::error("Cant emulate trivial basic block");

      // @todo: return false should fall back into the normal emulation in
      // case of weird opcode, not known instruction or combination of flags
      // we dont emulate yet
      return false;
    }

    // Emulate trivial instruction
    if (!instruction_emulators_[decoded.mnemonic].get()->EmulateInstruction(
            current_instruction_address_, instr, this, call_stack_)) {
      spdlog::error("EmulateInstruction failed");

      // Fall back?
      return false;
    }
  }

  return true;
}

InterceptionResult ExecutionThread::HandleSyscall(
    const UnixSyscall syscall) noexcept {
  if (!syscall_emulators_.contains(syscall)) {
    spdlog::error("Can't find emulator for syscall {}",
                  syscall_name_lookup_table[syscall]);
    return kPassDownTheStack;
  }

  return syscall_emulators_.at(syscall)->EmulateSyscall(this);
}

void ExecutionThread::PerformSyscall() noexcept {
  constexpr uint8_t kSyscallOpcode[2] = {0x0f, 0x05};

  ZydisDecoder decoder;
  ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
  ZydisDecodedInstruction instruction;
  ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
  ZydisDecoderDecodeFull(&decoder, kSyscallOpcode, 4, &instruction, operands);

  auto instructions = std::make_shared<std::vector<DecodedInstruction>>(
      std::vector{DecodedInstruction{.instruction = instruction,
                                     .operands = {},
                                     .address = 0,
                                     .size = 0},
                  DecodedInstruction{.instruction = instruction,
                                     .operands = {},
                                     .address = 0,
                                     .size = 0}});

  const auto basic_block = std::make_shared<BasicBlock>(instructions);

  // Context switch to guest code
  //
  // Create context switch stub
  const auto func = basic_block->AllocateContextSwitch(
      translation_context_->basic_block_allocator, context_switch_stub_);

  // Push address of prepared stub onto the guest stack (it will be used by
  // ret instruction)
  PushToGuestStack(reinterpret_cast<uint64_t>(func.code));

  // Run guest code
  reinterpret_cast<ContextSwitchFunction>(
      generator_.context_switch_from_translator_to_guest)();
}

void ExecutionThread::Stop() noexcept { should_exit_ = true; }

void ExecutionThread::SetupContextSwitchStub() noexcept {
  // Create and assemble instructions allowing us to get back to the translator
  // after executing basic block.
  zasm::Program program(zasm::MachineMode::AMD64);
  zasm::x86::Assembler assembler(program);

  // Push RAX to make some space for address to jump to; RAX value will be
  // popped off stack by context switching function.
  assembler.push(zasm::x86::rax);
  // Move address of context switching function to RAX
  assembler.mov(zasm::x86::rax,
                zasm::Imm(reinterpret_cast<uint64_t>(
                    this->generator_.context_switch_from_guest_to_translator)));
  // Jump
  assembler.jmp(zasm::x86::rax);

  zasm::Serializer serializer{};
  // Base is not really relevant because we don't use any relative operands
  serializer.serialize(program, 0x0);

  context_switch_stub_ = std::move(serializer);
}
