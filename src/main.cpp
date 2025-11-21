#include <sys/utsname.h>

#include <asio.hpp>
#include <filesystem>

#include "CbtBinary.h"
#include "execution/ExecutionThread.h"
#include "spdlog/spdlog.h"
#include "virtualization/fs/VfsFile.h"
#include "virtualization/fs/VfsRoot.h"

int main() {
  std::string exe_path = "";

  const auto vfs = std::make_shared<VfsRoot>();

  const auto root_file = std::make_shared<VfsFile>("root_file");
  const auto root_dir = std::make_shared<VfsDirectory>("dir1");
  const auto root_dir2 = std::make_shared<VfsDirectory>("inside");
  const auto dir2_subdir = std::make_shared<VfsDirectory>("dir2_subdir");
  const auto dir2_file = std::make_shared<VfsFile>("dir2_file");

  root_dir2->AddChild(dir2_file);
  root_dir2->AddChild(dir2_subdir);

  vfs->Add(root_file);
  vfs->Add(root_dir);
  vfs->Add(root_dir2);

  const auto* cwd = ".";
  static std::vector<std::string> args = {"./hello_world", "--flag", "value"};
  static std::vector<std::string> env = {"PATH=/usr/bin", "HOME=/home/user"};
  auto cbt_binary = CbtBinary(exe_path, vfs, cwd, args, env);

  // CBT can intercept syscalls and fake output:
  assert(cbt_binary.InterceptSyscall(
      kSysUname, [](ProcessorRegisters& registers,
                    const SyscallContext& context, ExecutionThread&) {
        utsname info{};

        strcpy(info.sysname, "ExampleOS");
        strcpy(info.nodename, "NodeName");
        strcpy(info.release, "11.09");
        strcpy(info.version, "01");
        strcpy(info.machine, "ARM");

        *reinterpret_cast<utsname*>(context.GetArgument(0)) = info;
        registers.rax = 0;

        return kReturnToTheGuest;
      }));

  // CBT can monitor virtual file system accesses:
  assert(cbt_binary.InterceptOpenVfsAccess(
      [](const std::shared_ptr<VfsEntry>& entry, const uint64_t flags) {
        spdlog::warn("Received open vfs access: \"{}\"", entry->GetName());

        return Result<bool, std::error_code>::Ok(true);
      }));

  // CBT can monitor write accesses on virtual file system:
  assert(cbt_binary.InterceptWriteVfsAccess(
      [](const FileDescriptor fd, const std::shared_ptr<VfsEntry>& entry,
         const uint64_t offset, const std::vector<uint8_t>& data) {
        if (fd == kStdout) {
          auto text = std::string(reinterpret_cast<const char*>(data.data()),
                                  data.size());

          std::erase(text, '\n');

          spdlog::info("Application stdout={}", text);

          return Result<bool, std::error_code>::Ok(true);
        }
        if (fd == kStderr) {
          const auto text = std::string_view(
              reinterpret_cast<const char*>(data.data()), data.size());

          spdlog::error("Application stderr={}", text);

          return Result<bool, std::error_code>::Ok(true);
        }

        spdlog::warn("Received write vfs access");

        return Result<bool, std::error_code>::Ok(true);
      }));

  cbt_binary.Run();

  return 0;
}
