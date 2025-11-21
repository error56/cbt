#include "CbtBinary.h"

#include <sys/mman.h>

#include <vector>

#include "execution/ExecutionThread.h"
#include "linker/ElfLoader.h"
#include "network/IdaTracingPluginServer.h"
#include "virtualization/fs/VfsFile.h"

void CbtBinary::Run() noexcept {
  const auto tracing_plugin =
      std::make_shared<IdaTracingPluginServer>("0.0.0.0", 9010);

  auto server_thread =
      std::thread([tracing_plugin]() { tracing_plugin->Start(); });
  server_thread.detach();

  std::vector<ElfExport> exports{};
  std::vector<std::shared_ptr<ElfLoader>> loaders{};

  const auto pe_loader = std::make_shared<ElfLoader>(path_, exports, loaders);
  if (!pe_loader->Load(pe_loader)) {
    spdlog::error("Failed to load PE file");
    return;
  }

  brk_start_ = reinterpret_cast<uint64_t>(
      mmap(reinterpret_cast<void *>(
               (0x400000 + pe_loader->allocation_size + 4096) % 4096),
           kGuestBrkMemorySize, PROT_WRITE | PROT_READ,
           MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));

  brk_end_ = brk_start_ + kGuestBrkMemorySize;

  const auto fd_table = std::make_shared<FileDescriptorTable>();

  const auto file = std::make_shared<VfsFile>("STD_FD");
  const auto fd_entry =
      std::make_shared<FileDescriptorFileEntry>(FileDescriptorFileEntry{
          .entry = file,
          .position = 0,
      });

  fd_table->AddDescriptor(0, fd_entry);
  fd_table->AddDescriptor(1, fd_entry);
  fd_table->AddDescriptor(2, fd_entry);

  std::shared_ptr<SyscallInterceptor> syscall_interceptors;
  std::shared_ptr<IdaTracingPluginServer> tracing_server;
  std::shared_ptr<FileDescriptorTable> file_descriptor_table;
  std::shared_ptr<VfsRoot> vfs_root;
  std::shared_ptr<VfsAccessInterceptor> vfs_access_interceptor;

  auto hlec =
      std::make_shared<HighLevelEmulationContext>(HighLevelEmulationContext{
          .syscall_interceptors = syscall_interceptor_,
          .tracing_server = tracing_plugin,
          .file_descriptor_table = fd_table,
          .vfs_root = vfs_root_,
          .vfs_access_interceptor = vfs_access_interceptor_,
          .cwd = cwd_,
          .brk_start = brk_start_,
          .brk_end = brk_end_,
          .args = args_,
          .env = env_,
      });

  const auto translation_context = std::make_shared<TranslationContext>(
      TranslationContext{.translation_cache = translation_cache_,
                         .basic_block_allocator = basic_block_allocator_});

  auto thread = ExecutionThread(pe_loader, pe_loader->GetEntryPoint(), hlec,
                                translation_context);

  thread.Run();
}

bool CbtBinary::InterceptSyscall(
    const UnixSyscall syscall,
    const SyscallInterceptorFunction &function) const noexcept {
  return syscall_interceptor_->RegisterInterceptor(syscall, function);
}

bool CbtBinary::InterceptOpenVfsAccess(
    const VfsOpenInterceptor &function) const noexcept {
  return vfs_access_interceptor_->RegisterOpenInterceptor(function);
}

bool CbtBinary::InterceptReadVfsAccess(
    const VfsReadInterceptor &function) const noexcept {
  return vfs_access_interceptor_->RegisterReadInterceptor(function);
}

bool CbtBinary::InterceptWriteVfsAccess(
    const VfsWriteInterceptor &function) const noexcept {
  return vfs_access_interceptor_->RegisterWriteInterceptor(function);
}

bool CbtBinary::InterceptDeleteVfsAccess(
    const VfsDeleteInterceptor &function) const noexcept {
  return vfs_access_interceptor_->RegisterDeleteInterceptor(function);
}
