#ifndef JNSINSTRUCTIONEMULATOR_H
#define JNSINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class JnsInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~JnsInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //JNSINSTRUCTIONEMULATOR_H
