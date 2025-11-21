#include "ShrInstructionEmulator.h"

#include "../../../../execution/ExecutionThread.h"

bool ShrInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto operands = decoded_instruction.operands;

  const uint64_t dst =
      ReadOperand(thread->GetGuestRegisters(), operands[0]);
  const uint64_t count =
      ReadOperand(thread->GetGuestRegisters(), operands[1]) & 0x3F;
  const uint64_t result = dst >> count;

  WriteOperand(thread->GetGuestRegisters(), operands[0], result);

  if (result > 0) {
    UpdateFlags(thread->GetGuestRegisters(), dst, count, result,
                operands[0].size, kShr);
  }

  return true;
}
