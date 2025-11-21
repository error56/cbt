#include "BasicBlockDecoder.h"

#include "Zydis/Decoder.h"
#include "Zydis/Formatter.h"
#include "spdlog/spdlog.h"

// Max instruction size in bytes.
constexpr auto kMaxInstructionSize = 15;

// Length of buffer containing text representation of current instruction.
constexpr auto kInstructionBufferLength = 256;

std::shared_ptr<BasicBlock> BasicBlockDecoder::ExtractBasicBlock(
    uint64_t& instruction_address) const noexcept {
  auto instructions = std::make_shared<std::vector<DecodedInstruction>>();
  // Initialize decoder and formatter

#ifdef LOG_ASM_INSTRUCTIONS
  ZydisFormatter formatter;
  ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
#endif
  ZydisDecoder decoder;
  ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
  ZydisDecodedInstruction instruction;

  while (true) {
    // Decode instruction with operands
    std::array<ZydisDecodedOperand, ZYDIS_MAX_OPERAND_COUNT> operands{};
    const auto& status = ZydisDecoderDecodeFull(
        &decoder, reinterpret_cast<const void*>(instruction_address),
        kMaxInstructionSize, &instruction, operands.data());

    if (status == ZYDIS_STATUS_DECODING_ERROR) {
      spdlog::error("Zydis decoding error");

      return std::make_shared<BasicBlock>(instructions);
    }

#ifdef LOG_ASM_INSTRUCTIONS
    // Print it to the screen
    std::array<char, kInstructionBufferLength> buffer{};
    ZydisFormatterFormatInstruction(&formatter, &instruction, operands.data(),
                                    instruction.operand_count_visible,
                                    buffer.data(), kInstructionBufferLength,
                                    instruction_address, nullptr);

    spdlog::info("0x{:x} {}", instruction_address, buffer.data());
#endif
#ifdef IDA_PLUGIN_LOGGING
    // Send debug information to the IDA plugin
    tracing_server_->BroadcastNewInstruction("exe",
                                             current_instruction_address);
#endif

    // Add instruction to the basic block
    instructions->push_back(DecodedInstruction{.instruction = instruction,
                                               .operands = operands,
                                               .address = instruction_address,
                                               .size = instruction.length});

    instruction_address += instruction.length;

    // Check if we haven't reached end of basic block
    if (std::ranges::find(kExitInstructions, instruction.mnemonic) !=
        std::end(kExitInstructions)) {
#ifdef LOG_ASM_INSTRUCTIONS
      spdlog::warn("== Got to the end of the basic block");
#endif

      return std::make_shared<BasicBlock>(instructions);
    }
  }
}
