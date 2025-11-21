#include "CmpInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"

bool CmpInstructionEmulator::EmulateInstruction(
    uint64_t& current_instruction_address,
    DecodedInstruction& decoded_instruction, ExecutionThread* thread,
    std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
  const auto operands = decoded_instruction.operands;

  uint64_t dst = ReadOperand(thread->GetGuestRegisters(), operands[0]);
  uint64_t src = ReadOperand(thread->GetGuestRegisters(), operands[1]);
  const uint8_t size = operands[0].size;

  // Mask operands to the correct size
  const uint64_t size_mask = (size >= 64) ? ~0ULL : ((1ULL << size) - 1);
  //const uint64_t size_mask = (size == 0 || size > 64) ? 64 : size;
  dst &= size_mask;
  src &= size_mask;

  // CMP is basically subtraction dst from src. We use __uint128_t here to
  // detect borrows.
  const __uint128_t extended_dst = dst;
  const __uint128_t extended_src = src;
  const __uint128_t extended_result = extended_dst - extended_src;

  // Cast the result of subtraction to the size mask
  const uint64_t result = static_cast<uint64_t>(extended_result & size_mask);

  // Save signs
  const bool dst_sign = ((dst >> (size - 1)) & 1) != 0;
  const bool src_sign = ((src >> (size - 1)) & 1) != 0;

  // Calculate CPU flags
  const bool zero_flag = (result == 0);
  const bool sign_flag = ((result >> (size - 1)) & 1) != 0;
  const bool result_sign = sign_flag;
  const bool carry_flag = (dst < src);
  const bool overflow_flag =
      (dst_sign != src_sign) && (dst_sign != result_sign);
  const bool auxiliary_flag = ((dst & 0xF) < (src & 0xF));
  const bool parity_flag = (__builtin_popcount(result & 0xFF) % 2) == 0;

  constexpr uint64_t kFlagMask =
      kRflagsCarryFlagMask | kRflagsParityFlagMask | kRflagsAuxiliaryFlagMask |
      kRflagsZeroFlagMask | kRflagsSignFlagMask | kRflagsOverflowFlagMask;

  // Reset flags
  auto& registers = thread->GetGuestRegisters();
  registers.rflags &= ~kFlagMask;

  // Set reserved bit (always 1)
  registers.rflags |= 1 << 1;

  // Set flags depending on our calculations
  if (carry_flag) {
    registers.rflags |= kRflagsCarryFlagMask;
  }
  if (parity_flag) {
    registers.rflags |= kRflagsParityFlagMask;
  }
  if (auxiliary_flag) {
    registers.rflags |= kRflagsAuxiliaryFlagMask;
  }
  if (zero_flag) {
    registers.rflags |= kRflagsZeroFlagMask;
  }
  if (sign_flag) {
    registers.rflags |= kRflagsSignFlagMask;
  }
  if (overflow_flag) {
    registers.rflags |= kRflagsOverflowFlagMask;
  }

  return true;
}
