#include "MulInstructionEmulator.h"

#include "../../../../execution/ExecutionThread.h"

bool MulInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto operands = decoded_instruction.operands;
  auto& registers = thread->GetGuestRegisters();

  const uint8_t size = operands[0].size;
  const uint64_t size_mask = (size >= 64) ? ~0ULL : ((1ULL << size) - 1);

  const uint64_t src = ReadOperand(registers, operands[0]) & size_mask;

  uint64_t rax_value;
  if (size == 8) {
    rax_value = registers.rax & 0xFF;  // AL
  } else if (size == 16) {
    rax_value = registers.rax & 0xFFFF;  // AX
  } else if (size == 32) {
    rax_value = registers.rax & 0xFFFFFFFF;  // EAX
  } else {
    rax_value = registers.rax;  // RAX
  }

  // Use uint128_t to detect overflow
  const __uint128_t full_result =
      static_cast<__uint128_t>(rax_value) * static_cast<__uint128_t>(src);

  if (size == 8) {
    // AX = AL * src
    registers.rax =
        (registers.rax & 0xFFFFFFFFFFFF0000ULL) | (full_result & 0xFFFF);
  } else if (size == 16) {
    // DX:AX = AX * src
    registers.rax =
        (registers.rax & 0xFFFFFFFFFFFF0000ULL) | (full_result & 0xFFFF);
    registers.rdx = (registers.rdx & 0xFFFFFFFFFFFF0000ULL) |
                    ((full_result >> 16) & 0xFFFF);
  } else if (size == 32) {
    // EDX:EAX = EAX * src
    registers.rax = full_result & 0xFFFFFFFF;
    registers.rdx = (full_result >> 32) & 0xFFFFFFFF;
  } else {
    // RDX:RAX = RAX * src
    registers.rax = static_cast<uint64_t>(full_result);
    registers.rdx = static_cast<uint64_t>(full_result >> 64);
  }

  // CF and OF are set if the upper half is non-zero
  bool overflow;
  if (size == 8) {
    overflow = (full_result > 0xFF);
  } else if (size == 16) {
    overflow = (full_result > 0xFFFF);
  } else if (size == 32) {
    overflow = (full_result > 0xFFFFFFFF);
  } else {
    overflow = (full_result >> 64) != 0;
  }

  constexpr uint64_t kCarryOverflowMask =
      kRflagsCarryFlagMask | kRflagsOverflowFlagMask;

  if (overflow) {
    registers.rflags |= kCarryOverflowMask;
  } else {
    registers.rflags &= ~kCarryOverflowMask;
  }

  return true;
}
