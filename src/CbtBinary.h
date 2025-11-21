#ifndef CBTBINARY_H
#define CBTBINARY_H

#include <memory>
#include <string>
#include <utility>

#include "execution/BasicBlockAllocator.h"
#include "execution/FileDescriptorTable.h"
#include "interception/SyscallId.h"
#include "interception/SyscallInterceptor.h"
#include "interception/VfsAccessInterceptor.h"
#include "virtualization/fs/VfsRoot.h"

struct TranslationCacheEntry;

// Size of memory pool dedicated for translated basic blocks
constexpr uint64_t kBasicBlockAllocatorSize = 4 * 1024 * 1024;

// Size of guest heap
constexpr uint64_t kGuestBrkMemorySize = 16 * 1024 * 1024;

class CbtBinary {
  // Path to the binary we're running
  std::string& path_;

  // User-registered syscall interceptors
  std::shared_ptr<SyscallInterceptor> syscall_interceptor_;

  // User-registered virtual file system access interceptors
  std::shared_ptr<VfsAccessInterceptor> vfs_access_interceptor_;

  // Fd table containing all file descriptors returned by the emulator to the
  // guest
  std::shared_ptr<FileDescriptorTable> file_descriptor_table_;

  // Root entry of virtual file system
  std::shared_ptr<VfsRoot> vfs_root_;

  // Maps start of the basic block to the translation cache entry, allowing us
  // to omit costly translation phase and just execute previously translated
  // basic block.
  std::shared_ptr<std::map<uint64_t, std::shared_ptr<TranslationCacheEntry>>>
      translation_cache_;

  // Allocates memory for translated basic blocks
  std::shared_ptr<BasicBlockAllocator> basic_block_allocator_;

  // Current working directory, relative to the root of VFS
  const std::string cwd_;

  // Program arguments.
  const std::vector<std::string> args_;

  // Environment variables.
  const std::vector<std::string> env_;

  // Start of the memory region allocated for the guest heap memory.
  uint64_t brk_start_;

  // End of the memory region allocated for the guest heap memory.
  uint64_t brk_end_;

 public:
  explicit CbtBinary(std::string& path,
                     const std::shared_ptr<VfsRoot>& vfs_root, std::string cwd,
                     std::vector<std::string> args,
                     std::vector<std::string> env)
      : path_(path),
        vfs_root_(vfs_root),
        cwd_(std::move(cwd)),
        args_(std::move(args)),
        env_(std::move(env)) {
    syscall_interceptor_ = std::make_shared<SyscallInterceptor>();
    file_descriptor_table_ = std::make_shared<FileDescriptorTable>();
    translation_cache_ = std::make_shared<
        std::map<uint64_t, std::shared_ptr<TranslationCacheEntry>>>();
    basic_block_allocator_ =
        std::make_shared<BasicBlockAllocator>(kBasicBlockAllocatorSize);
    vfs_access_interceptor_ = std::make_shared<VfsAccessInterceptor>();
  };

  ~CbtBinary() = default;
  CbtBinary(const CbtBinary&) = delete;
  CbtBinary& operator=(const CbtBinary&) = delete;

  // Registers interceptor for specified syscall. The lambda will be called by
  // the emulator every time guest program will invoke it. It can access guest
  // registers and modify them. Return value can instruct the emulator to pass
  // the syscall to the operating system, return to the guest or be emulated by
  // the CBT stack.
  [[nodiscard]] bool InterceptSyscall(
      UnixSyscall syscall,
      const SyscallInterceptorFunction& function) const noexcept;

  // Registers interceptor for every open() syscall.
  [[nodiscard]] bool InterceptOpenVfsAccess(
      const VfsOpenInterceptor& function) const noexcept;

  // Registers interceptor for every read() syscall.
  [[nodiscard]] bool InterceptReadVfsAccess(
      const VfsReadInterceptor& function) const noexcept;

  // Registers interceptor for every write() syscall.
  [[nodiscard]] bool InterceptWriteVfsAccess(
      const VfsWriteInterceptor& function) const noexcept;

  // Registers interceptor for every entry removal from VFS.
  [[nodiscard]] bool InterceptDeleteVfsAccess(
      const VfsDeleteInterceptor& function) const noexcept;

  // Runs the emulator. This function returns when the guest program performs
  // exit() or exit_group() syscall.
  void Run() noexcept;
};

#endif  // CBTBINARY_H
