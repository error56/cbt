#include "WritevInstructionEmulator.h"

#include "../../../execution/ExecutionThread.h"
#include "../../../virtualization/fs/VfsFile.h"

InterceptionResult WritevSyscallEmulator::EmulateSyscall(
    ExecutionThread* thread) const noexcept {
  auto& registers = thread->GetGuestRegisters();

  const auto file_descriptor =
      static_cast<uint32_t>(GetSyscallArgument(registers, 1));
  const auto* const iov =
      reinterpret_cast<const struct iovec*>(GetSyscallArgument(registers, 2));
  const auto iov_cnt = static_cast<uint32_t>(GetSyscallArgument(registers, 3));

  // @TODO: Reserve vector size at the beginning to avoid expensive
  // reallocations.
  //
  // Prepare data buffer
  auto data = std::vector<uint8_t>{};

  // Iterate over all entries and copy them to the buffer
  for (auto i = 0; i < iov_cnt; i++) {
    const auto [iov_base, iov_len] = iov[i];

    data.insert(data.end(), static_cast<uint8_t*>(iov_base),
                reinterpret_cast<uint8_t*>(reinterpret_cast<uint64_t>(iov_base) + iov_len));
  }

  // Find file entry using file descriptor table
  const auto entry =
      emulation_context->file_descriptor_table
          ->GetDescriptorValue<FileDescriptorFileEntry>(file_descriptor);
  const auto file = std::reinterpret_pointer_cast<VfsFile>(entry->entry);

  // Trigger write interceptor, if registered.
  if (emulation_context->vfs_access_interceptor
          ->IsWriteInterceptorRegistered()) {
    const auto interceptor =
        emulation_context->vfs_access_interceptor->GetWriteInterceptor();

    const auto result =
        interceptor(file_descriptor, file, entry->position, data);
    if (result.IsErr()) {
      const auto error = result.GetError();
      assert(error.category() == std::system_category());

      registers.rax = -error.value();

      return kReturnToTheGuest;
    }
  }

  // @TODO: Permission check
  //
  // Write the file if it's not stdin/stdout/stderr. To display application
  // output, the developer need to register write interceptor and print data to
  // the console by his own.
  if (file_descriptor != kStdin && file_descriptor != kStdout &&
      file_descriptor != kStderr) {
    file->Write(entry->position, data);
    entry->position += data.size();
  }

  registers.rax = data.size();

  return kReturnToTheGuest;
}
