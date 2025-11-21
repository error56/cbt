#include "JzInstructionEmulator.h"

#include "../../../../execution/ExecutionThread.h"

bool JzInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  // Check if condition is met and we follow the jump or skip the instruction
  if ((cpu_registers.rflags & kRflagsZeroFlagMask) == 0) {
    // Condition not met, skip instruction.

    return true;
  }

  // Follow the jump

  // Handle every operand type separately
  if (decoded_instruction.operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
    current_instruction_address += decoded_instruction.operands[0].imm.value.s;
  } else {
    spdlog::error("Invalid operand type for instruction.");

    return false;
  }

  return true;
}
