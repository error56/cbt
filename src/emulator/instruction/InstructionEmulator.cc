#include "InstructionEmulator.h"

#include "../../execution/ExecutionThread.h"
#include "spdlog/spdlog.h"

uint64_t InstructionEmulator::GetRegisterValue(
    const ProcessorRegisters& regs, const ZydisRegister reg) noexcept {
  switch (ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, reg)) {
    case ZYDIS_REGISTER_RAX:
      return regs.rax;
    case ZYDIS_REGISTER_RBX:
      return regs.rbx;
    case ZYDIS_REGISTER_RCX:
      return regs.rcx;
    case ZYDIS_REGISTER_RDX:
      return regs.rdx;
    case ZYDIS_REGISTER_RBP:
      return regs.rbp;
    case ZYDIS_REGISTER_RSP:
      return regs.rsp;
    case ZYDIS_REGISTER_RSI:
      return regs.rsi;
    case ZYDIS_REGISTER_RDI:
      return regs.rdi;
    case ZYDIS_REGISTER_R8:
      return regs.r8;
    case ZYDIS_REGISTER_R9:
      return regs.r9;
    case ZYDIS_REGISTER_R10:
      return regs.r10;
    case ZYDIS_REGISTER_R11:
      return regs.r11;
    case ZYDIS_REGISTER_R12:
      return regs.r12;
    case ZYDIS_REGISTER_R13:
      return regs.r13;
    case ZYDIS_REGISTER_R14:
      return regs.r14;
    case ZYDIS_REGISTER_R15:
      return regs.r15;
    case ZYDIS_REGISTER_RIP:
      return regs.rip;
    default:
      spdlog::error("Tried to get value of unknown register");
      // panic
      exit(16);
  }
}

void InstructionEmulator::SetRegisterValue(ProcessorRegisters& regs,
                                           const ZydisRegister reg,
                                           const uint64_t value,
                                           const uint8_t size) noexcept {
  const ZydisRegister largest =
      ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, reg);

  uint64_t* target = nullptr;
  switch (largest) {
    case ZYDIS_REGISTER_RAX:
      target = &regs.rax;
      break;
    case ZYDIS_REGISTER_RBX:
      target = &regs.rbx;
      break;
    case ZYDIS_REGISTER_RCX:
      target = &regs.rcx;
      break;
    case ZYDIS_REGISTER_RDX:
      target = &regs.rdx;
      break;
    case ZYDIS_REGISTER_RBP:
      target = &regs.rbp;
      break;
    case ZYDIS_REGISTER_RSP:
      target = &regs.rsp;
      break;
    case ZYDIS_REGISTER_RSI:
      target = &regs.rsi;
      break;
    case ZYDIS_REGISTER_RDI:
      target = &regs.rdi;
      break;
    case ZYDIS_REGISTER_R8:
      target = &regs.r8;
      break;
    case ZYDIS_REGISTER_R9:
      target = &regs.r9;
      break;
    case ZYDIS_REGISTER_R10:
      target = &regs.r10;
      break;
    case ZYDIS_REGISTER_R11:
      target = &regs.r11;
      break;
    case ZYDIS_REGISTER_R12:
      target = &regs.r12;
      break;
    case ZYDIS_REGISTER_R13:
      target = &regs.r13;
      break;
    case ZYDIS_REGISTER_R14:
      target = &regs.r14;
      break;
    case ZYDIS_REGISTER_R15:
      target = &regs.r15;
      break;
    case ZYDIS_REGISTER_RIP:
      target = &regs.rip;
      break;
    default:
      spdlog::error("Unknown register");
      exit(12345);
  }

  switch (size) {
    case 8: {
      // 8-bit register (AL/BL/CL/DL/AH/BH/CH/DH)

      // Check if it's high byte of the register (AH/BH/CH/DH)
      const bool is_high_byte =
          (reg >= ZYDIS_REGISTER_AH && reg <= ZYDIS_REGISTER_BH) ||
          (reg >= ZYDIS_REGISTER_CH && reg <= ZYDIS_REGISTER_DH);

      if (is_high_byte) {
        // Write to the higher byte
        *target = (*target & ~0xFF00ULL) | ((value & 0xFF) << 8);
      } else {
        // Write to the lower byte
        *target = (*target & ~0xFFULL) | (value & 0xFF);
      }
      break;
    }
    case 16: {
      // 16-bit register (AX/BX/CD/DX)
      *target = (*target & ~0xFFFFULL) | (value & 0xFFFF);
      break;
    }
    case 32: {
      // 32-bit register (EAX/EBX/ECX/EDX)
      // In x86_64, write to the 32-bit register zeroes high 32-bits
      *target = value & 0xFFFFFFFFULL;
      break;
    }
    case 64: {
      // 64-bit register (RAX/RBX/RCX/RDX)
      *target = value;
      break;
    }
    default:
      spdlog::error("Invalid size");
      exit(12345);
  }
}

uint64_t InstructionEmulator::ReadOperand(
    const ProcessorRegisters& regs,
    const ZydisDecodedOperand& operand) noexcept {
  if (operand.type == ZYDIS_OPERAND_TYPE_REGISTER) {
    return GetRegisterValue(regs, operand.reg.value);
  }

  if (operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
    return operand.imm.value.u;
  }

  if (operand.type == ZYDIS_OPERAND_TYPE_MEMORY) {
    uint64_t addr = 0;

    // Base register
    if (operand.mem.base != ZYDIS_REGISTER_NONE) {
      addr += GetRegisterValue(regs, operand.mem.base);
    }

    // Index register * scale
    if (operand.mem.index != ZYDIS_REGISTER_NONE) {
      addr += GetRegisterValue(regs, operand.mem.index) * operand.mem.scale;
    }

    // Displacement
    addr += operand.mem.disp.value;

    // Segment (only for FS/GS)
    if (operand.mem.segment == ZYDIS_REGISTER_FS) {
      addr += regs.fs;
    } else if (operand.mem.segment == ZYDIS_REGISTER_GS) {
      addr += regs.gs;
    }

    uint64_t value = 0;
    const size_t read_size = operand.size / 8;

    switch (read_size) {
      case 1:
        value = *reinterpret_cast<uint8_t*>(addr);
        break;
      case 2:
        value = *reinterpret_cast<uint16_t*>(addr);
        break;
      case 4:
        value = *reinterpret_cast<uint32_t*>(addr);
        break;
      case 8:
        value = *reinterpret_cast<uint64_t*>(addr);
        break;
      default:
        spdlog::error("Invalid read size");
        exit(12345);
    }

    return value;
  }

  spdlog::error("Unknown operand type");
  exit(12345);
}

void InstructionEmulator::WriteOperand(ProcessorRegisters& regs,
                                       const ZydisDecodedOperand& operand,
                                       const uint64_t value) noexcept {
  if (operand.type == ZYDIS_OPERAND_TYPE_REGISTER) {
    SetRegisterValue(regs, operand.reg.value, value, operand.size);
    return;
  }

  if (operand.type == ZYDIS_OPERAND_TYPE_MEMORY) {
    // The same logic as in ReadOperand
    uint64_t addr = 0;

    if (operand.mem.base != ZYDIS_REGISTER_NONE) {
      addr += GetRegisterValue(regs, operand.mem.base);
    }

    if (operand.mem.index != ZYDIS_REGISTER_NONE) {
      addr += GetRegisterValue(regs, operand.mem.index) * operand.mem.scale;
    }

    addr += operand.mem.disp.value;

    if (operand.mem.segment == ZYDIS_REGISTER_FS) {
      spdlog::info("Read from fs");
      addr += regs.fs;
    } else if (operand.mem.segment == ZYDIS_REGISTER_GS) {
      spdlog::info("Read from gs");
      addr += regs.gs;
    }

    const size_t write_size = operand.size / 8;

    switch (write_size) {
      case 1:
        *reinterpret_cast<uint8_t*>(addr) = value;
        break;
      case 2:
        *reinterpret_cast<uint16_t*>(addr) = value;
        break;
      case 4:
        *reinterpret_cast<uint32_t*>(addr) = value;
        break;
      case 8:
        *reinterpret_cast<uint64_t*>(addr) = value;
        break;
      default:
        spdlog::error("Invalid read size");
        exit(12345);
    }

    return;
  }

  spdlog::error("Unknown operand type");
  exit(12345);
}

void InstructionEmulator::UpdateFlags(ProcessorRegisters& registers,
                                      uint64_t operand1, uint64_t operand2,
                                      uint64_t result, const uint8_t size,
                                      const Operation operation) noexcept {
  constexpr uint64_t kFlagMask =
      kRflagsCarryFlagMask | kRflagsParityFlagMask | kRflagsAuxiliaryFlagMask |
      kRflagsZeroFlagMask | kRflagsSignFlagMask | kRflagsOverflowFlagMask;

  // Reset flags
  registers.rflags &= ~kFlagMask;
  // Set reserved bit (always 1)
  registers.rflags |= 1 << 1;

  const uint64_t size_mask = (size >= 64) ? ~0ULL : ((1ULL << size) - 1);
  operand1 &= size_mask;
  operand2 &= size_mask;
  result &= size_mask;

  if (CalculateCarryFlag(registers, operand1, operand2, result, size, size_mask,
                         operation)) {
    registers.rflags |= kRflagsCarryFlagMask;
  }

  if (CalculateParityFlag(result)) {
    registers.rflags |= kRflagsParityFlagMask;
  }

  if (CalculateAuxiliaryFlag(operand1, operand2, operation)) {
    registers.rflags |= kRflagsAuxiliaryFlagMask;
  }

  if (result == 0) {
    registers.rflags |= kRflagsZeroFlagMask;
  }

  if ((result >> (size - 1) & 1) == 1) {
    registers.rflags |= kRflagsSignFlagMask;
  }

  if (CalculateOverflowFlag(operand1, operand2, result, size, operation)) {
    registers.rflags |= kRflagsOverflowFlagMask;
  }
}

bool InstructionEmulator::CalculateParityFlag(uint64_t result) noexcept {
  const uint8_t low_byte = result & 0xFF;

  return (__builtin_popcount(low_byte) % 2) == 0;
}

bool InstructionEmulator::CalculateCarryFlag(
    const ProcessorRegisters& registers, const uint64_t operand1,
    const uint64_t operand2, const uint64_t result, const uint8_t size,
    const uint64_t size_mask, const Operation operation) noexcept {
  switch (operation) {
    case kCmp:
    case kSub:
    case kDec:
      return operand1 < operand2;

    case kAdd:
    case kInc:
      return (operand1 + operand2) > size_mask;

    case kShl: {
      if (operand2 == 0 || operand2 > size) {
        return false;
      }

      return (operand1 >> (size - operand2)) & 1;
    }

    case kShr: {
      if (operand2 == 0 || operand2 > size) {
        return false;
      }

      return (operand1 >> (operand2 - 1)) & 1;
    }
    case kSar: {
      if (operand2 == 0) {
        return false;
      }

      if (operand2 <= size) {
        return (operand1 >> (operand2 - 1)) & 1;
      }

      return (operand1 >> (size - 1)) & 1;
    }

    case kNeg:
      return result != 0;

    case kAnd:
    case kOr:
    case kXor:
    case kTest:
      return false;

    default:
      return registers.rflags & 1;
  }
}

bool InstructionEmulator::CalculateOverflowFlag(
    const uint64_t operand1, const uint64_t operand2, const uint64_t result,
    const uint8_t size, const Operation operation) noexcept {
  const bool sign_op1 = (operand1 >> (size - 1)) & 1;
  const bool sign_op2 = (operand2 >> (size - 1)) & 1;
  const bool sign_result = (result >> (size - 1)) & 1;

  switch (operation) {
    case kNeg: {
      const uint64_t min_value = (1ULL << (size - 1));
      return operand1 == min_value;
    }

    case kCmp:
    case kSub:
    case kDec:
      return (sign_op1 != sign_op2) && (sign_op1 != sign_result);

    case kAdd:
    case kInc:
      return (sign_op1 == sign_op2) && (sign_op1 != sign_result);

    case kShl:
      if (operand2 == 1) {
        const bool original_sign = (operand1 >> (size - 1)) & 1;
        return original_sign != sign_result;
      }
      return false;

    case kShr:
      return (operand2 == 1) ? ((operand1 >> (size - 1)) & 1) : false;

    case kSar:
    case kAnd:
    case kOr:
    case kXor:
    case kTest:
      return false;

    default:
      return false;
  }
}

bool InstructionEmulator::CalculateAuxiliaryFlag(
    const uint64_t operand1, const uint64_t operand2,
    const Operation operation) noexcept {
  switch (operation) {
    case kAdd:
    case kInc:
      return ((operand1 & 0xF) + (operand2 & 0xF)) > 0xF;

    case kSub:
    case kCmp:
    case kDec:
    case kNeg:
      return (operand1 & 0xF) < (operand2 & 0xF);

    default:
      return false;
  }
}
