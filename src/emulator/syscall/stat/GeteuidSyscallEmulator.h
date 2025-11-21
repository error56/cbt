#ifndef GETEUIDSYSCALLEMULATOR_H
#define GETEUIDSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class GeteuidSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~GeteuidSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //GETEUIDSYSCALLEMULATOR_H
