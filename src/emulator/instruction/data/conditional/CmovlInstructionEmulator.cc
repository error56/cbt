#include "CmovlInstructionEmulator.h"

#include "../../../../execution/ExecutionThread.h"

bool CmovlInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto rflags = thread->GetGuestRegisters().rflags;

  const auto sign_flag = (rflags & kRflagsSignFlagMask) == 1;
  const auto overflow_flag = (rflags & kRflagsOverflowFlagMask) == 1;

  if (sign_flag != overflow_flag) {
    const auto operands = decoded_instruction.operands;
    const auto src =
        ReadOperand(thread->GetGuestRegisters(), operands[1]);

    WriteOperand(thread->GetGuestRegisters(), operands[0], src);
  }

  return true;
}
