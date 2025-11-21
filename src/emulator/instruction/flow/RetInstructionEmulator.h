#ifndef RETINSTRUCTIONEMULATOR_HPP
#define RETINSTRUCTIONEMULATOR_HPP

#include "../InstructionEmulator.h"

class RetInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

 public:
  ~RetInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif  // RETINSTRUCTIONEMULATOR_HPP
