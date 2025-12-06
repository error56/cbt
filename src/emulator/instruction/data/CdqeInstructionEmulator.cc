#include "CdqeInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"

bool CdqeInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  // CDQE: Sign-extend EAX to RAX
  const int32_t eax_value = static_cast<int32_t>(registers.rax & 0xFFFFFFFF);
  registers.rax = static_cast<uint64_t>(static_cast<int64_t>(eax_value));

  return true;
}
