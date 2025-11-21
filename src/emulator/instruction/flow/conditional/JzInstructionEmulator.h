#ifndef JZINSTRUCTIONEMULATOR_H
#define JZINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class JzInstructionEmulator : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

 public:
  ~JzInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif  // JZINSTRUCTIONEMULATOR_H
