#ifndef WRITEVINSTRUCTIONEMULATOR_H
#define WRITEVINSTRUCTIONEMULATOR_H

#include "../SyscallEmulator.h"

class WritevSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~WritevSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //WRITEVINSTRUCTIONEMULATOR_H
