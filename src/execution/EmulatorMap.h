#ifndef EMULATORMAP_H
#define EMULATORMAP_H

#include <functional>
#include <memory>
#include <vector>

#include "ProcessState.h"

#include "../emulator/instruction/data/BtInstructionEmulator.h"
#include "../emulator/instruction/data/CmpInstructionEmulator.h"
#include "../emulator/instruction/data/LeaInstructionEmulator.h"
#include "../emulator/instruction/data/MovInstructionEmulator.h"
#include "../emulator/instruction/data/MovsxInstructionEmulator.h"
#include "../emulator/instruction/data/MovsxdInstructionEmulator.h"
#include "../emulator/instruction/data/MovzxInstructionEmulator.h"
#include "../emulator/instruction/data/TestInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovbInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovbeInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovlInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovleInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovnbeInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovnlInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovnleInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovnoInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovnpInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovnsInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovnzInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovoInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovpInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovsInstructionEmulator.h"
#include "../emulator/instruction/data/conditional/CmovzInstructionEmulator.h"
#include "../emulator/instruction/data/logical/AndInstructionEmulator.h"
#include "../emulator/instruction/data/logical/NegInstructionEmulator.h"
#include "../emulator/instruction/data/logical/NotInstructionEmulator.h"
#include "../emulator/instruction/data/logical/OrInstructionEmulator.h"
#include "../emulator/instruction/data/logical/SarInstructionEmulator.h"
#include "../emulator/instruction/data/logical/ShlInstructionEmulator.h"
#include "../emulator/instruction/data/logical/ShrInstructionEmulator.h"
#include "../emulator/instruction/data/logical/XorInstructionEmulator.h"
#include "../emulator/instruction/data/math/AddInstrucionEmulator.h"
#include "../emulator/instruction/data/math/DecInstructionEmulator.h"
#include "../emulator/instruction/data/math/IncInstructionEmulator.h"
#include "../emulator/instruction/data/math/SubInstructionEmulator.h"
#include "../emulator/instruction/flow/CallInstructionEmulator.h"
#include "../emulator/instruction/flow/JmpInstructionEmulator.h"
#include "../emulator/instruction/flow/RetInstructionEmulator.h"
#include "../emulator/instruction/flow/SyscallInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JbInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JbeInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JlInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JleInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JnbInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JnbeInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JnlInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JnleInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JnsInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JnzInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JoInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JsInstructionEmulator.h"
#include "../emulator/instruction/flow/conditional/JzInstructionEmulator.h"
#include "../emulator/instruction/special/Endbr32InstructionEmulator.h"
#include "../emulator/instruction/special/Endbr64InstructionEmulator.h"
#include "../emulator/instruction/special/NopInstructionEmulator.h"
#include "../emulator/syscall/io/CloseSyscallEmulator.h"
#include "../emulator/syscall/io/FcntlSyscallEmulator.h"
#include "../emulator/syscall/io/Getdents64SyscallEmulator.h"
#include "../emulator/syscall/io/IoctlSyscallEmulator.h"
#include "../emulator/syscall/io/OpenSyscallEmulator.h"
#include "../emulator/syscall/io/ReadSyscallEmulator.h"
#include "../emulator/syscall/io/WritevInstructionEmulator.h"
#include "../emulator/syscall/memory/BrkSyscallEmulator.h"
#include "../emulator/syscall/memory/MmapSyscallEmulator.h"
#include "../emulator/syscall/memory/MunmapSyscallEmulator.h"
#include "../emulator/syscall/process/ArchPrctlSyscallEmulator.h"
#include "../emulator/syscall/process/ExitGroupSyscallEmulator.h"
#include "../emulator/syscall/process/RseqSyscallEmulator.h"
#include "../emulator/syscall/process/SetRobustListSyscallEmulator.h"
#include "../emulator/syscall/process/SetTidAddressSyscallEmulator.h"
#include "../emulator/syscall/stat/GetegidSyscallEmulator.h"
#include "../emulator/syscall/stat/GeteuidSyscallEmulator.h"
#include "../emulator/syscall/stat/GetgidSyscallEmulator.h"
#include "../emulator/syscall/stat/GetuidSyscallEmulator.h"
#include "../emulator/syscall/stat/UnameSyscallEmulator.h"

// clang-format off
const std::vector<std::pair<ZydisMnemonic, std::function<std::unique_ptr<InstructionEmulator>(ProcessorRegisters&)>>> kInstructionEmulatorMap = { //NOLINT
    { ZYDIS_MNEMONIC_JMP,     [](ProcessorRegisters& registers) { return std::make_unique<JmpInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CALL,    [](ProcessorRegisters& registers) { return std::make_unique<CallInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JZ,      [](ProcessorRegisters& registers) { return std::make_unique<JzInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JNZ,     [](ProcessorRegisters& registers) { return std::make_unique<JnzInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_RET,     [](ProcessorRegisters& registers) { return std::make_unique<RetInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_SYSCALL, [](ProcessorRegisters& registers) { return std::make_unique<SyscallInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JL,      [](ProcessorRegisters& registers) { return std::make_unique<JlInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JNB,     [](ProcessorRegisters& registers) { return std::make_unique<JnbInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JNBE,    [](ProcessorRegisters& registers) { return std::make_unique<JnbeInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JS,      [](ProcessorRegisters& registers) { return std::make_unique<JsInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JB,      [](ProcessorRegisters& registers) { return std::make_unique<JbInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JBE,     [](ProcessorRegisters& registers) { return std::make_unique<JbeInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JNL,     [](ProcessorRegisters& registers) { return std::make_unique<JnlInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JNS,     [](ProcessorRegisters& registers) { return std::make_unique<JnsInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JNLE,    [](ProcessorRegisters& registers) { return std::make_unique<JnleInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JLE,     [](ProcessorRegisters& registers) { return std::make_unique<JleInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_JO,      [](ProcessorRegisters& registers) { return std::make_unique<JoInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_ENDBR32, [](ProcessorRegisters& registers) { return std::make_unique<Endbr32InstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_ENDBR64, [](ProcessorRegisters& registers) { return std::make_unique<Endbr64InstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_NOP,     [](ProcessorRegisters& registers) { return std::make_unique<NopInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMP,     [](ProcessorRegisters& registers) { return std::make_unique<CmpInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_LEA,     [](ProcessorRegisters& registers) { return std::make_unique<LeaInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_MOV,     [](ProcessorRegisters& registers) { return std::make_unique<MovInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_MOVSX,   [](ProcessorRegisters& registers) { return std::make_unique<MovsxInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_MOVSXD,  [](ProcessorRegisters& registers) { return std::make_unique<MovsxdInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_MOVZX,   [](ProcessorRegisters& registers) { return std::make_unique<MovzxInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_TEST,    [](ProcessorRegisters& registers) { return std::make_unique<TestInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_ADD,     [](ProcessorRegisters& registers) { return std::make_unique<AddInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_DEC,     [](ProcessorRegisters& registers) { return std::make_unique<DecInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_INC,     [](ProcessorRegisters& registers) { return std::make_unique<IncInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_SUB,     [](ProcessorRegisters& registers) { return std::make_unique<SubInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_AND,     [](ProcessorRegisters& registers) { return std::make_unique<AndInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_NEG,     [](ProcessorRegisters& registers) { return std::make_unique<NegInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_NOT,     [](ProcessorRegisters& registers) { return std::make_unique<NotInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_OR,      [](ProcessorRegisters& registers) { return std::make_unique<OrInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_SAR,     [](ProcessorRegisters& registers) { return std::make_unique<SarInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_SHL,     [](ProcessorRegisters& registers) { return std::make_unique<ShlInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_SHR,     [](ProcessorRegisters& registers) { return std::make_unique<ShrInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_XOR,     [](ProcessorRegisters& registers) { return std::make_unique<XorInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVBE,  [](ProcessorRegisters& registers) { return std::make_unique<CmovbeInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVB,   [](ProcessorRegisters& registers) { return std::make_unique<CmovbInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVLE,  [](ProcessorRegisters& registers) { return std::make_unique<CmovleInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVL,   [](ProcessorRegisters& registers) { return std::make_unique<CmovlInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVNBE, [](ProcessorRegisters& registers) { return std::make_unique<CmovnbeInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVNLE, [](ProcessorRegisters& registers) { return std::make_unique<CmovnleInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVNL,  [](ProcessorRegisters& registers) { return std::make_unique<CmovnlInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVNO,  [](ProcessorRegisters& registers) { return std::make_unique<CmovnoInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVNP,  [](ProcessorRegisters& registers) { return std::make_unique<CmovnpInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVNS,  [](ProcessorRegisters& registers) { return std::make_unique<CmovnsInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVNZ,  [](ProcessorRegisters& registers) { return std::make_unique<CmovnzInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVO,   [](ProcessorRegisters& registers) { return std::make_unique<CmovoInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVP,   [](ProcessorRegisters& registers) { return std::make_unique<CmovpInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVS,   [](ProcessorRegisters& registers) { return std::make_unique<CmovsInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_CMOVZ,   [](ProcessorRegisters& registers) { return std::make_unique<CmovzInstructionEmulator>(registers); } },
    { ZYDIS_MNEMONIC_BT,   [](ProcessorRegisters& registers) { return std::make_unique<BtInstructionEmulator>(registers); } }
};
// clang-format on

// clang-format off
const std::vector<std::pair<UnixSyscall, std::function<std::unique_ptr<SyscallEmulator>(std::shared_ptr<HighLevelEmulationContext>&)>>> kSyscallEmulatorMap = {
    { kSysClose, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<CloseSyscallEmulator>(hlec); } },
    { kSysFcntl, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<FcntlSyscallEmulator>(hlec); } },
    { kSysGetdents64, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<Getdents64SyscallEmulator>(hlec); } },
    { kSysIoctl, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<IoctlSyscallEmulator>(hlec); } },
    { kSysOpen, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<OpenSyscallEmulator>(hlec); } },
    { kSysRead, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<ReadSyscallEmulator>(hlec); } },
    { kSysWritev, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<WritevSyscallEmulator>(hlec); } },
    { kSysBrk, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<BrkSyscallEmulator>(hlec); } },
    { kSysMmap, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<MmapSyscallEmulator>(hlec); } },
    { kSysMunmap, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<MunmapSyscallEmulator>(hlec); } },
    { kSysArchPrctl, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<ArchPrctlSyscallEmulator>(hlec); } },
    { kSysExitGroup, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<ExitGroupSyscallEmulator>(hlec); } },
    { kSysRseq, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<RseqSyscallEmulator>(hlec); } },
    { kSysSetRobustList, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<SetRobustListSyscallEmulator>(hlec); } },
    { kSysSetTidAddress, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<SetTidAddressSyscallEmulator>(hlec); } },
    { kSysGetegid, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<GetegidSyscallEmulator>(hlec); } },
    { kSysGeteuid, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<GeteuidSyscallEmulator>(hlec); } },
    { kSysGetgid, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<GetgidSyscallEmulator>(hlec); } },
    { kSysGetuid, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<GetuidSyscallEmulator>(hlec); } },
    { kSysUname, [](std::shared_ptr<HighLevelEmulationContext>& hlec) { return std::make_unique<UnameSyscallEmulator>(hlec); } },
};
// clang-format on

#endif //EMULATORMAP_H
