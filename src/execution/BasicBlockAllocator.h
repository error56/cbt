#ifndef BASICBLOCKALLOCATOR_H
#define BASICBLOCKALLOCATOR_H

#include <sys/mman.h>

#include <cstdint>

// Start of memory region allocated for translated basic blocks.
constexpr auto kTranslatedBasicBlockMemoryPoolStart = 0x11340000;

class BasicBlockAllocator {
  // Base address of memory region.
  uint64_t base_address_;

  // Size of memory allocated for translated basic blocks.
  uint64_t allocation_size_;

  // Current offset at which new blocks will be placed.
  uint64_t current_offset_{0};

  // Changes memory pool protections to R-X or RW-.
  [[nodiscard]] bool ChangeMemoryPoolPermissions(
      bool executable) const noexcept;

 public:
  explicit BasicBlockAllocator(const uint64_t allocation_size) noexcept
      : allocation_size_(allocation_size) {
    base_address_ = reinterpret_cast<uint64_t>(
        mmap(reinterpret_cast<void *>(kTranslatedBasicBlockMemoryPoolStart),
             allocation_size, PROT_READ | PROT_WRITE | PROT_EXEC,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  };

  // Returns pointer to the allocated region.
  [[nodiscard]] void *AllocateMemoryForTranslatedBasicBlock(
      uint64_t size) noexcept;
};

#endif  // BASICBLOCKALLOCATOR_H
