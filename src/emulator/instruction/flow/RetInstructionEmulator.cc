#include "RetInstructionEmulator.h"
#include "../../../execution/ExecutionThread.h"

bool RetInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  current_instruction_address = call_stack.top().first;

  if (call_stack.empty()) {
    thread->Stop();
  } else {
    thread->PopFromGuestStack(8);
    call_stack.pop();
  }

  return true;
}
