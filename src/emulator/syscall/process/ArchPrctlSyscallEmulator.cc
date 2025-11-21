#include "ArchPrctlSyscallEmulator.h"

#include <asm/prctl.h>

#include "../../../execution/ExecutionThread.h"

InterceptionResult ArchPrctlSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetProcessorState();

  //         Name	Value
  // ARCH_SET_FS	0x1002
  // ARCH_GET_FS	0x1003
  // ARCH_SET_GS	0x1001
  // ARCH_GET_GS	0x1004

  // Currently we support only setting the value of FS register.
  if (GetSyscallArgument(registers.guest_state, 1) != ARCH_SET_FS) {
    registers.guest_state.rax = -ENOMEM;

    return kReturnToTheGuest;
  }

  // Set the guest FS and return with success code.
  registers.guest_state.fs = GetSyscallArgument(registers.guest_state, 2);
  registers.guest_state.rax = 0;

  return kReturnToTheGuest;
}
