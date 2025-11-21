#include "IoctlSyscallEmulator.h"

#include "../../../execution/ExecutionThread.h"

InterceptionResult IoctlSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  if (GetSyscallArgument(registers, 2) == TIOCGWINSZ) {
    registers.rax = 0;

    return kReturnToTheGuest;
  }

  return kPanic;
}
