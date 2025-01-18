cmake_minimum_required(VERSION 3.28)

project(cbt)

include(FetchContent)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
    pe-parse
    GIT_REPOSITORY https://github.com/trailofbits/pe-parse
    GIT_TAG        7888f1f8de2f6bc302c291a5b4519fad926c0133 # 2.1.1
)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG        8e5613379f5140fefb0b60412fbf1f5406e7c7f8 # 1.15.0
)
FetchContent_Declare(
    zydis
    GIT_REPOSITORY https://github.com/zyantific/zydis
    GIT_TAG        569320ad3c4856da13b9dbf1f0d9e20bda63870e # 4.1.0
)

# Everything
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# pe-parse
set(BUILD_COMMAND_LINE_TOOLS OFF CACHE BOOL "" FORCE)

# Zydis
set(ZYDIS_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(pe-parse spdlog zydis)

add_executable(cbt src/main.cpp)
target_include_directories(cbt PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(cbt PRIVATE spdlog pe-parse Zydis)
