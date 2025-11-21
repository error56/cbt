#ifndef GETDENTS64SYSCALLEMULATOR_H
#define GETDENTS64SYSCALLEMULATOR_H

#include "../SyscallEmulator.h"

struct linux_dirent64 {
  ino64_t d_ino;
  off64_t d_off;
  unsigned short d_reclen;
  unsigned char d_type;
  char d_name[];
};

class Getdents64SyscallEmulator final : public SyscallEmulator {
  using SyscallEmulator::SyscallEmulator;

public:
  ~Getdents64SyscallEmulator() override = default;
  InterceptionResult EmulateSyscall(
      ExecutionThread* thread) const noexcept override;
};

#endif //GETDENTS64SYSCALLEMULATOR_H
