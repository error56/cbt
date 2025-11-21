#ifndef JOINSTRUCTIONEMULATOR_H
#define JOINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class JoInstructionEmulator : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~JoInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};


#endif //JOINSTRUCTIONEMULATOR_H
