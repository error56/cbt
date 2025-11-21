#ifndef GETUIDSYSCALLEMULATOR_H
#define GETUIDSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class GetuidSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~GetuidSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //GETUIDSYSCALLEMULATOR_H
