#ifndef INSTRUCTIONEMULATOR_H
#define INSTRUCTIONEMULATOR_H
#include <cstdint>
#include <stack>

#include "Zydis/DecoderTypes.h"
#include "Zydis/Register.h"

struct ProcessorRegisters;
struct DecodedInstruction;
class ExecutionThread;

enum Operation : uint8_t {
  kCmp,
  kSub,
  kDec,
  kAdd,
  kInc,
  kShl,
  kShr,
  kSar,
  kNeg,
  kAnd,
  kOr,
  kXor,
  kTest
};

class InstructionEmulator {
  [[nodiscard]] static bool CalculateCarryFlag(
      const ProcessorRegisters& registers, uint64_t operand1, uint64_t operand2,
      uint64_t result, uint8_t size, uint64_t size_mask,
      Operation operation) noexcept;

  [[nodiscard]] static bool CalculateParityFlag(uint64_t result) noexcept;
  [[nodiscard]] static bool CalculateAuxiliaryFlag(
      uint64_t operand1, uint64_t operand2, Operation operation) noexcept;
  [[nodiscard]] static bool CalculateOverflowFlag(uint64_t operand1,
                                                  uint64_t operand2,
                                                  uint64_t result, uint8_t size,
                                                  Operation operation) noexcept;

 protected:
  ProcessorRegisters& cpu_registers;

 public:
  virtual ~InstructionEmulator() = default;
  explicit InstructionEmulator(ProcessorRegisters& cpu_registers)
      : cpu_registers(cpu_registers) {};

  [[nodiscard]] virtual bool EmulateInstruction(
      uint64_t& current_instruction_address,
      DecodedInstruction& decoded_instruction, ExecutionThread* thread,
      // [address of call instruction], [address of procedure] <--- allows us to
      // a) print stacktrace b) resolve syscalls easily
      std::stack<std::pair<uint64_t, uint64_t>>& call_stack) const noexcept {
    return false;
  };

  [[nodiscard]] static uint64_t GetRegisterValue(const ProcessorRegisters& regs,
                                                 ZydisRegister reg) noexcept;

  static void SetRegisterValue(ProcessorRegisters& regs, ZydisRegister reg,
                               uint64_t value, uint8_t size) noexcept;

  [[nodiscard]] static uint64_t ReadOperand(
      const ProcessorRegisters& regs,
      const ZydisDecodedOperand& operand) noexcept;

  static void WriteOperand(ProcessorRegisters& regs,
                           const ZydisDecodedOperand& operand,
                           uint64_t value) noexcept;

  static void UpdateFlags(ProcessorRegisters& registers, uint64_t operand1,
                          uint64_t operand2, uint64_t result, uint8_t size,
                          Operation operation) noexcept;
};

#endif  // INSTRUCTIONEMULATOR_H
