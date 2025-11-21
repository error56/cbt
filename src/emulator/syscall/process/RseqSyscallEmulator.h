#ifndef RSEQSYSCALLEMULATOR_H
#define RSEQSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class RseqSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~RseqSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //RSEQSYSCALLEMULATOR_H
