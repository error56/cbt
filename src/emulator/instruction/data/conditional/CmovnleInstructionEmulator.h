#ifndef CMOVNLEINSTRUCTIONEMULATOR_H
#define CMOVNLEINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class CmovnleInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~CmovnleInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //CMOVNLEINSTRUCTIONEMULATOR_H
