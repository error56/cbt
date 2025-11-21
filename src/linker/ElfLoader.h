#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <elf.h>

#include <cstdint>
#include <memory>
#include <string>

#include "LinuxOpenedFile.h"
#include "spdlog/spdlog.h"

struct ElfSymbol {
  std::string name;
  uint64_t address;
  uint64_t relocated_address;
  uint64_t size;
  uint8_t type;
  uint8_t binding;
  uint16_t section_index;
};

struct ElfExport {
  std::string name;
  uint64_t address;
};

struct ElfImport {
  std::string name;
  uint64_t relocation_offset;
  uint64_t relocation_type;
  uint64_t plt_address;
  uint64_t got_address;
};

class ElfLoader {
  // Address of memory-mapped file
  void* mapped_file_{nullptr};
  // Base address of relocated binary in lower 4GiB of memory
  void* relocated_binary_{nullptr};
  // Mapped file base
  uint64_t base_virtual_addr_{0};
  // Vector of all exports in main executable and loaded libraries
  std::vector<ElfExport>& exports_;
  // Vector of all loaded binaries needed to properly link the executable
  std::vector<std::shared_ptr<ElfLoader>>& binaries_;
  // Memory mapped file instance
  LinuxOpenedFile file_;
  // Symbol table data
  const Elf64_Sym* symbol_table_ = nullptr;
  // Symbol count
  size_t symbol_count_ = 0;
  // String table pointer
  const char* string_table_ = nullptr;
  // String table size
  size_t string_table_size_ = 0;

  // Initializes ELF loader.
  // Returns true on success, false on error.
  bool Initialize();

 public:
  std::string path_;
  size_t allocation_size{0};

  explicit ElfLoader(const std::string& path, std::vector<ElfExport>& exports,
                     std::vector<std::shared_ptr<ElfLoader>>& binaries)
      : exports_(exports), binaries_(binaries), file_(path), path_(path) {
    this->Initialize();
  };

  // Returns next available offset to load new binary to.
  [[nodiscard]] static uint64_t GetNextOffset() noexcept;

  // Loads the binary into the memory, takes care of imports and exports,
  // loads necessary dynamic-linked libraries and does all the stuff such as
  // resolving references or relocations. Basically loads and links binary.
  //
  // Returns true on success, false on error.
  [[nodiscard]] bool Load(std::shared_ptr<ElfLoader> loader) noexcept;

  // Returns relative address of entry point from the image base
  [[nodiscard]] uint64_t GetEntryPoint() const noexcept;

  // Returns base of relocated binary in memory
  [[nodiscard]] uint64_t GetBinaryBase() const noexcept;

  [[nodiscard]] uint64_t GetMappedFileBase() const noexcept;

  [[nodiscard]] std::vector<ElfSymbol> GetSymbols(
      const Elf64_Shdr* dynsym, const Elf64_Shdr* dynstr) const noexcept;

  void ApplyRelocation(Elf64_Addr offset, int type, int sym_index,
                       Elf64_Sxword addend);
  void ProcessRelSection(int section_index, const Elf64_Shdr* section_header,
                         const Elf64_Ehdr* elf_header);

  void ProcessRelaSection(int section_index, const Elf64_Shdr* section_header,
                          const Elf64_Ehdr* elf_header);

  void ProcessRelocations(const Elf64_Shdr* rela_section,
                          const Elf64_Sym* dynsym_table,
                          const char* dynstr_table,
                          std::vector<ElfImport>& imports,
                          bool is_plt) const noexcept;

  uint64_t ResolveSymbol(int sym_index);

  uint64_t ResolveExternalSymbol(const char* symbol_name) const;

  bool InitializeSymbolTable();
  bool LoadSegments();

  [[nodiscard]] std::vector<uintptr_t> GetConstructors() noexcept;

  void ExtractConstructorsFromSection(const Elf64_Shdr& section,
                                      std::vector<uintptr_t>& constructors,
                                      const char* section_type);
};
#endif  // ELFLOADER_H
