#include "SyscallInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"

bool SyscallInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto syscall_name =
      syscall_name_lookup_table[thread->GetGuestRegisters().rax];
  const auto unix_syscall = syscall_id_lookup_table[syscall_name];

  // Try to handle syscall using user-provided code
  const auto result = thread->hlec_->syscall_interceptors->Intercept(
      thread, unix_syscall, thread->GetGuestRegisters());

  if (result) {
    // If user code handled the syscall by itself, then just return
    return true;
  }

  // Try to use CBT-provided syscall stubs
  const auto cbt_decision = thread->HandleSyscall(unix_syscall);

  if (cbt_decision == kReturnToTheGuest) {
    // CBT handling succeeded, so return to the translator
    return true;
  } else if (cbt_decision == kExecuteSyscall) {
    // Execute syscall normally and return to the translator.
    thread->PerformSyscall();

    return true;
  } else {
    // Otherwise, it's some kind of unknown syscall not implemented by the guest
    // nor by guest, so panic
    spdlog::error("Failed to handle intercepted syscall {}", syscall_name);

    return false;
  }
}
