#include "FcntlSyscallEmulator.h"

#include "../../../execution/ExecutionThread.h"

InterceptionResult FcntlSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  // If it's not flag manipulation, then bail out.
  if (GetSyscallArgument(registers, 2) != 2) {
    return kPassDownTheStack;
  }

  // CBT does not track FD flags currently, so return with success code.
  registers.rax = 0;

  return kReturnToTheGuest;
}
