#include "VfsFile.h"

void VfsFile::Write(const uint64_t position, const std::vector<uint8_t>& data) {
  if (position + data.size() > content_.size()) {
    content_.resize(position + data.size());
  }

  std::ranges::copy(data, content_.begin() + position);
}

uint64_t VfsFile::GetLength() const noexcept { return content_.size(); }

std::vector<std::shared_ptr<VfsEntry>> VfsFile::List() const { return {}; }

std::vector<uint8_t> VfsFile::Read(const uint64_t offset,
                                   const uint64_t size) const {
  if (offset >= content_.size()) {
    return {};
  }

  const auto available = content_.size() - offset;
  const auto to_read = std::min(size, available);

  return {
      content_.begin() + offset,
      content_.begin() + offset + to_read
  };
}
