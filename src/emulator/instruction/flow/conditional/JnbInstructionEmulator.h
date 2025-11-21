#ifndef JNBINSTRUCTIONEMULATOR_H
#define JNBINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class JnbInstructionEmulator : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

 public:
  ~JnbInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif  // JNBINSTRUCTIONEMULATOR_H
