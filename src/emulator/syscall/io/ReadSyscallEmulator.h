#ifndef READSYSCALLEMULATOR_H
#define READSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class ReadSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~ReadSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //READSYSCALLEMULATOR_H
