#ifndef ADDINSTRUCIONEMULATOR_H
#define ADDINSTRUCIONEMULATOR_H

#include "../../InstructionEmulator.h"

class AddInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~AddInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //ADDINSTRUCIONEMULATOR_H
