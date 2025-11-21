#include "GetuidSyscallEmulator.h"

InterceptionResult GetuidSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  return kExecuteSyscall;
}
