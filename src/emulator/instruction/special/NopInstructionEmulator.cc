#include "NopInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"

bool NopInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  // Nothing to do here.

  return true;
}
