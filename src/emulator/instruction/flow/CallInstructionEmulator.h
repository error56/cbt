#ifndef CALLINSTRUCTIONEMULATOR_H
#define CALLINSTRUCTIONEMULATOR_H
#include "../InstructionEmulator.h"

class CallInstructionEmulator : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

 public:
  ~CallInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif  // CALLINSTRUCTIONEMULATOR_H
