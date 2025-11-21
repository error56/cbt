#ifndef VFSDIRECTORY_H
#define VFSDIRECTORY_H

#include <memory>

#include "VfsEntry.h"

class VfsDirectory final : public VfsEntry {
  // Directories and files that are placed in this directory
  std::vector<std::shared_ptr<VfsEntry>> children_;

 public:
  explicit VfsDirectory(const std::string& name) : VfsEntry(name) {}

  // Adds entry to the directory
  void AddChild(const std::shared_ptr<VfsEntry>& child);

  // We can't write to the directory, so it's stubbed out
  void Write(uint64_t position, const std::vector<uint8_t>& data) override;

  // Returns list of files and directories contained in this dir
  [[nodiscard]] std::vector<std::shared_ptr<VfsEntry>> List() const override;

  // Stubbed out read function
  [[nodiscard]] std::vector<uint8_t> Read(uint64_t offset,
                                          uint64_t size) const override;
};

#endif  // VFSDIRECTORY_H
