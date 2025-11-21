#include "OpenSyscallEmulator.h"

#include "../../../execution/ExecutionThread.h"
#include "../../../virtualization/fs/VfsFile.h"

std::string FilenameFromPath(const std::string& path) {
  const size_t pos = path.find_last_of("/\\");
  if (pos == std::string::npos) {
    return path;
  }

  return path.substr(pos + 1);
}

InterceptionResult OpenSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  const auto* path_ptr =
      reinterpret_cast<const char*>(GetSyscallArgument(registers, 1));
  const auto path = std::string(path_ptr);
  const auto flags = static_cast<int32_t>(GetSyscallArgument(registers, 2));

  auto file = emulation_context->vfs_root->Find(path, emulation_context->cwd);

  if (file == nullptr) {
    // If we didn't find the file in VFS, check if we need to create it
    if ((flags & O_CREAT) == 0) {
      // If we can't create it, return an error.
      registers.rax = -ENOENT;

      return kReturnToTheGuest;
    }

    const auto file_name = FilenameFromPath(path);
    const auto entry =
        emulation_context->vfs_root->Find(path, emulation_context->cwd);

    if (entry == nullptr) {
      // No parent dir
      registers.rax = -ENOENT;

      return kReturnToTheGuest;
    }

    // Get directory from entry, make new file and add it to the directory
    const auto dir = std::dynamic_pointer_cast<VfsDirectory>(entry);
    const auto new_file = std::make_shared<VfsFile>(file_name);
    dir->AddChild(new_file);

    file = new_file;
  }

  // Check if there's any open interceptor registered
  if (emulation_context->vfs_access_interceptor
          ->IsOpenInterceptorRegistered()) {
    const auto interceptor =
        emulation_context->vfs_access_interceptor->GetOpenInterceptor();
    const auto result = interceptor(file, flags);

    if (result.IsErr()) {
      const auto error = result.GetError();
      assert(error.category() == std::system_category());

      registers.rax = -error.value();

      return kReturnToTheGuest;
    }
  }

  // Make file descriptor entry connected to found file
  const auto fd_entry =
      std::make_shared<FileDescriptorFileEntry>(FileDescriptorFileEntry{
          .entry = file,
          .position = 0,
      });

  const auto fd = emulation_context->file_descriptor_table
                ->GetNextMonotonicallyIncreasingDescriptorId();
  emulation_context->file_descriptor_table->AddDescriptor(fd, fd_entry);

  registers.rax = fd;

  return kReturnToTheGuest;
}
