#ifndef VFSROOT_H
#define VFSROOT_H
#include "VfsDirectory.h"

class VfsRoot {
  std::shared_ptr<VfsDirectory> root_;

 public:
  VfsRoot() : root_(std::make_shared<VfsDirectory>("root")) {}

  [[nodiscard]] std::shared_ptr<VfsEntry> Find(
      const std::string& path, const std::string& cwd) const noexcept;
  void Add(const std::shared_ptr<VfsEntry>& entry) const noexcept;
  [[nodiscard]] std::vector<std::shared_ptr<VfsEntry>> List() const noexcept;
};

#endif  // VFSROOT_H
