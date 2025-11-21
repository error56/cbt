#ifndef MOVINSTRUCTIONEMULATOR_H
#define MOVINSTRUCTIONEMULATOR_H

#include "../InstructionEmulator.h"

class MovInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~MovInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //MOVINSTRUCTIONEMULATOR_H
