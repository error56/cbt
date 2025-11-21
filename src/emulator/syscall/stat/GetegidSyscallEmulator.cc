#include "GetegidSyscallEmulator.h"

InterceptionResult GetegidSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  return kExecuteSyscall;
}
