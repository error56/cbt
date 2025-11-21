#ifndef VFSACCESSINTERCEPTION_H
#define VFSACCESSINTERCEPTION_H

#include <functional>
#include <memory>
#include <system_error>

#include "../execution/FileDescriptorTable.h"
#include "../util/Result.h"

class VfsEntry;

// Generic wrapper for virtual file system interceptor functions. Every
// interceptor returns true/false or error code, translated by the CBT to the
// Linux ABI error code.
template <typename... Args>
using VfsInterceptor = std::function<Result<bool, std::error_code>(Args...)>;

// Interceptor for every open() syscall on virtual file system. Arguments here
// are:
// - reference to the vfs entry being accessed,
// - flags specified to the open() syscall
using VfsOpenInterceptor =
    VfsInterceptor<const std::shared_ptr<VfsEntry>&, const uint64_t>;

// Interceptor for every read() access to thr virtual file system. Arguments
// here are:
// - file descriptor (and it's permissions),
// - reference to the vfs entry being read,
// - offset,
// - read length
using VfsReadInterceptor =
    VfsInterceptor<const FileDescriptor, const std::shared_ptr<VfsEntry>&,
                   const uint64_t, const uint64_t>;

// Interceptor for every write() access to the virtual file system. Arguments
// here are:
// - file descriptor (and it's permissions),
// - reference to the vfs entry being written to,
// - offset,
// - data to write
using VfsWriteInterceptor =
    VfsInterceptor<const FileDescriptor, const std::shared_ptr<VfsEntry>&,
                   const uint64_t, const std::vector<uint8_t>&>;

// Interceptor for delete access to the virtual file system. Arguments here are:
// - file descriptor (and it's permissions),
// - reference to the vfs entry being deleted,
using VfsDeleteInterceptor =
    VfsInterceptor<const FileDescriptor, const std::shared_ptr<VfsEntry>&>;

class VfsAccessInterceptor {
  // Interceptor for every open() access to the virtual file system.
  VfsOpenInterceptor open_;

  // Interceptor for every read() access to the virtual file system.
  VfsReadInterceptor read_;

  // Interceptor for every write() access to the virtual file system.
  VfsWriteInterceptor write_;

  // Interceptor for every delete operation on the virtual file system.
  VfsDeleteInterceptor delete_;

  // Flag indicating if interceptor is present.
  bool open_present_{false};

  // Flag indicating if interceptor is present.
  bool read_present_{false};

  // Flag indicating if interceptor is present.
  bool write_present_{false};

  // Flag indicating if interceptor is present.
  bool delete_present_{false};

 public:
  VfsAccessInterceptor() = default;
  ~VfsAccessInterceptor() = default;
  VfsAccessInterceptor(const VfsAccessInterceptor&) = delete;
  VfsAccessInterceptor& operator=(const VfsAccessInterceptor&) = delete;

  // Registers open interceptor. Returns false when interceptor already exists,
  // true otherwise.
  [[nodiscard]] bool RegisterOpenInterceptor(
      const VfsOpenInterceptor& func) noexcept;

  // Registers read interceptor. Returns false when interceptor already exists,
  // true otherwise.
  [[nodiscard]] bool RegisterReadInterceptor(
      const VfsReadInterceptor& func) noexcept;

  // Registers write interceptor. Returns false when interceptor already exists,
  // true otherwise.
  [[nodiscard]] bool RegisterWriteInterceptor(
      const VfsWriteInterceptor& func) noexcept;

  // Registers delete interceptor. Returns false when interceptor already
  // exists, true otherwise.
  [[nodiscard]] bool RegisterDeleteInterceptor(
      const VfsDeleteInterceptor& func) noexcept;

  // Allows to check if open interceptor is registered. Used mainly by the CBT
  // internals, but can also be called by the user to check if the interceptor
  // is already registered.
  [[nodiscard]] bool IsOpenInterceptorRegistered() const noexcept;

  // Allows to check if read interceptor is registered. Used mainly by the CBT
  // internals, but can also be called by the user to check if the interceptor
  // is already registered.
  [[nodiscard]] bool IsReadInterceptorRegistered() const noexcept;

  // Allows to check if write interceptor is registered. Used mainly by the CBT
  // internals, but can also be called by the user to check if the interceptor
  // is already registered.
  [[nodiscard]] bool IsWriteInterceptorRegistered() const noexcept;

  // Allows to check if delete interceptor is registered. Used mainly by the CBT
  // internals, but can also be called by the user to check if the interceptor
  // is already registered.
  [[nodiscard]] bool IsDeleteInterceptorRegistered() const noexcept;

  // Returns open interceptor.
  [[nodiscard]] VfsOpenInterceptor GetOpenInterceptor() const noexcept;

  // Returns read interceptor.
  [[nodiscard]] VfsReadInterceptor GetReadInterceptor() const noexcept;

  // Returns write interceptor.
  [[nodiscard]] VfsWriteInterceptor GetWriteInterceptor() const noexcept;

  // Returns delete interceptor.
  [[nodiscard]] VfsDeleteInterceptor GetDeleteInterceptor() const noexcept;
};

#endif  // VFSACCESSINTERCEPTION_H
