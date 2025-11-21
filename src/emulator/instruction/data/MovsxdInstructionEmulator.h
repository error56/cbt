#ifndef MOVSXDINSTRUCTIONEMULATOR_H
#define MOVSXDINSTRUCTIONEMULATOR_H

#include "../InstructionEmulator.h"

class MovsxdInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~MovsxdInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //MOVSXDINSTRUCTIONEMULATOR_H
