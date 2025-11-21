#include "LeaInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"

bool LeaInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto& operands = decoded_instruction.operands;
  const auto operand = operands[1];

  if (operand.type != ZYDIS_OPERAND_TYPE_MEMORY) {
    return false;
  }

  uint64_t addr = 0;

  if (operand.mem.base != ZYDIS_REGISTER_NONE) {
    addr += GetRegisterValue(thread->GetGuestRegisters(), operand.mem.base);
  }

  if (operand.mem.index != ZYDIS_REGISTER_NONE) {
    addr += GetRegisterValue(thread->GetGuestRegisters(), operand.mem.index) * operand.mem.scale;
  }

  addr += operand.mem.disp.value;

  WriteOperand(thread->GetGuestRegisters(), operands[0], addr);

  return true;
}
