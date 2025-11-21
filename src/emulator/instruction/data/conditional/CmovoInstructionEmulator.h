#ifndef CMOVOINSTRUCTIONEMULATOR_H
#define CMOVOINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class CmovoInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~CmovoInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //CMOVOINSTRUCTIONEMULATOR_H
