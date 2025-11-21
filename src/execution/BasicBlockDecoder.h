#ifndef BASICBLOCKDECODER_H
#define BASICBLOCKDECODER_H

#include <memory>

#include "BasicBlock.h"

// A basic block is a straight-line code sequence with no branches.
// Every basic block is ended by a branch instruction like a conditional
// jmp. We assume that instructions such as syscall or int also end a basic
// block and need to emulate it to be able to really sandbox the code.
constexpr std::array<uint64_t, 36> kExitInstructions = {
    // Procedure calls/returns
    ZYDIS_MNEMONIC_CALL, ZYDIS_MNEMONIC_RET,

    // System calls
    ZYDIS_MNEMONIC_SYSCALL, ZYDIS_MNEMONIC_SYSENTER, ZYDIS_MNEMONIC_SYSRET,

    // Interrupts
    ZYDIS_MNEMONIC_INT, ZYDIS_MNEMONIC_INT1, ZYDIS_MNEMONIC_INT3,
    ZYDIS_MNEMONIC_INTO,

    // Jumps
    ZYDIS_MNEMONIC_JMP, ZYDIS_MNEMONIC_JB, ZYDIS_MNEMONIC_JBE,
    ZYDIS_MNEMONIC_JCXZ, ZYDIS_MNEMONIC_JECXZ, ZYDIS_MNEMONIC_JKNZD,
    ZYDIS_MNEMONIC_JKZD, ZYDIS_MNEMONIC_JL, ZYDIS_MNEMONIC_JLE,
    ZYDIS_MNEMONIC_JMP, ZYDIS_MNEMONIC_JNB, ZYDIS_MNEMONIC_JNBE,
    ZYDIS_MNEMONIC_JNL, ZYDIS_MNEMONIC_JNLE, ZYDIS_MNEMONIC_JNO,
    ZYDIS_MNEMONIC_JNP, ZYDIS_MNEMONIC_JNS, ZYDIS_MNEMONIC_JNZ,
    ZYDIS_MNEMONIC_JO, ZYDIS_MNEMONIC_JP, ZYDIS_MNEMONIC_JRCXZ,
    ZYDIS_MNEMONIC_JS, ZYDIS_MNEMONIC_JZ,

    // Loops
    ZYDIS_MNEMONIC_LOOP, ZYDIS_MNEMONIC_LOOPE, ZYDIS_MNEMONIC_LOOPNE,

    // Special
    ZYDIS_MNEMONIC_CPUID  // NOLINT
};

class BasicBlockDecoder {
 public:
  BasicBlockDecoder() = default;
  BasicBlockDecoder(const BasicBlockDecoder&) = delete;
  BasicBlockDecoder& operator=(const BasicBlockDecoder&) = delete;
  ~BasicBlockDecoder() = default;

  // Gets next instructions from the provided address and tries to get whole
  // basic block. All instructions are pushed into the vector and returned to
  // the caller. Vector has always at least one element (the instruction that
  // ended the basic-block). In such case it will be emulated fully in
  // translator.
  [[nodiscard]] std::shared_ptr<BasicBlock> ExtractBasicBlock(
      uint64_t& instruction_address) const noexcept;
};

#endif  // BASICBLOCKDECODER_H
