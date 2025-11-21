#ifndef UNAMESYSCALLEMULATOR_H
#define UNAMESYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class UnameSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~UnameSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //UNAMESYSCALLEMULATOR_H
