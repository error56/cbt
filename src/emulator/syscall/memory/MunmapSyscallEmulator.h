#ifndef MUNMAPSYSCALLEMULATOR_H
#define MUNMAPSYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

class MunmapSyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~MunmapSyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //MUNMAPSYSCALLEMULATOR_H
