#ifndef FCNTLSYSCALLEMULATOR_H
#define FCNTLSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class FcntlSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~FcntlSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //FCNTLSYSCALLEMULATOR_H
