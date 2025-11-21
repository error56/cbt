#ifndef JSINSTRUCTIONEMULATOR_H
#define JSINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class JsInstructionEmulator : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~JsInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //JSINSTRUCTIONEMULATOR_H
