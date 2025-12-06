#ifndef MPROTECTSYSCALLEMULATOR_H
#define MPROTECTSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class MprotectSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~MprotectSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //MPROTECTSYSCALLEMULATOR_H
