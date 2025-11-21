#include "SyscallInterceptor.h"

#include "../execution/ExecutionThread.h"

bool SyscallInterceptor::Intercept(
    ExecutionThread* thread, const UnixSyscall syscall,
    ProcessorRegisters& registers) const noexcept {
  if (!functions_.contains(syscall)) {
    // There's nothing to call
    //
    // @TODO: Silently pass?
    return false;
  }

  const auto callback = functions_.at(syscall);

  SyscallContext arg = {
      .regs = registers,
  };

  const auto result = callback(registers, arg, *thread);

  if (result == kReturnToTheGuest) {
    return true;
  }
  if (result == kExecuteSyscall) {
    thread->PerformSyscall();
    return true;
  }

  return false;
}

bool SyscallInterceptor::RegisterInterceptor(
    UnixSyscall syscall, const SyscallInterceptorFunction& function) noexcept {
  if (!functions_.contains(syscall)) {
    functions_.insert(std::make_pair(syscall, function));

    return true;
  }

  return false;
}

uint64_t SyscallContext::GetArgument(
    const uint64_t argument_number) const noexcept {
  assert(argument_number < 6);

  switch (argument_number) {
    case 0:
      return regs.rdi;
    case 1:
      return regs.rsi;
    case 2:
      return regs.rdx;
    case 3:
      return regs.r10;
    case 4:
      return regs.r8;
    case 5:
      return regs.r9;
    default:
      break;
      // unreachable
  }
  return 0;
}
