#include "MprotectSyscallEmulator.h"

#include "../../../execution/ExecutionThread.h"

InterceptionResult MprotectSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  // @TODO: Track guest memory allocations - their size, flags, length and base
  // addresses (and also track the amount of total allocated memory and memory
  // ranges to use in munmap)

  return kExecuteSyscall;
}
