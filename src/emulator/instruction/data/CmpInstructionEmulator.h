#ifndef CMPINSTRUCTIONEMULATOR_H
#define CMPINSTRUCTIONEMULATOR_H

#include "../InstructionEmulator.h"

class CmpInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~CmpInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //CMPINSTRUCTIONEMULATOR_H
