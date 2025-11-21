#include "CallInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"

bool CallInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto address_before_call = current_instruction_address;

  // Handle every operand type separately
  if (decoded_instruction.operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
    current_instruction_address += decoded_instruction.operands[0].imm.value.s;

    // Push address of next instruction to the call stack
    call_stack.push(std::make_pair(address_before_call, current_instruction_address));
  } else if (decoded_instruction.operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY) {

    if (decoded_instruction.operands[0].mem.base == ZYDIS_REGISTER_RBX) {
      const auto addr = thread->GetGuestRegisters().rbx + decoded_instruction.operands[0].mem.disp.value;
      const auto value = *reinterpret_cast<uint64_t*>(addr);
      current_instruction_address = value;
    } else if (decoded_instruction.operands[0].mem.base == ZYDIS_REGISTER_RBP) {
      const auto addr = thread->GetGuestRegisters().rbp + decoded_instruction.operands[0].mem.disp.value;
      const auto value = *reinterpret_cast<uint64_t*>(addr);
      current_instruction_address = value;
    } else if (decoded_instruction.operands[0].mem.base == ZYDIS_REGISTER_R12) {
      const auto addr = thread->GetGuestRegisters().r12 + decoded_instruction.operands[0].mem.disp.value;
      const auto value = *reinterpret_cast<uint64_t*>(addr);
      current_instruction_address = value;
    } else {
      assert(decoded_instruction.operands[0].mem.base == ZYDIS_REGISTER_RIP);

      const auto addr = current_instruction_address +
                        decoded_instruction.operands[0].mem.disp.value;
      const auto value = *reinterpret_cast<uint64_t*>(addr);
      current_instruction_address = value;
    }

    // Push address of next instruction to the call stack
    call_stack.push(std::make_pair(address_before_call, current_instruction_address));
  } else if (decoded_instruction.operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
    assert(decoded_instruction.operands[0].reg.value == ZYDIS_REGISTER_RBX);

    current_instruction_address = thread->GetGuestRegisters().rbx;

    call_stack.push(std::make_pair(address_before_call, current_instruction_address));
  } else {
    spdlog::error("Invalid operand type for instruction.");

    return false;
  }

  constexpr uint64_t kCallSignature = 0x54534E494C4C4143;
  thread->PushToGuestStack(kCallSignature);

  return true;
}
