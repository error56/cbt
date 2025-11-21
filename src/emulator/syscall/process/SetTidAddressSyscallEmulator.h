#ifndef SETTIDADDRESSSYSCALLEMULATOR_H
#define SETTIDADDRESSSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class SetTidAddressSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~SetTidAddressSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //SETTIDADDRESSSYSCALLEMULATOR_H
