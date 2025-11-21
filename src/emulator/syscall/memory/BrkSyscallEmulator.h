#ifndef BRKSYSCALLEMULATOR_H
#define BRKSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class BrkSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~BrkSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //BRKSYSCALLEMULATOR_H
