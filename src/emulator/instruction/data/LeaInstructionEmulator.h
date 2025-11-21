#ifndef LEAINSTRUCTIONEMULATOR_H
#define LEAINSTRUCTIONEMULATOR_H

#include "../InstructionEmulator.h"

class LeaInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~LeaInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //LEAINSTRUCTIONEMULATOR_H
