#include "MovsxdInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"

bool MovsxdInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto operands = decoded_instruction.operands;

  const uint64_t src =
      ReadOperand(thread->GetGuestRegisters(), operands[1]);
  const auto size = operands[1].size;
  auto result = 0;

  if (size == 8) {
    result =
        static_cast<uint64_t>(static_cast<int64_t>(static_cast<int8_t>(src)));
  } else if (size == 16) {
    result =
        static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(src)));
  } else if (size == 32) {
    result =
        static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(src)));
  }

  WriteOperand(thread->GetGuestRegisters(), operands[0], result);

  return true;
}
