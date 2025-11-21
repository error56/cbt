#ifndef CMOVPINSTRUCTIONEMULATOR_H
#define CMOVPINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class CmovpInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~CmovpInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //CMOVPINSTRUCTIONEMULATOR_H
