#ifndef HANDLETABLE_H
#define HANDLETABLE_H

#include <cstdint>
#include <map>
#include <memory>

// Starting number of file descriptor allocated by the emulator.
constexpr uint64_t kEmulatorSpaceStartingIndex = 0x64000;

// Make a type alias for uint64_t allowing for easier understanding of code
// mentioning file descriptors.
using FileDescriptor = uint64_t;

// Known file descriptors for accessing special fds like standard input or
// standard out.
enum KnownFileDescriptors : uint8_t {
  kStdin = 0,
  kStdout = 1,
  kStderr = 2,
};

class FileDescriptorTable {
  // Map of FD ID to object.
  std::map<uint64_t, std::shared_ptr<void>> file_descriptor_table_;

  // Allocation offset.
  uint64_t current_fd_index_{3};  // omit stdin, stdout and stderr

 public:
  // Adds file descriptor to the table.
  template <typename T>
  bool AddDescriptor(FileDescriptor handle, std::shared_ptr<T> value) {
    auto [it, inserted] = file_descriptor_table_.emplace(
        handle, std::static_pointer_cast<void>(value));
    return inserted;
  }

  // Removes file descriptor from the table.
  bool RemoveDescriptor(FileDescriptor handle) noexcept;

  // Returns next available file descriptor.
  [[nodiscard]] FileDescriptor
  GetNextMonotonicallyIncreasingDescriptorId() noexcept;

  // Gets value from file descriptor table.
  template <typename T>
  [[nodiscard]] std::shared_ptr<T> GetDescriptorValue(
      const FileDescriptor handle) const noexcept {
    if (file_descriptor_table_.contains(handle)) {
      return std::static_pointer_cast<T>(file_descriptor_table_.at(handle));
    }

    return nullptr;
  }
};

#endif  // HANDLETABLE_H
