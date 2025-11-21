#include "BrkSyscallEmulator.h"

#include "../../../execution/ExecutionThread.h"

InterceptionResult BrkSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  const auto address = static_cast<uint32_t>(GetSyscallArgument(registers, 1));

  if (address == 0) {
    registers.rax = emulation_context->brk_start;
  } else {
    assert(address < emulation_context->brk_end);

    registers.rax = address;
  }

  return kReturnToTheGuest;
}