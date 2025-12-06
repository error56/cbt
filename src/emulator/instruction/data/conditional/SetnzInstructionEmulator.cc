#include "SetnzInstructionEmulator.h"

#include "../../../../execution/ExecutionThread.h"

bool SetnzInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto operands = decoded_instruction.operands;
  auto& registers = thread->GetGuestRegisters();

  // SETNZ (SETNE): Set byte if not zero (ZF=0)
  const bool zero_flag = (registers.rflags & kRflagsZeroFlagMask) != 0;
  const uint8_t result = !zero_flag ? 1 : 0;

  WriteOperand(registers, operands[0], result);

  return true;
}
