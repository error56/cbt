#ifndef VFSFILE_H
#define VFSFILE_H
#include "VfsEntry.h"

class VfsFile final : public VfsEntry {
  std::vector<uint8_t> content_;

 public:
  explicit VfsFile(const std::string& name) : VfsEntry(name) {}
  VfsFile(const std::string& name, const std::vector<uint8_t>& content)
      : VfsEntry(name), content_(content) {}

  void Write(uint64_t position, const std::vector<uint8_t>& data) override;
  [[nodiscard]] std::vector<std::shared_ptr<VfsEntry>> List() const override;
  [[nodiscard]] std::vector<uint8_t> Read(uint64_t offset,
                                          uint64_t size) const override;

  [[nodiscard]] uint64_t GetLength() const noexcept;
};

#endif  // VFSFILE_H
