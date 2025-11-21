#ifndef SHRINSTRUCTIONEMULATOR_H
#define SHRINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class ShrInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~ShrInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};
#endif //SHRINSTRUCTIONEMULATOR_H
