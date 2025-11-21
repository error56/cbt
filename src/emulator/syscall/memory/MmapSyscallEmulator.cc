#include "MmapSyscallEmulator.h"

#include "../../../execution/ExecutionThread.h"

InterceptionResult MmapSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  const auto address = GetSyscallArgument(registers, 1);
  const auto length = GetSyscallArgument(registers, 2);
  const auto protection = GetSyscallArgument(registers, 3);
  const auto flags = GetSyscallArgument(registers, 4);
  const auto fd = GetSyscallArgument(registers, 5);
  const auto offset = GetSyscallArgument(registers, 6);

  // @TODO: Track guest memory allocations - their size, flags, length and base
  // addresses (and also track the amount of total allocated memory and memory
  // ranges to use in munmap)
  const auto return_value = mmap(reinterpret_cast<void*>(address), length,
                                 protection, flags, fd, offset);

  registers.rax = reinterpret_cast<uint64_t>(return_value);

  return kReturnToTheGuest;
}
