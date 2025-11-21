#include "GetgidSyscallEmulator.h"

InterceptionResult GetgidSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  return kExecuteSyscall;
}
