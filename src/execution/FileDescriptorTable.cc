#include "FileDescriptorTable.h"

FileDescriptor
FileDescriptorTable::GetNextMonotonicallyIncreasingDescriptorId() noexcept {
  current_fd_index_++;

  return current_fd_index_;
}

bool FileDescriptorTable::RemoveDescriptor(
    const FileDescriptor handle) noexcept {
  if (file_descriptor_table_.contains(handle)) {
    file_descriptor_table_.erase(handle);

    return true;
  }

  return false;
}