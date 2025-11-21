#include "ExitGroupSyscallEmulator.h"

#include "../../../execution/ExecutionThread.h"

InterceptionResult ExitGroupSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  thread->Stop();

  return kReturnToTheGuest;
}
