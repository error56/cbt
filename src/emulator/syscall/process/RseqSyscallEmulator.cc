#include "RseqSyscallEmulator.h"

InterceptionResult RseqSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  // @TODO: This is useful when supporting multithreading, but CBT currently
  // does not support it

  return kExecuteSyscall;
}
