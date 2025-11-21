#include "MovInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"

bool MovInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto operands = decoded_instruction.operands;

  const uint64_t src =
      ReadOperand(thread->GetGuestRegisters(), operands[1]);
  WriteOperand(thread->GetGuestRegisters(), operands[0], src);

  return true;
}
