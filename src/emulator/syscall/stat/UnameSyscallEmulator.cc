#include "UnameSyscallEmulator.h"

InterceptionResult UnameSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  return kExecuteSyscall;
}
