#ifndef JNZINSTRUCTIONEMULATOR_H
#define JNZINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class JnzInstructionEmulator : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

 public:
  ~JnzInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif  // JNZINSTRUCTIONEMULATOR_H
