#include "Getdents64SyscallEmulator.h"

#include <dirent.h>

#include "../../../execution/ExecutionThread.h"

InterceptionResult Getdents64SyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  const auto file_descriptor = GetSyscallArgument(registers, 1);
  const auto data_pointer = GetSyscallArgument(registers, 2);
  const auto entries_count = GetSyscallArgument(registers, 3);

  const auto dir =
      emulation_context->file_descriptor_table
          ->GetDescriptorValue<FileDescriptorFileEntry>(file_descriptor);
  const auto entries = dir->entry->List();

  if (dir->position >= entries.size()) {
    // End of directory
    registers.rax = 0;

    return kReturnToTheGuest;
  }

  size_t bytes_written = 0;
  size_t entries_written = 0;
  size_t current_position = dir->position;

  while (current_position < entries.size()) {
    const auto& entry = entries[current_position];
    const auto name_len = entry->GetName().size();

    // Calculate record size
    const size_t base_size = offsetof(linux_dirent64, d_name) + name_len + 1;
    // Pad to 8 bytes
    const size_t record_len = (base_size + 7) & ~7;

    // Check if there's enough space in the buffer
    if (bytes_written + record_len > entries_count) {
      break;
    }

    auto* dirent =
        reinterpret_cast<linux_dirent64*>(data_pointer + bytes_written);

    // Fill the entry
    //
    // Set inode
    dirent->d_ino = 123;  // @TODO: Try to implement valid inode here.

    // Offset to next entry
    dirent->d_off = current_position + 1;

    // Record length
    dirent->d_reclen = static_cast<uint16_t>(record_len);

    // Set file type
    dirent->d_type =
        DT_REG;  // @TODO: Check if entry is directory and change the type here

    // Copy file name
    std::memcpy(dirent->d_name, entry->GetName().c_str(), name_len + 1);

    // Zero padding
    if (record_len > base_size) {
      std::memset(
          reinterpret_cast<void*>(data_pointer + bytes_written + base_size), 0,
          record_len - base_size);
    }

    // Adjust counters
    bytes_written += record_len;
    entries_written++;
    current_position++;
  }

  // Update position in directory and return
  dir->position = current_position;
  registers.rax = bytes_written;

  return kReturnToTheGuest;
}
