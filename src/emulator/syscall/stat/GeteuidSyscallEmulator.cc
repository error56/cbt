#include "GeteuidSyscallEmulator.h"

InterceptionResult GeteuidSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  return kExecuteSyscall;
}
