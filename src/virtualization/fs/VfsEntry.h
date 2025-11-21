#ifndef VFSENTRY_H
#define VFSENTRY_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class VfsEntry {
 protected:
  // Name of the VFS entry
  std::string name;

 public:
  explicit VfsEntry(std::string name) : name(std::move(name)) {}
  virtual ~VfsEntry() = default;

  // Returns name of the entry
  [[nodiscard]] std::string GetName() const { return name; }

  // Returns list of children entries (valid only for directories)
  [[nodiscard]] virtual std::vector<std::shared_ptr<VfsEntry>> List() const = 0;

  // Writes data to the entry (valid only for files)
  virtual void Write(uint64_t position, const std::vector<uint8_t>& data) = 0;

  // Reads data from entry (valid only for files)
  [[nodiscard]] virtual std::vector<uint8_t> Read(uint64_t offset,
                                                  uint64_t size) const = 0;
};

#endif  // VFSENTRY_H
