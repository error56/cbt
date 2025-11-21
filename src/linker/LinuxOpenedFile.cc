#include "LinuxOpenedFile.h"

#include <fcntl.h>
#include <sys/stat.h>

#include "spdlog/spdlog.h"

Option<void *> LinuxOpenedFile::MapFileToMemory() noexcept {
  const int file_descriptor = open(path_.c_str(), O_RDONLY | O_CLOEXEC);
  if (file_descriptor == -1) {
    spdlog::debug("open failed (path: {})", path_.c_str());

    return Option<void *>::None();
  }

  struct stat stat{};
  if (fstat(file_descriptor, &stat) == -1 || stat.st_size == 0) {
    spdlog::debug("fstat failed or file is empty (path: {})", path_.c_str());

    close(file_descriptor);
    return Option<void *>::None();
  }

  const auto file_size = static_cast<size_t>(stat.st_size);
  auto *const buffer = new (std::nothrow) char[file_size];
  if (buffer == nullptr) {
    spdlog::debug("Memory allocation failed");

    close(file_descriptor);
    return Option<void *>::None();
  }

  if (const ssize_t bytes_read = read(file_descriptor, buffer, file_size);
      bytes_read != static_cast<ssize_t>(file_size)) {
    spdlog::debug("read failed or incomplete read (path: {})", path_.c_str());

    delete[] buffer;
    close(file_descriptor);
    return Option<void *>::None();
  }

  base_ptr_ = buffer;

  close(file_descriptor);
  return Option<void *>::Some(buffer);
}

LinuxOpenedFile::~LinuxOpenedFile() {
  if (base_ptr_ != nullptr) {
    delete[] static_cast<char *>(base_ptr_);
    base_ptr_ = nullptr;
  }
}
