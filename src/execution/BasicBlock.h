#ifndef BASICBLOCK_H
#define BASICBLOCK_H

#include <bitset>
#include <memory>
#include <vector>

#include "BasicBlockAllocator.h"
#include "ProcessState.h"
#include "Zydis/Encoder.h"
#include "zasm/serialization/serializer.hpp"

// Size of bitset describing all possible instructions of x86_64, where one bit
// is one mnemonic.
constexpr auto kInstructionBitSetMapSize = ZYDIS_MNEMONIC_MAX_VALUE;

// Instruction count needed to be put after each basic block to successfully
// return to the translator.
constexpr size_t kBasicBlockAdditionalInstructionCount = 3;  // push + mov + jmp

// Makes compile time bitset of blocked mnemonics in trivial basic blocks.
constexpr auto MakeCompileTimeMapOfBlockedMnemonics() {
  std::bitset<kInstructionBitSetMapSize> bits{};
  for (const auto mnemonic : {
           ZYDIS_MNEMONIC_IN,        ZYDIS_MNEMONIC_OUT,
           ZYDIS_MNEMONIC_INVLPG,    ZYDIS_MNEMONIC_INVPCID,
           ZYDIS_MNEMONIC_XCHG,      ZYDIS_MNEMONIC_CMPXCHG,
           ZYDIS_MNEMONIC_CMPXCHG8B, ZYDIS_MNEMONIC_CMPXCHG16B,
           ZYDIS_MNEMONIC_MFENCE,    ZYDIS_MNEMONIC_LFENCE,
           ZYDIS_MNEMONIC_SFENCE,    ZYDIS_MNEMONIC_CPUID,
           ZYDIS_MNEMONIC_RDTSC,     ZYDIS_MNEMONIC_RDTSCP,
           ZYDIS_MNEMONIC_RDMSR,     ZYDIS_MNEMONIC_WRMSR,
           ZYDIS_MNEMONIC_HLT,       ZYDIS_MNEMONIC_UD0,
           ZYDIS_MNEMONIC_UD1,       ZYDIS_MNEMONIC_UD2,
           ZYDIS_MNEMONIC_POP,       ZYDIS_MNEMONIC_POPA,
           ZYDIS_MNEMONIC_POPAD,     ZYDIS_MNEMONIC_POPF,
           ZYDIS_MNEMONIC_POPFD,     ZYDIS_MNEMONIC_POPFQ,
           ZYDIS_MNEMONIC_PUSH,      ZYDIS_MNEMONIC_PUSHA,
           ZYDIS_MNEMONIC_PUSHF,     ZYDIS_MNEMONIC_PUSHAD,
           ZYDIS_MNEMONIC_PUSHFQ,    ZYDIS_MNEMONIC_PUSHFD,
           ZYDIS_MNEMONIC_LEAVE,     ZYDIS_MNEMONIC_IMUL,
       }) {
    bits.set(static_cast<size_t>(mnemonic));
  }
  return bits;
}

// Map of blocked mnemonics in trivial basic block.
constexpr auto kBlockedMnemonicsInTrivialBlock =
    MakeCompileTimeMapOfBlockedMnemonics();

class BasicBlock {
  // All instructions contained in the basic block.
  std::shared_ptr<std::vector<DecodedInstruction>> instructions_;

  // Fixes relative operands in instruction taking into account current
  // instruction address.
  [[nodiscard]] static bool FixOperands(
      ZydisEncoderRequest& request, uint64_t original_instruction_address,
      uint64_t translated_instruction_address) noexcept;

 public:
  explicit BasicBlock(
      const std::shared_ptr<std::vector<DecodedInstruction>>& instructions)
      : instructions_(instructions) {};

  BasicBlock(const BasicBlock&) = delete;
  BasicBlock& operator=(const BasicBlock&) = delete;
  ~BasicBlock() = default;

  // Checks if basic block meets the definition of trivial basic block.
  [[nodiscard]] bool IsTrivial() const noexcept;

  // Checks if basic block accesses FS/GS registers and requires segment switch.
  [[nodiscard]] bool RequiresSegmentSwitch() const noexcept;

  // Checks if basic block is empty (contains only one instruction, breaking the
  // basic block).
  [[nodiscard]] bool IsEmpty() const noexcept;

  // Returns reference to the last instruction.
  [[nodiscard]] DecodedInstruction& LastInstruction() const noexcept;

  // Returns all instructions in this basic block.
  [[nodiscard]] std::shared_ptr<std::vector<DecodedInstruction>>
  GetInstructions() const noexcept;

  // Allocates some executable memory for translated instructions, adds a
  // short code that allows us to save the state, perform context-switch and
  // safely return to the translator.
  [[nodiscard]] ContextSwitchStubInfo AllocateContextSwitch(
      const std::shared_ptr<BasicBlockAllocator>& allocator,
      const zasm::Serializer& context_switch_stub) const noexcept;
};

#endif  // BASICBLOCK_H
