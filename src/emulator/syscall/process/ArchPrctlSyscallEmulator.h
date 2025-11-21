#ifndef ARCHPRCTLSYSCALLEMULATOR_H
#define ARCHPRCTLSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class ArchPrctlSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~ArchPrctlSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //ARCHPRCTLSYSCALLEMULATOR_H
