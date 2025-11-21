#ifndef XORINSTRUCTIONEMULATOR_H
#define XORINSTRUCTIONEMULATOR_H

#include "../../InstructionEmulator.h"

class XorInstructionEmulator final : public InstructionEmulator {
  using InstructionEmulator::InstructionEmulator;

public:
  ~XorInstructionEmulator() override = default;
  bool EmulateInstruction(uint64_t& current_instruction_address,
                          DecodedInstruction& decoded_instruction,
                          ExecutionThread* thread,
                          std::stack<std::pair<uint64_t, uint64_t>>& call_stack)
      const noexcept override;
};

#endif //XORINSTRUCTIONEMULATOR_H
