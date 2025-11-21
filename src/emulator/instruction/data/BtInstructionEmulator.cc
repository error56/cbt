#include "BtInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"

bool BtInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto& operands = decoded_instruction.operands;

  const uint64_t base = ReadOperand(thread->GetGuestRegisters(), operands[0]);
  const uint64_t bit_offset = ReadOperand(thread->GetGuestRegisters(), operands[1]);

  const auto size = operands[0].size;
  const uint64_t bit_position = bit_offset % size;

  const auto bit_value = (base >> bit_position) & 1;

  auto& rflags = thread->GetGuestRegisters().rflags;

  if (bit_value) {
    rflags |= kRflagsCarryFlagMask;
  } else {
    rflags &= ~kRflagsCarryFlagMask;
  }

  return true;
}
