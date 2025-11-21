#ifndef INCINSTRUCTIONEMULATOR_H
#define INCINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class IncInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~IncInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //INCINSTRUCTIONEMULATOR_H
