#ifndef CMOVNBEINSTRUCTIONEMULATOR_H
#define CMOVNBEINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class CmovnbeInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~CmovnbeInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //CMOVNBEINSTRUCTIONEMULATOR_H
