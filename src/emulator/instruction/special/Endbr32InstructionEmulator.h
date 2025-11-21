#ifndef ENDBR32INSTRUCTIONEMULATOR_H
#define ENDBR32INSTRUCTIONEMULATOR_H

#include "../InstructionEmulator.h"

class Endbr32InstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~Endbr32InstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //ENDBR32INSTRUCTIONEMULATOR_H
