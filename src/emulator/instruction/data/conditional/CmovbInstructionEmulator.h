#ifndef CMOVBINSTRUCTIONEMULATOR_H
#define CMOVBINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class CmovbInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~CmovbInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //CMOVBINSTRUCTIONEMULATOR_H
