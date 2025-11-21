#ifndef CLOSESYSCALLEMULATOR_H
#define CLOSESYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class CloseSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~CloseSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //CLOSESYSCALLEMULATOR_H
