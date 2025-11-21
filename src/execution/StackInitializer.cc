#include "StackInitializer.h"

#include <random>

void StackInitializer::AlignTo8Bytes(
    ProcessorRegisters& registers) noexcept {
  while (registers.rsp % 8 != 0) {
    PushByte(0, registers);
  }
}

void StackInitializer::PushByte(const uint8_t byte,
                                ProcessorRegisters& registers) noexcept {
  registers.rsp -= 1;

  // Write the value onto the stack
  *reinterpret_cast<uint8_t*>(registers.rsp) = byte;
}

void StackInitializer::PushQword(const uint64_t qword,
                                ProcessorRegisters& registers) noexcept {
  registers.rsp -= 8;

  // Write the value onto the stack
  std::memcpy(reinterpret_cast<void*>(registers.rsp), &qword, sizeof(qword));
  //*reinterpret_cast<uint64_t*>(registers.rsp) = qword;
}

void StackInitializer::PushString(
    const char* str, ProcessorRegisters& registers) noexcept {
  // reverse order

  // Null terminator
  PushByte('\0', registers);

  const auto len = static_cast<int>(strlen(str));
  for (int i = len - 1; i >= 0; --i) {
    PushByte(str[i], registers);
  }
}

std::vector<uint64_t> StackInitializer::PushStringArray(
    const std::vector<std::string>& strings,
    ProcessorRegisters& guest_registers) const noexcept {
  std::vector<uint64_t> string_addresses;
  string_addresses.reserve(strings.size());

  for (int i = static_cast<int>(strings.size() - 1); i >= 0; --i) {
    PushString(strings[i].c_str(), guest_registers);
    string_addresses.push_back(guest_registers.rsp);
    AlignTo8Bytes(guest_registers);
  }

  std::ranges::reverse(string_addresses);

  return string_addresses;
}

uint64_t StackInitializer::PushStringAndGetAddress(
    const char* str, ProcessorRegisters& guest_registers) noexcept {
  PushString(str, guest_registers);

  const uint64_t addr = guest_registers.rsp;

  AlignTo8Bytes(guest_registers);

  return addr;
}

void StackInitializer::Initialize(
    ProcessorRegisters& guest_registers,
    const uint64_t mapped_file_base) const noexcept {
  const auto env_strings = PushStringArray(envp_, guest_registers);
  const auto argv_strings = PushStringArray(argv_, guest_registers);

  // Push AUX vector
  std::random_device random;
  for (int i = 0; i < 16; ++i) {
    PushByte(static_cast<uint8_t>(random()), guest_registers);
  }

  const uint64_t at_random_addr = guest_registers.rsp;

  AlignTo8Bytes(guest_registers);

  const uint64_t at_platform_addr =
      PushStringAndGetAddress("x86_64", guest_registers);
  const uint64_t at_base_platform_addr =
      PushStringAndGetAddress("x86_64", guest_registers);

  const auto* const elf_header =
      reinterpret_cast<Elf64_Ehdr*>(mapped_file_base);

  auto push_aux_pair = [&](const uint64_t type, const uint64_t val) {
    PushQword(val, guest_registers);
    PushQword(type, guest_registers);
  };

  // Ensure 8-byte alignment before starting AUX vector (pairs are 8-byte
  // aligned)
  AlignTo8Bytes(guest_registers);

  // Push entries in REVERSE of final memory order (so that auxv[0] is the last
  // pushed pair)
  push_aux_pair(AT_NULL, 0);

  // Platform + random + secure
  push_aux_pair(AT_SECURE, 0);
  push_aux_pair(AT_RANDOM, at_random_addr);
  push_aux_pair(AT_PLATFORM, at_platform_addr);
  push_aux_pair(AT_BASE_PLATFORM, at_base_platform_addr);

  push_aux_pair(AT_HWCAP2, 0);
  push_aux_pair(AT_HWCAP, 1ULL | (1ULL << 1) | (1ULL << 2) | (1ULL << 3) | (1ULL << 4) |
    (1ULL << 5) | (1ULL << 6) | (1ULL << 8) | (1ULL << 9) | (1ULL << 11) |
    (1ULL << 13) | (1ULL << 15) | (1ULL << 23) | (1ULL << 24) | (1ULL << 25) |
    (1ULL << 26));

  push_aux_pair(AT_PAGESZ, 4096);
  push_aux_pair(AT_CLKTCK, 100);
  push_aux_pair(AT_UID, 1000);
  push_aux_pair(AT_EUID, 1000);
  push_aux_pair(AT_GID, 1000);
  push_aux_pair(AT_EGID, 1000);

  // ELF related entries
  push_aux_pair(AT_PHDR, mapped_file_base + elf_header->e_phoff);
  push_aux_pair(AT_PHENT, sizeof(Elf64_Phdr));
  push_aux_pair(AT_PHNUM, elf_header->e_phnum);
  push_aux_pair(AT_ENTRY, elf_header->e_entry);
  push_aux_pair(AT_FLAGS, elf_header->e_flags);

  // Push envp pointers
  for (int i = static_cast<int>(env_strings.size()) - 1; i >= 0; --i) {
    PushQword(env_strings[i], guest_registers);
  }
  // envp NULL terminator
  PushQword(0, guest_registers);

  // Push argv pointers
  for (int i = static_cast<int>(argv_strings.size()) - 1; i >= 0; --i) {
    PushQword(argv_strings[i], guest_registers);
  }

  // Push argc
  PushQword(argv_.size(), guest_registers);
}
