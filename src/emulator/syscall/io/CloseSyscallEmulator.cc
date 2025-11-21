#include "CloseSyscallEmulator.h"

#include "../../../execution/ExecutionThread.h"

InterceptionResult CloseSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  const auto fd = GetSyscallArgument(registers, 1);

  // Remove descriptor from the file descriptor table. It's basically what
  // close() syscall is doing in emulated environment like this.
  emulation_context->file_descriptor_table->RemoveDescriptor(fd);

  // Closed successfully
  registers.rax = 0;

  return kReturnToTheGuest;
}