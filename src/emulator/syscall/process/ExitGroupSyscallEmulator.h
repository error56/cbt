#ifndef EXITGROUPSYSCALLEMULATOR_H
#define EXITGROUPSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class ExitGroupSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~ExitGroupSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //EXITGROUPSYSCALLEMULATOR_H
