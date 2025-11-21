#include "ContextSwitchGenerator.h"

#include <sys/mman.h>

#include "../execution/ExecutionThread.h"
#include "zasm/program/program.hpp"
#include "zasm/serialization/serializer.hpp"
#include "zasm/x86/assembler.hpp"
#include "zasm/x86/memory.hpp"

void ContextSwitchGenerator::
    GenerateContextSwitchFromTranslatorToGuest() noexcept {
  using namespace zasm::x86;  // NOLINT

  zasm::Program program(zasm::MachineMode::AMD64);
  Assembler assembler(program);

  // Push current RAX at the stack
  assembler.push(rax);

  // Move address of structure containing emulator state to the RAX
  assembler.mov(
      rax, zasm::Imm64(reinterpret_cast<uint64_t>(&cpu_state_.emulator_state)));

  // Save emulator state
  assembler.mov(qword_ptr(rax, kRbxOffset), rbx);
  assembler.mov(qword_ptr(rax, kRcxOffset), rcx);
  assembler.mov(qword_ptr(rax, kRdxOffset), rdx);
  assembler.mov(qword_ptr(rax, kRbpOffset), rbp);
  assembler.mov(qword_ptr(rax, kRspOffset), rsp);
  assembler.mov(qword_ptr(rax, kRsiOffset), rsi);
  assembler.mov(qword_ptr(rax, kRdiOffset), rdi);
  assembler.mov(qword_ptr(rax, kR8Offset), r8);
  assembler.mov(qword_ptr(rax, kR9Offset), r9);
  assembler.mov(qword_ptr(rax, kR10Offset), r10);
  assembler.mov(qword_ptr(rax, kR11Offset), r11);
  assembler.mov(qword_ptr(rax, kR12Offset), r12);
  assembler.mov(qword_ptr(rax, kR13Offset), r13);
  assembler.mov(qword_ptr(rax, kR14Offset), r14);
  assembler.mov(qword_ptr(rax, kR15Offset), r15);

  // RDX is saved now in context_switch_data_.emulator_state, so can freely
  // save current RAX (pointer to the structure) in it.
  //
  // Save pointer into RDX
  assembler.mov(rdx, rax);
  // Push RFLAGS into the stack
  assembler.pushfq();
  // Pop RFLAGS into RAX
  assembler.pop(rax);
  // Save RFLAGS
  assembler.mov(qword_ptr(rdx, kRflagsOffset), rax);

  // Pop RAX value (saved at the beginning) and save it in emulator_state.
  assembler.pop(rax);
  assembler.mov(qword_ptr(rdx, kRaxOffset), rax);
  assembler.mov(qword_ptr(rdx, kRspOffset), rsp);

  assembler.nop();
  assembler.nop();
  assembler.nop();

  //
  // Load guest state
  //

  // Set RAX to address of translator_state
  assembler.mov(
      rax, zasm::Imm64(reinterpret_cast<uint64_t>(&cpu_state_.guest_state)));

  // Set RFLAGS
  assembler.mov(rcx, qword_ptr(rax, kRflagsOffset));
  assembler.push(rcx);
  assembler.popfq();

  // Restore registers
  assembler.mov(rbx, qword_ptr(rax, kRbxOffset));
  assembler.mov(rcx, qword_ptr(rax, kRcxOffset));
  assembler.mov(rdx, qword_ptr(rax, kRdxOffset));
  assembler.mov(rbp, qword_ptr(rax, kRbpOffset));
  assembler.mov(rsp, qword_ptr(rax, kRspOffset));
  assembler.mov(rsi, qword_ptr(rax, kRsiOffset));
  assembler.mov(rdi, qword_ptr(rax, kRdiOffset));
  assembler.mov(r8, qword_ptr(rax, kR8Offset));
  assembler.mov(r9, qword_ptr(rax, kR9Offset));
  assembler.mov(r10, qword_ptr(rax, kR10Offset));
  assembler.mov(r11, qword_ptr(rax, kR11Offset));
  assembler.mov(r12, qword_ptr(rax, kR12Offset));
  assembler.mov(r13, qword_ptr(rax, kR13Offset));
  assembler.mov(r14, qword_ptr(rax, kR14Offset));
  assembler.mov(r15, qword_ptr(rax, kR15Offset));

  assembler.mov(rax, qword_ptr(rax, kRaxOffset));

  assembler.ret();

  zasm::Serializer serializer{};
  serializer.serialize(program, 0x0);  // base does not really matter, as we
                                       // don't have any relative offsets here

  auto* code_memory = mmap(nullptr, serializer.getCodeSize(),
                           PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  std::memcpy(code_memory, serializer.getCode(), serializer.getCodeSize());
  context_switch_from_translator_to_guest = code_memory;
  context_switch_from_translator_to_guest_allocation_size_ =
      serializer.getCodeSize();
}

void ContextSwitchGenerator::
    GenerateContextSwitchFromGuestToTranslator() noexcept {
  using namespace zasm::x86;  // NOLINT

  zasm::Program program(zasm::MachineMode::AMD64);
  Assembler assembler(program);

  // RAX is currently on the stack (pushed by
  // ExecutionThread::AllocateContextSwitch)

  // Move address of structure containing emulator state to the RAX
  assembler.mov(
      rax, zasm::Imm64(reinterpret_cast<uint64_t>(&cpu_state_.guest_state)));

  // Save emulator state
  assembler.mov(qword_ptr(rax, kRbxOffset), rbx);
  assembler.mov(qword_ptr(rax, kRcxOffset), rcx);
  assembler.mov(qword_ptr(rax, kRdxOffset), rdx);
  assembler.mov(qword_ptr(rax, kRbpOffset), rbp);
  assembler.mov(qword_ptr(rax, kRspOffset), rsp);
  assembler.mov(qword_ptr(rax, kRsiOffset), rsi);
  assembler.mov(qword_ptr(rax, kRdiOffset), rdi);
  assembler.mov(qword_ptr(rax, kR8Offset), r8);
  assembler.mov(qword_ptr(rax, kR9Offset), r9);
  assembler.mov(qword_ptr(rax, kR10Offset), r10);
  assembler.mov(qword_ptr(rax, kR11Offset), r11);
  assembler.mov(qword_ptr(rax, kR12Offset), r12);
  assembler.mov(qword_ptr(rax, kR13Offset), r13);
  assembler.mov(qword_ptr(rax, kR14Offset), r14);
  assembler.mov(qword_ptr(rax, kR15Offset), r15);

  // RDX is saved now in context_switch_data_.emulator_state, so can freely
  // save current RAX (pointer to the structure) in it.
  //
  // Save pointer into RDX
  assembler.mov(rdx, rax);
  // Push RFLAGS onto the stack
  assembler.pushfq();
  // Pop RFLAGS into RAX
  assembler.pop(rax);
  // Save RFLAGS
  assembler.mov(qword_ptr(rdx, kRflagsOffset), rax);

  // Pop RAX value (saved at the beginning) and save it in emulator_state.
  assembler.pop(rax);
  assembler.mov(qword_ptr(rdx, kRaxOffset), rax);
  assembler.mov(qword_ptr(rdx, kRspOffset), rsp);

  assembler.nop();
  assembler.nop();
  assembler.nop();

  //
  // Load guest state
  //

  // Set RAX to address of translator_state
  assembler.mov(
      rax, zasm::Imm64(reinterpret_cast<uint64_t>(&cpu_state_.emulator_state)));

  // Set RFLAGS
  assembler.mov(rcx, qword_ptr(rax, kRflagsOffset));
  assembler.push(rcx);
  assembler.popfq();

  // Restore registers
  assembler.mov(rbx, qword_ptr(rax, kRbxOffset));
  assembler.mov(rcx, qword_ptr(rax, kRcxOffset));
  assembler.mov(rdx, qword_ptr(rax, kRdxOffset));
  assembler.mov(rbp, qword_ptr(rax, kRbpOffset));
  assembler.mov(rsp, qword_ptr(rax, kRspOffset));
  assembler.mov(rsi, qword_ptr(rax, kRsiOffset));
  assembler.mov(rdi, qword_ptr(rax, kRdiOffset));
  assembler.mov(r8, qword_ptr(rax, kR8Offset));
  assembler.mov(r9, qword_ptr(rax, kR9Offset));
  assembler.mov(r10, qword_ptr(rax, kR10Offset));
  assembler.mov(r11, qword_ptr(rax, kR11Offset));
  assembler.mov(r12, qword_ptr(rax, kR12Offset));
  assembler.mov(r13, qword_ptr(rax, kR13Offset));
  assembler.mov(r14, qword_ptr(rax, kR14Offset));
  assembler.mov(r15, qword_ptr(rax, kR15Offset));

  assembler.mov(rax, qword_ptr(rax, kRflagsOffset));

  assembler.ret();

  zasm::Serializer serializer{};
  serializer.serialize(program, 0x0);  // base does not really matter, as we
                                       // don't have any relative offsets here

  auto* code_memory = mmap(nullptr, serializer.getCodeSize(),
                           PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  std::memcpy(code_memory, serializer.getCode(), serializer.getCodeSize());
  context_switch_from_guest_to_translator = code_memory;
  context_switch_from_guest_to_translator_allocation_size_ =
      serializer.getCodeSize();
}
