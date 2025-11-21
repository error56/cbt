#ifndef IOCTLSYSCALLEMULATOR_H
#define IOCTLSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class IoctlSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

 public:
  ~IoctlSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif  // IOCTLSYSCALLEMULATOR_H
