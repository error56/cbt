cmake_minimum_required(VERSION 3.22.1)

project(cbt)

include(FetchContent)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG 8e5613379f5140fefb0b60412fbf1f5406e7c7f8 # 1.15.0
)

FetchContent_Declare(
        zydis
        GIT_REPOSITORY https://github.com/zyantific/zydis
        GIT_TAG 569320ad3c4856da13b9dbf1f0d9e20bda63870e # 4.1.0
)

FetchContent_Declare(
        zasm
        GIT_REPOSITORY https://github.com/zyantific/zasm
        GIT_TAG 916f28f882801c048eaececc2466c8fdc17653fa
)

FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-28-1
)
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_COMMAND_LINE_TOOLS OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

FetchContent_MakeAvailable(spdlog zydis zasm)

add_executable(cbt
        src/main.cpp
        src/util/Option.h
        src/util/Result.h

        src/generator/ContextSwitchGenerator.cc
        src/generator/ContextSwitchGenerator.h

        src/execution/ExecutionThread.cc
        src/execution/ExecutionThread.h
        src/execution/FileDescriptorTable.cc
        src/execution/FileDescriptorTable.h
        src/execution/ProcessState.h
        src/execution/BasicBlockAllocator.cc
        src/execution/BasicBlockAllocator.h
        src/execution/BasicBlock.cc
        src/execution/BasicBlock.h
        src/execution/BasicBlockDecoder.cc
        src/execution/BasicBlockDecoder.h
        src/execution/StackInitializer.cc
        src/execution/StackInitializer.h
        src/execution/EmulatorMap.h

        src/CbtBinary.cc
        src/CbtBinary.h

        src/emulator/instruction/InstructionEmulator.cc
        src/emulator/instruction/InstructionEmulator.h

        src/emulator/instruction/flow/JmpInstructionEmulator.cc
        src/emulator/instruction/flow/JmpInstructionEmulator.h
        src/emulator/instruction/flow/CallInstructionEmulator.cc
        src/emulator/instruction/flow/CallInstructionEmulator.h
        src/emulator/instruction/flow/RetInstructionEmulator.cc
        src/emulator/instruction/flow/RetInstructionEmulator.h
        src/emulator/instruction/flow/SyscallInstructionEmulator.cc
        src/emulator/instruction/flow/SyscallInstructionEmulator.h

        src/emulator/instruction/flow/conditional/JzInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JzInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JnzInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JnzInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JlInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JlInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JnbInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JnbInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JnbeInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JnbeInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JsInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JsInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JbInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JbInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JbeInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JbeInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JnlInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JnlInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JnsInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JnsInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JnleInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JnleInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JleInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JleInstructionEmulator.h
        src/emulator/instruction/flow/conditional/JoInstructionEmulator.cc
        src/emulator/instruction/flow/conditional/JoInstructionEmulator.h

        src/emulator/instruction/data/MovInstructionEmulator.cc
        src/emulator/instruction/data/MovInstructionEmulator.h
        src/emulator/instruction/data/MovzxInstructionEmulator.cc
        src/emulator/instruction/data/MovzxInstructionEmulator.h
        src/emulator/instruction/data/MovsxInstructionEmulator.cc
        src/emulator/instruction/data/MovsxInstructionEmulator.h
        src/emulator/instruction/data/MovsxdInstructionEmulator.cc
        src/emulator/instruction/data/MovsxdInstructionEmulator.h
        src/emulator/instruction/data/LeaInstructionEmulator.cc
        src/emulator/instruction/data/LeaInstructionEmulator.h
        src/emulator/instruction/data/TestInstructionEmulator.cc
        src/emulator/instruction/data/TestInstructionEmulator.h
        src/emulator/instruction/data/CmpInstructionEmulator.cc
        src/emulator/instruction/data/CmpInstructionEmulator.h
        src/emulator/instruction/data/BtInstructionEmulator.cc
        src/emulator/instruction/data/BtInstructionEmulator.h

        src/emulator/instruction/data/math/AddInstrucionEmulator.cc
        src/emulator/instruction/data/math/AddInstrucionEmulator.h
        src/emulator/instruction/data/math/SubInstructionEmulator.cc
        src/emulator/instruction/data/math/SubInstructionEmulator.h
        src/emulator/instruction/data/math/IncInstructionEmulator.cc
        src/emulator/instruction/data/math/IncInstructionEmulator.h
        src/emulator/instruction/data/math/DecInstructionEmulator.cc
        src/emulator/instruction/data/math/DecInstructionEmulator.h

        src/emulator/instruction/data/logical/AndInstructionEmulator.cc
        src/emulator/instruction/data/logical/AndInstructionEmulator.h
        src/emulator/instruction/data/logical/OrInstructionEmulator.cc
        src/emulator/instruction/data/logical/OrInstructionEmulator.h
        src/emulator/instruction/data/logical/XorInstructionEmulator.cc
        src/emulator/instruction/data/logical/XorInstructionEmulator.h
        src/emulator/instruction/data/logical/NegInstructionEmulator.cc
        src/emulator/instruction/data/logical/NegInstructionEmulator.h
        src/emulator/instruction/data/logical/NotInstructionEmulator.cc
        src/emulator/instruction/data/logical/NotInstructionEmulator.h
        src/emulator/instruction/data/logical/ShlInstructionEmulator.cc
        src/emulator/instruction/data/logical/ShlInstructionEmulator.h
        src/emulator/instruction/data/logical/ShrInstructionEmulator.cc
        src/emulator/instruction/data/logical/ShrInstructionEmulator.h
        src/emulator/instruction/data/logical/SarInstructionEmulator.cc
        src/emulator/instruction/data/logical/SarInstructionEmulator.h

        src/emulator/instruction/data/conditional/CmovnbeInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovnbeInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovbInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovbInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovbeInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovbeInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovzInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovzInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovnleInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovnleInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovnlInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovnlInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovlInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovlInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovleInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovleInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovnzInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovnzInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovnoInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovnoInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovnpInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovnpInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovnsInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovnsInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovoInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovoInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovpInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovpInstructionEmulator.h
        src/emulator/instruction/data/conditional/CmovsInstructionEmulator.cc
        src/emulator/instruction/data/conditional/CmovsInstructionEmulator.h

        src/emulator/instruction/special/NopInstructionEmulator.cc
        src/emulator/instruction/special/NopInstructionEmulator.h
        src/emulator/instruction/special/Endbr32InstructionEmulator.cc
        src/emulator/instruction/special/Endbr32InstructionEmulator.h
        src/emulator/instruction/special/Endbr64InstructionEmulator.cc
        src/emulator/instruction/special/Endbr64InstructionEmulator.h

        src/emulator/syscall/SyscallEmulator.cc
        src/emulator/syscall/SyscallEmulator.h

        src/emulator/syscall/io/IoctlSyscallEmulator.cc
        src/emulator/syscall/io/IoctlSyscallEmulator.h
        src/emulator/syscall/io/FcntlSyscallEmulator.cc
        src/emulator/syscall/io/FcntlSyscallEmulator.h
        src/emulator/syscall/io/OpenSyscallEmulator.cc
        src/emulator/syscall/io/OpenSyscallEmulator.h
        src/emulator/syscall/io/CloseSyscallEmulator.cc
        src/emulator/syscall/io/CloseSyscallEmulator.h
        src/emulator/syscall/io/ReadSyscallEmulator.cc
        src/emulator/syscall/io/ReadSyscallEmulator.h
        src/emulator/syscall/io/WritevInstructionEmulator.cc
        src/emulator/syscall/io/WritevInstructionEmulator.h
        src/emulator/syscall/io/Getdents64SyscallEmulator.cc
        src/emulator/syscall/io/Getdents64SyscallEmulator.h

        src/emulator/syscall/memory/MmapSyscallEmulator.cc
        src/emulator/syscall/memory/MmapSyscallEmulator.h
        src/emulator/syscall/memory/MunmapSyscallEmulator.cc
        src/emulator/syscall/memory/MunmapSyscallEmulator.h
        src/emulator/syscall/memory/BrkSyscallEmulator.cc
        src/emulator/syscall/memory/BrkSyscallEmulator.h

        src/emulator/syscall/process/RseqSyscallEmulator.cc
        src/emulator/syscall/process/RseqSyscallEmulator.h
        src/emulator/syscall/process/SetRobustListSyscallEmulator.cc
        src/emulator/syscall/process/SetRobustListSyscallEmulator.h
        src/emulator/syscall/process/SetTidAddressSyscallEmulator.cc
        src/emulator/syscall/process/SetTidAddressSyscallEmulator.h
        src/emulator/syscall/process/ExitGroupSyscallEmulator.cc
        src/emulator/syscall/process/ExitGroupSyscallEmulator.h
        src/emulator/syscall/process/ArchPrctlSyscallEmulator.cc
        src/emulator/syscall/process/ArchPrctlSyscallEmulator.h

        src/emulator/syscall/stat/UnameSyscallEmulator.cc
        src/emulator/syscall/stat/UnameSyscallEmulator.h
        src/emulator/syscall/stat/GeteuidSyscallEmulator.cc
        src/emulator/syscall/stat/GeteuidSyscallEmulator.h
        src/emulator/syscall/stat/GetuidSyscallEmulator.cc
        src/emulator/syscall/stat/GetuidSyscallEmulator.h
        src/emulator/syscall/stat/GetegidSyscallEmulator.cc
        src/emulator/syscall/stat/GetegidSyscallEmulator.h
        src/emulator/syscall/stat/GetgidSyscallEmulator.cc
        src/emulator/syscall/stat/GetgidSyscallEmulator.h

        src/interception/SyscallInterceptor.cc
        src/interception/SyscallInterceptor.h
        src/interception/SyscallId.h
        src/interception/VfsAccessInterceptor.cc
        src/interception/VfsAccessInterceptor.h

        src/linker/ElfLoader.cc
        src/linker/ElfLoader.h
        src/linker/LinuxOpenedFile.cc
        src/linker/LinuxOpenedFile.h

        src/virtualization/fs/VfsEntry.cc
        src/virtualization/fs/VfsEntry.h
        src/virtualization/fs/VfsRoot.cc
        src/virtualization/fs/VfsRoot.h
        src/virtualization/fs/VfsFile.cc
        src/virtualization/fs/VfsFile.h
        src/virtualization/fs/VfsDirectory.cc
        src/virtualization/fs/VfsDirectory.h

        # Network
        src/network/IdaTracingPluginServer.cc
        src/network/IdaTracingPluginServer.h
        src/emulator/instruction/data/math/MulInstructionEmulator.cc
        src/emulator/instruction/data/math/MulInstructionEmulator.h
        src/emulator/syscall/memory/MprotectSyscallEmulator.cc
        src/emulator/syscall/memory/MprotectSyscallEmulator.h
        src/emulator/instruction/data/CdqeInstructionEmulator.cc
        src/emulator/instruction/data/CdqeInstructionEmulator.h
        src/emulator/instruction/data/conditional/SetnzInstructionEmulator.cc
        src/emulator/instruction/data/conditional/SetnzInstructionEmulator.h
)

target_include_directories(cbt PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${asio_SOURCE_DIR}/asio/include"
)
target_link_libraries(cbt PRIVATE ${Protobuf_LIBRARIES} spdlog Zydis zasm)

target_compile_options(cbt PRIVATE -fPIE)
target_link_options(cbt PRIVATE
        -static-libstdc++
        -static-libgcc
        -pie
)
