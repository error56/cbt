#ifndef SYSCALLINTERCEPTOR_H
#define SYSCALLINTERCEPTOR_H

#include <functional>
#include <map>

#include "SyscallId.h"

class ExecutionThread;
struct ProcessorRegisters;

// Context structure passed to syscall interceptors allowing them to modify
// registers and get syscall parameters by the argument number
struct SyscallContext {
  // Mutable reference to guest registers
  ProcessorRegisters &regs;

  // Returns `n`-th argument value
  [[nodiscard]] uint64_t GetArgument(uint64_t argument_number) const noexcept;
};

// Enum returned by the syscall interceptors, telling the CBT how to process the
// syscall
enum InterceptionResult : uint8_t {
  // Execute syscall (load guest registers and execute real `syscall`
  // instruction)
  kExecuteSyscall = 0,

  // The user interceptor can't handle the request, so the CBT-registered
  // default interceptor should be invoked
  kPassDownTheStack = 1,

  // User interceptor handled the syscall and CBT should return to executing
  // guest code
  kReturnToTheGuest = 2,

  // Internal error happened and CBT should break the emulation to not corrupt
  // the state
  kPanic = 3,
};

// Interceptor returns InterceptorResult to indicate how the CBT should handle
// the syscall.
using SyscallInterceptorFunction = std::function<InterceptionResult(
    ProcessorRegisters &, SyscallContext &, ExecutionThread &)>;

class SyscallInterceptor {
  // Map available syscalls to their interceptors
  std::map<UnixSyscall, SyscallInterceptorFunction> functions_;

 public:
  explicit SyscallInterceptor() = default;

  // Registers interceptor for given syscall
  [[nodiscard]] bool RegisterInterceptor(
      UnixSyscall syscall, const SyscallInterceptorFunction &function) noexcept;

  // Called by the CBT internals when the syscall is handled. The function
  // returns true, if interceptor for the syscall is found and returned
  // kReturnToTheGuest, false otherwise.
  [[nodiscard]] bool Intercept(ExecutionThread *thread, UnixSyscall syscall,
                               ProcessorRegisters &registers) const noexcept;
};

#endif  // SYSCALLINTERCEPTOR_H
