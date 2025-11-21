#include "VfsDirectory.h"

std::vector<std::shared_ptr<VfsEntry>> VfsDirectory::List() const {
  return children_;
}

void VfsDirectory::AddChild(const std::shared_ptr<VfsEntry>& child) {
  children_.push_back(child);
}

void VfsDirectory::Write(const uint64_t position,
                         const std::vector<uint8_t>& data) {}

std::vector<uint8_t> VfsDirectory::Read(uint64_t offset, uint64_t size) const {
  return {};
}
