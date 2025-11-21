#ifndef ENDBR64INSTRUCTIONEMULATOR_H
#define ENDBR64INSTRUCTIONEMULATOR_H

#include "../InstructionEmulator.h"

class Endbr64InstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~Endbr64InstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //ENDBR64INSTRUCTIONEMULATOR_H
