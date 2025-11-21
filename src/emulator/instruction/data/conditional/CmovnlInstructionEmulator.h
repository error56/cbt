#ifndef CMOVNLINSTRUCTIONEMULATOR_H
#define CMOVNLINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class CmovnlInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~CmovnlInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //CMOVNLINSTRUCTIONEMULATOR_H
