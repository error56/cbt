#ifndef MOVSXINSTRUCTIONEMULATOR_H
#define MOVSXINSTRUCTIONEMULATOR_H

#include "../InstructionEmulator.h"

class MovsxInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~MovsxInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //MOVSXINSTRUCTIONEMULATOR_H
