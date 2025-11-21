#ifndef SYSCALLINSTRUCTIONEMULATOR_H
#define SYSCALLINSTRUCTIONEMULATOR_H

#include <string>

#include "../InstructionEmulator.h"

class SyscallInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

  [[nodiscard]] static std::string GetCurrentFunctionName(
      uint64_t function_address, const ExecutionThread* thread) noexcept;

 public:
  ~SyscallInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif  // SYSCALLINSTRUCTIONEMULATOR_H
