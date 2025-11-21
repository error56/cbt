#include "IncInstructionEmulator.h"

#include "../../../../execution/ExecutionThread.h"

bool IncInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto operands = decoded_instruction.operands;

  const uint64_t dst =
      ReadOperand(thread->GetGuestRegisters(), operands[0]);
  const uint64_t result = dst + 1;

  WriteOperand(thread->GetGuestRegisters(), operands[0], result);
  UpdateFlags(thread->GetGuestRegisters(), dst, 1, result,
              operands[0].size, kInc);

  return true;
}