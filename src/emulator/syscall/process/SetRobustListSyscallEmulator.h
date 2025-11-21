#ifndef SETROBUSTLISTSYSCALLEMULATOR_H
#define SETROBUSTLISTSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class SetRobustListSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~SetRobustListSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //SETROBUSTLISTSYSCALLEMULATOR_H
