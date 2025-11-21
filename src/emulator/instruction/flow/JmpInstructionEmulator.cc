#include "JmpInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"
#include "spdlog/spdlog.h"

bool JmpInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction,
    ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  // Handle every operand type separately
  if (decoded_instruction.operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
    current_instruction_address += decoded_instruction.operands[0].imm.value.s;
  } else if (decoded_instruction.operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY) {
    assert(decoded_instruction.operands[0].mem.base == ZYDIS_REGISTER_RIP);

    const auto addr = current_instruction_address +
                      decoded_instruction.operands[0].mem.disp.value;
    const auto value = *reinterpret_cast<uint64_t*>(addr);
    current_instruction_address = value;
  } else if (decoded_instruction.operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) {
    switch (decoded_instruction.operands[0].reg.value) {
      case ZYDIS_REGISTER_RAX:
        current_instruction_address = thread->GetGuestRegisters().rax;
      break;
      case ZYDIS_REGISTER_RDX:
        current_instruction_address = thread->GetGuestRegisters().rdx;
      break;
      default:
        // @TOOD: Research possible registers
        spdlog::error("Invalid register passed");
        return false;
      break;
    }
  } else {
    spdlog::error("Invalid operand type for instruction.");

    return false;
  }

  return true;
}
