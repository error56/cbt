#ifndef JMPINSTRUCTIONEMULATOR_H
#define JMPINSTRUCTIONEMULATOR_H
#include "../InstructionEmulator.h"

class JmpInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

 public:
  ~JmpInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif  // JMPINSTRUCTIONEMULATOR_H
