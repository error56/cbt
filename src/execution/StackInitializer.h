#ifndef STACKINITIALIZER_H
#define STACKINITIALIZER_H

#include <string>
#include <vector>

#include "ExecutionThread.h"

// Initializes stack for thread execution.
class StackInitializer {
  // Vector of arguments passed to the emulated program.
  std::vector<std::string> argv_;

  // Vector of environment variables passed to the emulated program.
  std::vector<std::string> envp_;

  // Aligns the stack to the 8 bytes boundary.
  static void AlignTo8Bytes(ProcessorRegisters& registers) noexcept;

  // Pushes byte to the stack.
  static void PushByte(uint8_t byte, ProcessorRegisters& registers) noexcept;

  // Pushes qword (8 bytes) to the stack.
  static void PushQword(uint64_t qword, ProcessorRegisters& registers) noexcept;

  // Pushes string to the stack.
  static void PushString(const char* str, ProcessorRegisters& registers) noexcept;

  // Pushes string array in reversed order to the guest stack.
  [[nodiscard]] std::vector<uint64_t> PushStringArray(
      const std::vector<std::string>& strings,
      ProcessorRegisters& guest_registers) const noexcept;

  // Pushes string and returns address.
  [[nodiscard]] static uint64_t PushStringAndGetAddress(
      const char* str,
      ProcessorRegisters& guest_registers) noexcept;

 public:
  StackInitializer(const std::vector<std::string>& argv,
                   const std::vector<std::string>& envp)
      : argv_(argv), envp_(envp) {};

  // Initializes guest stack with derived argv and envp
  void Initialize(ProcessorRegisters& guest_registers,
                  uint64_t mapped_file_base) const noexcept;
};

#endif  // STACKINITIALIZER_H
