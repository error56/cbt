#ifndef PROCESSSTATE_H
#define PROCESSSTATE_H

#include "BasicBlockAllocator.h"

struct ProcessorRegisters;
struct HighLevelEmulationContext;
struct DecodedInstruction;
struct ContextSwitchStubInfo;
struct TranslationCacheEntry;
struct TranslationContext;
class BasicBlock;

#include "../interception/SyscallInterceptor.h"
#include "../interception/VfsAccessInterceptor.h"
#include "../network/IdaTracingPluginServer.h"
#include "../virtualization/fs/VfsRoot.h"
#include "FileDescriptorTable.h"
#include "ProcessState.h"
#include "Zydis/DecoderTypes.h"

#pragma pack(push, 1)
// Structure holding the whole state of the CPU.
// Used to save state of guest or emulator during the context-switch.
// The offsets and order of fields is very important, cause instruction
// emulation functions manipulate guest state by writing raw values to the
// registers.
struct ProcessorRegisters {
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rbp;
  uint64_t rsp;
  uint64_t rsi;
  uint64_t rdi;

  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;

  uint64_t rip;
  uint64_t rflags;

  uint64_t fs;
  uint64_t gs;

  // @TODO: AVX-512? AVX-2? SSE?
};
#pragma pack(pop)

constexpr size_t kRaxOffset = 0x00;
constexpr size_t kRbxOffset = 0x08;
constexpr size_t kRcxOffset = 0x10;
constexpr size_t kRdxOffset = 0x18;
constexpr size_t kRbpOffset = 0x20;
constexpr size_t kRspOffset = 0x28;
constexpr size_t kRsiOffset = 0x30;
constexpr size_t kRdiOffset = 0x38;
constexpr size_t kR8Offset = 0x40;
constexpr size_t kR9Offset = 0x48;
constexpr size_t kR10Offset = 0x50;
constexpr size_t kR11Offset = 0x58;
constexpr size_t kR12Offset = 0x60;
constexpr size_t kR13Offset = 0x68;
constexpr size_t kR14Offset = 0x70;
constexpr size_t kR15Offset = 0x78;
constexpr size_t kRipOffset = 0x80;
constexpr size_t kRflagsOffset = 0x88;
constexpr size_t kFsOffset = 0x90;
constexpr size_t kGsOffset = 0x98;

// We REALLY need to make sure offsets didn't change.
static_assert(offsetof(ProcessorRegisters, rax) == kRaxOffset);
static_assert(offsetof(ProcessorRegisters, rbx) == kRbxOffset);
static_assert(offsetof(ProcessorRegisters, rcx) == kRcxOffset);
static_assert(offsetof(ProcessorRegisters, rdx) == kRdxOffset);
static_assert(offsetof(ProcessorRegisters, rbp) == kRbpOffset);
static_assert(offsetof(ProcessorRegisters, rsp) == kRspOffset);
static_assert(offsetof(ProcessorRegisters, rsi) == kRsiOffset);
static_assert(offsetof(ProcessorRegisters, rdi) == kRdiOffset);

static_assert(offsetof(ProcessorRegisters, r8) == kR8Offset);
static_assert(offsetof(ProcessorRegisters, r9) == kR9Offset);
static_assert(offsetof(ProcessorRegisters, r10) == kR10Offset);
static_assert(offsetof(ProcessorRegisters, r11) == kR11Offset);
static_assert(offsetof(ProcessorRegisters, r12) == kR12Offset);
static_assert(offsetof(ProcessorRegisters, r13) == kR13Offset);
static_assert(offsetof(ProcessorRegisters, r14) == kR14Offset);
static_assert(offsetof(ProcessorRegisters, r15) == kR15Offset);

static_assert(offsetof(ProcessorRegisters, rip) == kRipOffset);
static_assert(offsetof(ProcessorRegisters, rflags) == kRflagsOffset);

static_assert(offsetof(ProcessorRegisters, fs) == kFsOffset);
static_assert(offsetof(ProcessorRegisters, gs) == kGsOffset);

// Context structure containing information used by the high level
// functionalities of CBT, like virtual file system emulation or mmap() syscall
// emulation.
struct HighLevelEmulationContext {
  // User registered syscall interceptors.
  std::shared_ptr<SyscallInterceptor> syscall_interceptors;

  // Debugging service pushing address of currently executed basic block to
  // external disassembler.
  std::shared_ptr<IdaTracingPluginServer> tracing_server;

  // File descriptor table containing pointers to various objects (mostly
  // connected with virtual file system).
  std::shared_ptr<FileDescriptorTable> file_descriptor_table;

  // Pointer to the root of the virtual file system.
  std::shared_ptr<VfsRoot> vfs_root;

  // User-provided interceptors for virtual file system access.
  std::shared_ptr<VfsAccessInterceptor> vfs_access_interceptor;

  // Current working directory, relative to the root of the virtual file system.
  std::string cwd;

  // Start of the guest heap.
  uint64_t brk_start;

  // End of the guest heap.
  uint64_t brk_end;

  // Program arguments.
  const std::vector<std::string> args;

  // Environment variables.
  const std::vector<std::string> env;
};

// Structure describing decoded instruction of guest code, as well as its
// operands and address.
struct DecodedInstruction {
  // Decoded instruction.
  ZydisDecodedInstruction instruction;

  // Instruction operands.
  std::array<ZydisDecodedOperand, ZYDIS_MAX_OPERAND_COUNT> operands;

  // Instruction address.
  uint64_t address;

  // Instruction size in bytes.
  uint8_t size;
};

using ContextSwitchFunction = void (*)();

// Structure describing the merging ability of current basic block.
struct MergingInfo {
  // Informs whether merging operation can be performed on this basic block.
  bool merging_available;

  // Informs whether merging operation was already performed on this basic
  // block.
  bool merging_performed;

  // Address of next instruction to decode.
  uint64_t instruction_address_at_the_end;

  // Pointer to the last instruction of the translated basic block.
  void* jump_address;
};

// Contains info about context switch.
struct ContextSwitchStubInfo {
  // Pointer to the start of the context switch the emulator can jump to.
  ContextSwitchFunction code;

  // Pointer to the last instruction of the translated basic block.
  void* last_instruction_address;
};

// Cache entry holding information about basic block.
struct TranslationCacheEntry {
  // Pointer to context switch code.
  ContextSwitchStubInfo context_switch;

  // Size of the basic block in bytes.
  uint64_t basic_block_size;

  // Does the basic block requires FS/GS segment switch.
  bool requires_segment_switch;

  // Instruction that broke the basic block.
  DecodedInstruction last_instruction;

  // Whether the basic block is empty (contains only one instruction, that broke
  // the basic block).
  bool is_not_empty;

  // Whether the basic block is trivial.
  bool is_trivial;

  // Pointer to BasicBlock structure containing all instructions.
  std::shared_ptr<BasicBlock> instructions;

  // Merging-related info.
  MergingInfo merging_info;

  // Address of next instruction to decode.
  uint64_t instruction_address_at_the_end;
};

// Translation context shared between all threads in the process.
struct TranslationContext {
  // Translation cache containing already translated basic blocks that are ready
  // to execute.
  std::shared_ptr<std::map<uint64_t, std::shared_ptr<TranslationCacheEntry>>>
      translation_cache;

  // Allocator of translated basic blocks.
  std::shared_ptr<BasicBlockAllocator> basic_block_allocator;
};

#endif  // PROCESSSTATE_H
