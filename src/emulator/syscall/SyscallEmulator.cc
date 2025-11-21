#include "SyscallEmulator.h"

#include <assert.h>

uint64_t SyscallEmulator::GetSyscallArgument(
    const ProcessorRegisters& registers, const uint8_t argument) noexcept {
  assert(argument > 0 && argument < 7);

  switch (argument) {
    case 1:
      return registers.rdi;
    case 2:
      return registers.rsi;
    case 3:
      return registers.rdx;
    case 4:
      return registers.r10;
    case 5:
      return registers.r8;
    case 6:
      return registers.r9;
    default:
      // Unreachable
      return 0;
  }
}
