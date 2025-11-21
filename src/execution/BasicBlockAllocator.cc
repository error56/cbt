#include "BasicBlockAllocator.h"

#include <cassert>

bool BasicBlockAllocator::ChangeMemoryPoolPermissions(
    const bool executable) const noexcept {
  const auto protection =
      executable ? PROT_READ | PROT_EXEC : PROT_READ | PROT_WRITE;

  return mprotect(reinterpret_cast<void *>(base_address_), allocation_size_,
                  protection) == 0;
}

void* BasicBlockAllocator::AllocateMemoryForTranslatedBasicBlock(
    const uint64_t size) noexcept {
  assert(current_offset_ + size < allocation_size_);

  auto *const ptr = reinterpret_cast<void *>(base_address_ + current_offset_);

  current_offset_ += size + 32;

  return ptr;
}

