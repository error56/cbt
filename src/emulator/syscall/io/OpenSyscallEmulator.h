#ifndef OPENSYSCALLEMULATOR_H
#define OPENSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class OpenSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~OpenSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //OPENSYSCALLEMULATOR_H
