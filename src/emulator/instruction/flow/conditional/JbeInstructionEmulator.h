#ifndef JBEINSTRUCTIONEMULATOR_H
#define JBEINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class JbeInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~JbeInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //JBEINSTRUCTIONEMULATOR_H
