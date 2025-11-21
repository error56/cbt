#ifndef SYSCALLEMULATOR_H
#define SYSCALLEMULATOR_H

#include <memory>

#include "../../execution/ProcessState.h"

class ExecutionThread;

class SyscallEmulator {
 protected:
  std::shared_ptr<HighLevelEmulationContext> emulation_context;

  [[nodiscard]] static uint64_t GetSyscallArgument(
      const ProcessorRegisters& registers, uint8_t argument) noexcept;

 public:
  virtual ~SyscallEmulator() = default;
  explicit SyscallEmulator(
      const std::shared_ptr<HighLevelEmulationContext>& emulation_context)
      : emulation_context(emulation_context) {};

  [[nodiscard]] virtual InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept {
    return kPassDownTheStack;
  };
};

#endif  // SYSCALLEMULATOR_H
