#ifndef SHLINSTRUCTIONEMULATOR_H
#define SHLINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class ShlInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~ShlInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //SHLINSTRUCTIONEMULATOR_H
