#ifndef GETGIDSYSCALLEMULATOR_H
#define GETGIDSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class GetgidSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~GetgidSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //GETGIDSYSCALLEMULATOR_H
