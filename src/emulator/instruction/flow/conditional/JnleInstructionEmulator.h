#ifndef JNLEINSTRUCTIONEMULATOR_H
#define JNLEINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class JnleInstructionEmulator : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~JnleInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //JNLEINSTRUCTIONEMULATOR_H
