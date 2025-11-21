#ifndef MMAPSYSCALLEMULATOR_H
#define MMAPSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class MmapSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~MmapSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //MMAPSYSCALLEMULATOR_H
