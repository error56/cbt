#ifndef LINUXOPENEDFILE_H
#define LINUXOPENEDFILE_H

#include <string>
#include <utility>

#include "../util/Option.h"

class LinuxOpenedFile {
  void *base_ptr_{nullptr};
  const std::string path_;

public:
  explicit LinuxOpenedFile(std::string path) : path_(std::move(path)) {}
  ~LinuxOpenedFile();
  LinuxOpenedFile(const LinuxOpenedFile &) = delete;
  LinuxOpenedFile &operator=(const LinuxOpenedFile &) = delete;

  [[nodiscard]] Option<void *> MapFileToMemory() noexcept;
};

#endif //LINUXOPENEDFILE_H
