#ifndef CMOVLEINSTRUCTIONEMULATOR_H
#define CMOVLEINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class CmovleInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~CmovleInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //CMOVLEINSTRUCTIONEMULATOR_H
