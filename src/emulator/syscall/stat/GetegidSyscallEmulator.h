#ifndef GETEGIDSYSCALLEMULATOR_H
#define GETEGIDSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class GetegidSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~GetegidSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //GETEGIDSYSCALLEMULATOR_H
