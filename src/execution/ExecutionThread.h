#ifndef EXECUTIONTHREAD_H
#define EXECUTIONTHREAD_H

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include "../emulator/instruction/InstructionEmulator.h"
#include "../emulator/syscall/SyscallEmulator.h"
#include "../generator/ContextSwitchGenerator.h"
#include "../interception/SyscallInterceptor.h"
#include "../linker/ElfLoader.h"
#include "../network/IdaTracingPluginServer.h"
#include "../virtualization/fs/VfsRoot.h"
#include "BasicBlockDecoder.h"
#include "ProcessState.h"
#include "Zydis/Encoder.h"
#include "zasm/serialization/serializer.hpp"

// Entire processor state - all CPU registers, both for emulator and guest
// state.
struct ProcessorState {
  // Emulator CPU state.
  ProcessorRegisters emulator_state;

  // Guest CPU state.
  ProcessorRegisters guest_state;
};

constexpr uint64_t kRflagsCarryFlagShift = 0;
constexpr uint64_t kRflagsCarryFlagMask = 1 << kRflagsCarryFlagShift;

constexpr uint64_t kRflagsParityFlagShift = 2;
constexpr uint64_t kRflagsParityFlagMask = 1 << kRflagsParityFlagShift;

constexpr uint64_t kRflagsAuxiliaryFlagShift = 4;
constexpr uint64_t kRflagsAuxiliaryFlagMask = 1 << kRflagsAuxiliaryFlagShift;

constexpr uint64_t kRflagsZeroFlagShift = 6;
constexpr uint64_t kRflagsZeroFlagMask = 1 << kRflagsZeroFlagShift;

constexpr uint64_t kRflagsSignFlagShift = 7;
constexpr uint64_t kRflagsSignFlagMask = 1 << kRflagsSignFlagShift;

constexpr uint64_t kRflagsDirectionFlagShift = 10;
constexpr uint64_t kRflagsDirectionFlagMask = 1 << kRflagsDirectionFlagShift;

constexpr uint64_t kRflagsOverflowFlagShift = 11;
constexpr uint64_t kRflagsOverflowFlagMask = 1 << kRflagsOverflowFlagShift;

constexpr uint64_t kRflagsReserved = 1 << 1;
constexpr uint64_t kRflagsCpl3 = 0b11 << 12;

struct FileDescriptorFileEntry {
  std::shared_ptr<VfsEntry> entry;
  uint64_t position;
};

constexpr uint64_t kKilo = 1024;
constexpr uint64_t kMega = 1024 * kKilo;

// Stack size in megabytes
constexpr uint64_t kStackSize = 16 * kMega;  // 16mb

// Opcode of rel jmp32
constexpr uint8_t kRelativeJumpOpcode = 0xE9;

// @TODO: Check if we can use `std::is_arithmetic` here safely.
template <typename T>
concept Numeric = std::is_integral_v<T>;

// Enum used to pass information about emulated instruction and update flags in
// RFLAGS registers, according to the operands and operation type.
enum class EmulatedInstructionForFlagUpdate : uint8_t {
  kAdd,
  kSub,
  kAnd,
  kOr,
  kXor,
  kInc,
  kDec,
  kNeg,
  kShl,
  kShr,
  kSar,
  kTest,
  kCmp
};

class ExecutionThread {
  // Syscall emulator needs to have access to `syscall_interceptors_`
  friend class SyscallInstructionEmulator;

  // Call emulator needs to access internal thread management routines, like
  // pushing values to the stack.
  friend class CallInstructionEmulator;

  // Ret emulator needs to access internal thread management routines, like
  // popping values from the stack.
  friend class RetInstructionEmulator;

  // Pointer to loader of main executable
  std::shared_ptr<ElfLoader> binary_;

  // Emulators of basic-block breaking instructions
  std::map<ZydisMnemonic, std::unique_ptr<InstructionEmulator>>
      instruction_emulators_{};

  // Emulators of system calls
  std::map<UnixSyscall, std::unique_ptr<SyscallEmulator>> syscall_emulators_{};

  // Current call-stack (used by CALL and RET emulator)
  std::stack<std::pair<uint64_t, uint64_t>> call_stack_;

  // Generator of context switch routine
  ContextSwitchGenerator generator_;

  // Stack of current thread
  void* stack_{};

  // Flag used to determine if the thread should end its execution at the next
  // basic block
  bool should_exit_{false};

  // Address of currently processed instruction by the translator. Most of the
  // time it contains last instruction of the currently processed basic block.
  uint64_t current_instruction_address_;

  // Network server broadcasting currently executed instruction.
  std::shared_ptr<IdaTracingPluginServer> tracing_server_;

  // Context structure used by syscall emulators for managing the internal
  // state of VFS emulation.
  std::shared_ptr<HighLevelEmulationContext> hlec_;

  // Context structure used by translation loop.
  std::shared_ptr<TranslationContext> translation_context_;

  // Generated stub used to context switch between host and guest code.
  zasm::Serializer context_switch_stub_;

  // Decodes basic block starting at given address and prepares for emulation.
  BasicBlockDecoder decoder_{};

  // Processor registers of current thread
  ProcessorState processor_state_{};

  // Initializes thread context, allocates the stack and sets starting register
  // values.
  [[nodiscard]] bool InitializeThreadContext() noexcept;

  // Performs FS/GS segment switch from guest to host. It is performed by a
  // special, `arch_prctl`, syscall, so context switch stub generated at the
  // start can't really perform it.
  void PerformSegmentSwitch(bool entry_to_emulation) noexcept;

  // Finds syscall emulator and calls it with current thread state to emulate a
  // syscall.
  [[nodiscard]] InterceptionResult HandleSyscall(UnixSyscall syscall) noexcept;

  // Allocates context switch stub added at the end of the basic block, that
  // returns to context switch function and returns to the translation loop.
  void SetupContextSwitchStub() noexcept;

  // Emulates trivial basic block and executes instructions without unnecessary
  // context switch. Returns true on success and false otherwise. This function
  // failure is currently considered as critical error.
  //
  // @TODO: Figure out if emulation failure can fallback into normal execution
  // path, but need to take care of modified state.
  [[nodiscard]] bool EmulateTrivialBasicBlock(
      std::vector<DecodedInstruction>& instructions) noexcept;

  // Gets translation cache entry from current basic block, or creates one if
  // the basic block is extracted for the very first time.
  [[nodiscard]] std::shared_ptr<TranslationCacheEntry>
  GetTranslationCacheEntryFromCurrentBasicBlock() noexcept;

  // Tries to merge two basic blocks into one by inserting an unconditional jump
  // at the end of the first basic block, to the start of the next block.
  //
  // This function does not return anything, because if also modifies
  // TranslationCacheEntry associated with first basic block.
  bool TryToMergeBasicBlock(
      const std::shared_ptr<TranslationCacheEntry>& entry) noexcept;

  // Tries to extract effective address of the jump instruction using the
  // provided operand and registers state.
  [[nodiscard]] uint64_t GetJumpTarget(
      const DecodedInstruction& instruction) const noexcept;

  // Pushes value onto the stack and fixes RSP.
  template <typename T>
    requires Numeric<T>
  void PushToGuestStack(T value) noexcept;

  // Pops value from guest stack and does not return it.
  void PopFromGuestStack(uint8_t size) noexcept;

 public:
  // Starts thread execution.
  void Run();

  // Stops the execution.
  void Stop() noexcept;

  // Performs a context switch to guest state, executes a `syscall` instruction
  // and returns to the translation loop. Used by the syscall emulators to pass
  // the syscall for the real operating system.
  void PerformSyscall() noexcept;

  // Returns reference to structure containing both guest and host registers.
  [[nodiscard]] ProcessorState& GetProcessorState() noexcept;

  // Returns reference to structure containing state of all guest registers.
  [[nodiscard]] ProcessorRegisters& GetGuestRegisters() noexcept;

  explicit ExecutionThread(
      std::shared_ptr<ElfLoader> binary,
      uint64_t starting_instruction_address,
      const std::shared_ptr<HighLevelEmulationContext>& hlec,
      const std::shared_ptr<TranslationContext>& translation_context);
};

#endif  // EXECUTIONTHREAD_H
