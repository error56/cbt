#include "ReadSyscallEmulator.h"

#include "../../../execution/ExecutionThread.h"
#include "../../../virtualization/fs/VfsFile.h"

InterceptionResult ReadSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  const auto file_descriptor = GetSyscallArgument(registers, 1);
  const auto buffer = GetSyscallArgument(registers, 2);
  const auto size = GetSyscallArgument(registers, 3);

  // @TODO: Permission check

  // Get file entry from fd table
  const auto file_entry =
      emulation_context->file_descriptor_table
          ->GetDescriptorValue<FileDescriptorFileEntry>(file_descriptor);

  // Get VfsFile from fd table entry
  const auto file = std::reinterpret_pointer_cast<VfsFile>(file_entry->entry);

  // Determine read length
  const auto read_len = file->GetLength() > (size + file_entry->position)
                              ? size
                              : file->GetLength() - file_entry->position;

  // Get data from the file
  const auto read_data = file->Read(file_entry->position, read_len);

  // Make sure that we didn't read more than we should
  assert(read_data.size() <= size);

  // Copy data to the user buffer
  memcpy(reinterpret_cast<void*>(buffer), read_data.data(), read_len);

  // Adjust position and return value
  registers.rax = read_len;
  file_entry->position += read_len;

  return kReturnToTheGuest;
}