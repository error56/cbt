#include "ElfLoader.h"

#include <elf.h>
#include <sys/mman.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>

#include "../execution/ExecutionThread.h"
#include "spdlog/spdlog.h"

// Base address of file mapping
constexpr uint64_t kFileMappingBaseAddress = 0x400000;

// @TODO: Refactor entire loader
bool ElfLoader::Initialize() {
  // Map file in the memory
  const auto mapped_file = file_.MapFileToMemory();
  if (mapped_file.IsNone()) {
    spdlog::error("Cannot map file to memory");
    return false;
  }
  mapped_file_ = mapped_file.GetValue();

  // Figure out allocation size and allocate memory for relocated binary
  const auto file_path = std::filesystem::path(path_);
  const auto file_size = std::filesystem::file_size(file_path);

  const auto elf_header = static_cast<Elf64_Ehdr *>(mapped_file_);

  // Validate ELF header
  if (memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0) {
    spdlog::error("Invalid ELF file");
    return false;
  }

  if (elf_header->e_ident[EI_CLASS] != ELFCLASS64) {
    spdlog::error("Invalid ELF class");
    return false;
  }

  // Validate program header table
  if (elf_header->e_phoff == 0 || elf_header->e_phnum == 0) {
    spdlog::error("No program header table found");
    return false;
  }

  // Check if program header entry size is valid
  if (elf_header->e_phentsize != sizeof(Elf64_Phdr)) {
    spdlog::error("Invalid program header entry size: {} (expected: {})",
                  elf_header->e_phentsize, sizeof(Elf64_Phdr));
    return false;
  }

  // Validate that program headers are within file bounds
  const uint64_t pht_end =
      elf_header->e_phoff + (elf_header->e_phnum * elf_header->e_phentsize);
  if (pht_end > file_size) {
    spdlog::error("Program header table extends beyond file size");
    return false;
  }

  const auto program_headers = reinterpret_cast<Elf64_Phdr *>(
      static_cast<uint8_t *>(mapped_file_) + elf_header->e_phoff);

  // Calculate memory layout from LOAD segments
  uint64_t min_addr = UINT64_MAX;
  uint64_t max_addr = 0;
  bool found_load_segment = false;

  for (int i = 0; i < elf_header->e_phnum; ++i) {
    const auto &phdr = program_headers[i];

    if (phdr.p_type == PT_LOAD) {
      found_load_segment = true;

      uint64_t segment_start = phdr.p_vaddr;
      uint64_t segment_end = phdr.p_vaddr + phdr.p_memsz;

      min_addr = std::min(min_addr, segment_start);
      max_addr = std::max(max_addr, segment_end);

      spdlog::debug("LOAD segment {}: 0x{:x}-0x{:x} (size: 0x{:x})", i,
                    segment_start, segment_end, phdr.p_memsz);
    }
  }

  if (!found_load_segment) {
    spdlog::error("No LOAD segments found in ELF file");
    return false;
  }

  base_virtual_addr_ = min_addr;
  allocation_size = max_addr - min_addr;

  // Ensure allocation size is reasonable
  if (allocation_size == 0 || allocation_size > 0x100000000ULL) {  // 4GB limit
    spdlog::error("Invalid allocation size: 0x{:x}", allocation_size);
    return false;
  }

  uint64_t mapping_address = kFileMappingBaseAddress + GetNextOffset();

  relocated_binary_ = static_cast<uint8_t *>(mmap(
      reinterpret_cast<void *>(mapping_address), allocation_size,
      PROT_READ | PROT_WRITE,
      MAP_PRIVATE | MAP_ANONYMOUS | (mapping_address ? MAP_FIXED : 0), -1, 0));

  spdlog::info("{} -> 0x{:x}-0x{:x} (size: 0x{:x})", path_.c_str(),
               reinterpret_cast<uintptr_t>(relocated_binary_),
               reinterpret_cast<uintptr_t>(relocated_binary_) + allocation_size,
               allocation_size);

  if (relocated_binary_ == MAP_FAILED) {
    spdlog::error("Cannot allocate memory at address 0x{:x}", mapping_address);
    return false;
  }

  return true;
}

// Helper method to load LOAD segments properly
bool ElfLoader::LoadSegments() {
  auto *const elf_header = static_cast<Elf64_Ehdr *>(mapped_file_);
  auto *const program_headers = reinterpret_cast<Elf64_Phdr *>(
      static_cast<uint8_t *>(mapped_file_) + elf_header->e_phoff);

  for (int i = 0; i < elf_header->e_phnum; ++i) {
    const auto &phdr = program_headers[i];

    if (phdr.p_type != PT_LOAD) {
      continue;
    }

    // Calculate destination address
    if (phdr.p_vaddr < base_virtual_addr_) {
      spdlog::error("Segment virtual address 0x{:x} below base 0x{:x}",
                    phdr.p_vaddr, base_virtual_addr_);
      return false;
    }

    const uintptr_t offset = phdr.p_vaddr - base_virtual_addr_;
    if (offset + phdr.p_memsz > allocation_size) {
      spdlog::error("Segment extends beyond allocated memory");
      return false;
    }

    void *dest = reinterpret_cast<void *>(
        reinterpret_cast<uint64_t>(relocated_binary_) + offset);

    // Copy file data
    if (phdr.p_filesz > 0) {
      if (phdr.p_offset + phdr.p_filesz > std::filesystem::file_size(path_)) {
        spdlog::error("Segment file data extends beyond file size");
        return false;
      }

      const uint8_t *src = static_cast<uint8_t *>(mapped_file_) + phdr.p_offset;
      std::memcpy(dest, src, phdr.p_filesz);
    }

    // Zero-fill remaining memory
    if (phdr.p_memsz > phdr.p_filesz) {
      memset(static_cast<uint8_t *>(dest) + phdr.p_filesz, 0,
             phdr.p_memsz - phdr.p_filesz);
    }

    // Set memory protection
    // @TODO: Overlapping pages support
    const int protection = PROT_READ | PROT_WRITE | PROT_EXEC;
    // if (phdr.p_flags & PF_R) prot |= PROT_READ;
    // if (phdr.p_flags & PF_W) prot |= PROT_WRITE;
    // if (phdr.p_flags & PF_X) prot |= PROT_EXEC;

    // Align to page boundaries for mprotect
    const uintptr_t page_start = reinterpret_cast<uintptr_t>(dest) & ~0xFFF;
    const size_t page_size = ((phdr.p_memsz + 0xFFF) & ~0xFFF);

    if (mprotect(reinterpret_cast<void *>(page_start), page_size, protection) !=
        0) {
      spdlog::error("Failed to set memory protection for segment {}", i);
      return false;
    }

    // spdlog::info("Loaded segment {}: 0x{:x}-0x{:x} (prot: {}{}{})", i,
    //              phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz,
    //              (protection & PROT_READ) ? "R" : "-",
    //              (protection & PROT_WRITE) ? "W" : "-",
    //             (protection & PROT_EXEC) ? "X" : "-");
  }

  return true;
}

uint64_t ElfLoader::GetBinaryBase() const noexcept {
  return reinterpret_cast<uint64_t>(relocated_binary_);
}

bool ElfLoader::Load(std::shared_ptr<ElfLoader> loader) noexcept {
  auto *const elf_header = static_cast<Elf64_Ehdr *>(mapped_file_);
  auto *const section_header = reinterpret_cast<Elf64_Shdr *>(
      static_cast<uint8_t *>(mapped_file_) + elf_header->e_shoff);

  assert(elf_header->e_shstrndx != SHN_UNDEF);

  const auto str_tab = reinterpret_cast<char *>(
      static_cast<uint8_t *>(mapped_file_) +
      section_header[elf_header->e_shstrndx].sh_offset);

  Elf64_Shdr *dynsym_section = nullptr;
  Elf64_Shdr *dynstr_section = nullptr;
  Elf64_Shdr *symtab_section = nullptr;
  Elf64_Shdr *strtab_section = nullptr;
  Elf64_Shdr *dynamic_section = nullptr;
  Elf64_Shdr *rela_plt_section = nullptr;
  Elf64_Shdr *rela_dyn_section = nullptr;

  InitializeSymbolTable();
  LoadSegments();

  for (auto i = 0; i < elf_header->e_shnum; i++) {
    const auto &section = section_header[i];

    if (!(section.sh_flags & SHF_ALLOC)) {
      // Skip non-allocatable sections
      continue;
    }

    const auto *section_name = &str_tab[section.sh_name];

    // spdlog::info("Loading section {} (addr: 0x{:x}, size: {})", section_name,
    //              section.sh_addr, section.sh_size);

    // Calculate destination address relative to our base
    if (section.sh_addr < base_virtual_addr_) {
      spdlog::warn("Section address below base virtual address");
      continue;
    }

    const uintptr_t offset = section.sh_addr - base_virtual_addr_;
    if (offset >= allocation_size) {
      spdlog::error("Section offset beyond allocated memory");
      continue;
    }

    auto dest = reinterpret_cast<void *>(
        reinterpret_cast<uint64_t>(relocated_binary_) + offset);

    if (section.sh_type == SHT_NOBITS) {
      // BSS
      memset(dest, 0, section.sh_size);
    } else if (section.sh_size > 0) {
      // Section with data
      const uint8_t *src =
          static_cast<uint8_t *>(mapped_file_) + section.sh_offset;
      std::memcpy(dest, src, section.sh_size);
    }

    // Set protection
    // @TODO: Refactor
    const auto protection = PROT_READ | PROT_WRITE | PROT_EXEC;

    // Apply memory protection (align to page boundaries)
    const auto page_address = reinterpret_cast<uintptr_t>(dest) & ~0xFFF;
    const auto page_size = ((section.sh_size + 0xFFF) & ~0xFFF);
    if (mprotect(reinterpret_cast<void *>(page_address), page_size,
                 protection) != 0) {
      spdlog::error("Failed to set memory protection for section {}",
                    section_name);
      return false;
    }

    // Handle relocation sections
    if (section.sh_type == SHT_DYNSYM) {
      dynsym_section = &section_header[i];
      dynstr_section = &section_header[section.sh_link];
    } else if (section.sh_type == SHT_DYNAMIC) {
      dynamic_section = &section_header[i];
    }

    if (strcmp(section_name, ".rela.plt") == 0) {
      rela_plt_section = &section_header[i];
    } else if (strcmp(section_name, ".rela.dyn") == 0) {
      rela_dyn_section = &section_header[i];
    } else if (strcmp(section_name, ".symtab") == 0) {
      symtab_section = &section_header[i];
    } else if (strcmp(section_name, ".strtab") == 0) {
      strtab_section = &section_header[i];
    }
  }
  if (dynstr_section && dynsym_section) {
    for (auto symbols = GetSymbols(dynsym_section, dynstr_section);
         const auto &symbol : symbols) {
      if (symbol.address != 0) {
        // export
        exports_.push_back(ElfExport{
            .name = symbol.name,
            .address = symbol.relocated_address,
        });
      }
    }
  }
  if (symtab_section && strtab_section) {
    for (auto static_symbols = GetSymbols(symtab_section, strtab_section);
         const auto &symbol : static_symbols) {
      if (symbol.address != 0) {
        exports_.push_back(ElfExport{
            .name = symbol.name,
            .address = symbol.relocated_address,
        });
      }
    }
  }

  return true;
}

void ElfLoader::ProcessRelaSection(const int section_index,
                                   const Elf64_Shdr *section_header,
                                   const Elf64_Ehdr *elf_header) {
  const auto &rela_section = section_header[section_index];

  auto *const relocations = reinterpret_cast<Elf64_Rela *>(
      static_cast<uint8_t *>(mapped_file_) + rela_section.sh_offset);
  const int num_relocations = rela_section.sh_size / sizeof(Elf64_Rela);

  for (int i = 0; i < num_relocations; i++) {
    auto &[r_offset, r_info, r_addend] = relocations[i];
    ApplyRelocation(r_offset, ELF64_R_TYPE(r_info), ELF64_R_SYM(r_info),
                    r_addend);
  }
}

void ElfLoader::ProcessRelSection(const int section_index,
                                  const Elf64_Shdr *section_header,
                                  const Elf64_Ehdr *elf_header) {
  const auto &rel_section = section_header[section_index];

  auto *const relocations = reinterpret_cast<Elf64_Rel *>(
      static_cast<uint8_t *>(mapped_file_) + rel_section.sh_offset);
  const int num_relocations = rel_section.sh_size / sizeof(Elf64_Rel);

  for (int i = 0; i < num_relocations; i++) {
    auto &[r_offset, r_info] = relocations[i];
    ApplyRelocation(r_offset, ELF64_R_TYPE(r_info), ELF64_R_SYM(r_info), 0);
  }
}

void ElfLoader::ApplyRelocation(Elf64_Addr offset, int type, int sym_index,
                                Elf64_Sxword addend) {
  if (offset < base_virtual_addr_) {
    spdlog::error("Relocation offset below base address");
    return;
  }

  uintptr_t relative_offset = offset - base_virtual_addr_;
  if (relative_offset >= allocation_size) {
    spdlog::error("Relocation offset beyond allocated memory");
    return;
  }

  void *reloc_addr = reinterpret_cast<void *>(
      reinterpret_cast<uint64_t>(relocated_binary_) + relative_offset);

  switch (type) {
    case R_X86_64_RELATIVE:
      // Base address + addend
      *static_cast<uint64_t *>(reloc_addr) =
          reinterpret_cast<uintptr_t>(relocated_binary_) + addend -
          base_virtual_addr_;
      break;

    case R_X86_64_GLOB_DAT: {
      const uint64_t symbol_value = ResolveSymbol(sym_index);
      if (symbol_value == 0) {
        spdlog::error(
            "Failed to resolve symbol {} for R_X86_64_GLOB_DAT relocation",
            sym_index);
        return;
      }
      *static_cast<uint64_t *>(reloc_addr) = symbol_value;
    } break;

    case R_X86_64_64:
      // Symbol value + addend
      {
        uint64_t symbol_value = ResolveSymbol(sym_index);
        if (symbol_value == 0) {
          spdlog::error(
              "Failed to resolve symbol {} for R_X86_64_64 relocation",
              sym_index);
          return;
        }
        uint64_t final_value = symbol_value + addend;
        *static_cast<uint64_t *>(reloc_addr) = final_value;
      }
      break;
    case R_X86_64_TLSGD: {
      const uint64_t symbol_value = ResolveSymbol(sym_index);
      if (symbol_value == 0) {
        spdlog::error(
            "Failed to resolve symbol {} for R_X86_64_TLSGD relocation",
            sym_index);
        return;
      }
      *static_cast<uint64_t *>(reloc_addr) = symbol_value + addend;
    } break;

    case R_X86_64_32:
      spdlog::error("R_X86_64_32 relocation requires symbol resolution");
      break;
    case R_X86_64_IRELATIVE: {
      uintptr_t resolver_addr = reinterpret_cast<uintptr_t>(relocated_binary_) +
                                addend - base_virtual_addr_;

      spdlog::debug("R_X86_64_IRELATIVE: Calling resolver function at 0x{:x}",
                    resolver_addr);

      // Call the resolver function to get the actual function address
      typedef uint64_t (*resolver_func_t)();
      const auto resolver = reinterpret_cast<resolver_func_t>(resolver_addr);

      const uint64_t resolved_addr = resolver();
      *static_cast<uint64_t *>(reloc_addr) = resolved_addr;
    } break;
    case R_X86_64_JUMP_SLOT: {
      uint64_t symbol_value = ResolveSymbol(sym_index);
      if (symbol_value == 0) {
        spdlog::error(
            "Failed to resolve symbol {} for R_X86_64_JUMP_SLOT relocation",
            sym_index);
        return;
      }
      *static_cast<uint64_t *>(reloc_addr) = symbol_value;
    } break;

    default:
      spdlog::info("Unsupported relocation type: {}", type);
      break;
  }
}

uint64_t ElfLoader::ResolveSymbol(int sym_index) {
  if (sym_index == 0) {
    spdlog::debug("Symbol index 0 (STN_UNDEF) - undefined symbol");
    return 0;
  }

  if (!symbol_table_ || !string_table_) {
    spdlog::error("No symbol table or string table available");
    return 0;
  }

  if (sym_index < 0 || sym_index >= symbol_count_) {
    spdlog::error("Symbol index {} out of range (max: {})", sym_index,
                  symbol_count_ - 1);
    return 0;
  }

  const Elf64_Sym &symbol = symbol_table_[sym_index];

  // Get symbol name for debugging
  const auto *symbol_name = "";
  if (symbol.st_name < string_table_size_) {
    symbol_name = string_table_ + symbol.st_name;
  }

  spdlog::info("Resolving symbol {}: '{}' (type: {}, bind: {}, value: 0x{:x})",
               sym_index, symbol_name, ELF64_ST_TYPE(symbol.st_info),
               ELF64_ST_BIND(symbol.st_info), symbol.st_value);

  // Handle different symbol types and bindings
  switch (ELF64_ST_BIND(symbol.st_info)) {
    case STB_LOCAL:
    case STB_GLOBAL:
    case STB_WEAK:
      break;
    default:
      spdlog::warn("Unsupported symbol binding: {}",
                   ELF64_ST_BIND(symbol.st_info));
      break;
  }

  if (symbol.st_shndx == SHN_UNDEF) {
    if (uint64_t external_addr = ResolveExternalSymbol(symbol_name);
        external_addr != 0) {
      spdlog::debug("Resolved external symbol '{}' to 0x{:x}", symbol_name,
                    external_addr);
      return external_addr;
    }

    if (ELF64_ST_BIND(symbol.st_info) == STB_WEAK) {
      spdlog::error("Weak symbol '{}' unresolved, using 0", symbol_name);
      return 0;
    }

    spdlog::error("Undefined symbol '{}' could not be resolved", symbol_name);
    return 0;
  }

  uint64_t symbol_addr = 0;

  if (symbol.st_shndx == SHN_ABS) {
    symbol_addr = symbol.st_value;
  } else if (symbol.st_shndx == SHN_COMMON) {
    spdlog::error("Common symbol '{}' not yet supported", symbol_name);
    return 0;
  } else {
    if (symbol.st_value < base_virtual_addr_) {
      spdlog::error("Symbol '{}' value 0x{:x} below base address 0x{:x}",
                    symbol_name, symbol.st_value, base_virtual_addr_);
      return 0;
    }

    uintptr_t relative_offset = symbol.st_value - base_virtual_addr_;
    if (relative_offset >= allocation_size) {
      spdlog::error("Symbol '{}' offset 0x{:x} beyond allocated memory",
                    symbol_name, relative_offset);
      return 0;
    }

    symbol_addr =
        reinterpret_cast<uintptr_t>(relocated_binary_) + relative_offset;
  }

  return symbol_addr;
}

uint64_t ElfLoader::ResolveExternalSymbol(const char *symbol_name) const {
  if (!symbol_name || strlen(symbol_name) == 0) {
    return 0;
  }

  for (const auto &[name, address] : exports_) {
    if (symbol_name == name) {
      return reinterpret_cast<uint64_t>(&address);
    }
  }

  spdlog::error("External symbol '{}' not found in resolver", symbol_name);
  return 0;
}

// Helper function to initialize symbol table data
bool ElfLoader::InitializeSymbolTable() {
  const auto *const elf_header = static_cast<Elf64_Ehdr *>(mapped_file_);
  const auto *const section_headers = reinterpret_cast<Elf64_Shdr *>(
      static_cast<uint8_t *>(mapped_file_) + elf_header->e_shoff);

  for (int i = 0; i < elf_header->e_shnum; ++i) {
    const Elf64_Shdr &section = section_headers[i];

    if (section.sh_type == SHT_SYMTAB || section.sh_type == SHT_DYNSYM) {
      symbol_table_ = reinterpret_cast<const Elf64_Sym *>(
          static_cast<uint8_t *>(mapped_file_) + section.sh_offset);
      symbol_count_ = section.sh_size / sizeof(Elf64_Sym);

      if (section.sh_link < elf_header->e_shnum) {
        const Elf64_Shdr &strtab_section = section_headers[section.sh_link];
        string_table_ = reinterpret_cast<const char *>(
            static_cast<uint8_t *>(mapped_file_) + strtab_section.sh_offset);
        string_table_size_ = strtab_section.sh_size;

        return true;
      }
    }
  }

  spdlog::error("No symbol table found");
  return false;
}

void ElfLoader::ProcessRelocations(const Elf64_Shdr *rela_section,
                                   const Elf64_Sym *dynsym_table,
                                   const char *dynstr_table,
                                   std::vector<ElfImport> &imports,
                                   const bool is_plt) const noexcept {
  const auto *const relocations = reinterpret_cast<Elf64_Rela *>(
      static_cast<uint8_t *>(mapped_file_) + rela_section->sh_offset);
  const auto reloc_count = rela_section->sh_size / sizeof(Elf64_Rela);

  for (auto i = 0; i < reloc_count; i++) {
    const auto &reloc = relocations[i];

    const auto sym_index = ELF64_R_SYM(reloc.r_info);
    const auto reloc_type = ELF64_R_TYPE(reloc.r_info);

    if (sym_index == 0) {
      continue;
    }

    const auto &symbol = dynsym_table[sym_index];
    if (symbol.st_shndx != SHN_UNDEF) {
      continue;
    }

    auto import = ElfImport{
        .name = &dynstr_table[symbol.st_name],
        .relocation_offset = reloc.r_offset,
        .relocation_type = reloc_type,
        .plt_address = 0,
        .got_address = 0,
    };

    if (reloc.r_offset >= base_virtual_addr_) {
      const auto offset = reloc.r_offset - base_virtual_addr_;
      if (offset < allocation_size) {
        if (is_plt) {
          import.plt_address =
              reinterpret_cast<uintptr_t>(relocated_binary_) + offset;
        }
        import.got_address =
            reinterpret_cast<uintptr_t>(relocated_binary_) + offset;
      }
    }

    imports.push_back(import);
  }
}

uint64_t ElfLoader::GetEntryPoint() const noexcept {
  auto *const elf_header = static_cast<Elf64_Ehdr *>(mapped_file_);

  return reinterpret_cast<uintptr_t>(relocated_binary_) +
         (elf_header->e_entry - base_virtual_addr_);
}

std::vector<ElfSymbol> ElfLoader::GetSymbols(
    const Elf64_Shdr *dynsym, const Elf64_Shdr *dynstr) const noexcept {
  std::vector<ElfSymbol> symbol_table;

  const auto *const symbols = reinterpret_cast<Elf64_Sym *>(
      static_cast<uint8_t *>(mapped_file_) + dynsym->sh_offset);

  const auto *const string_table = reinterpret_cast<char *>(
      static_cast<uint8_t *>(mapped_file_) + dynstr->sh_offset);

  const size_t symbol_count = dynsym->sh_size / sizeof(Elf64_Sym);

  for (size_t i = 0; i < symbol_count; i++) {
    const auto &sym = symbols[i];

    // Skip undefined, local, and unnamed symbols for exports
    if (sym.st_name == 0) {
      continue;
    }

    ElfSymbol symbol{
        .name = &string_table[sym.st_name],
        .address = sym.st_value,
        .size = sym.st_size,
        .type = static_cast<uint8_t> ELF64_ST_TYPE(sym.st_info),
        .binding = static_cast<uint8_t> ELF64_ST_BIND(sym.st_info),
        .section_index = sym.st_shndx,
    };

    // Calculate relocated address
    if (sym.st_shndx != SHN_ABS && sym.st_value >= base_virtual_addr_) {
      symbol.relocated_address =
          reinterpret_cast<uintptr_t>(relocated_binary_) +
          (sym.st_value - base_virtual_addr_);
    } else {
      symbol.relocated_address = 0;
    }

    symbol_table.push_back(symbol);
  }

  return symbol_table;
}

uint64_t ElfLoader::GetNextOffset() noexcept {
  constexpr uint64_t kPredefinedMaxImageSize = 0x2000000;

  static uint64_t offset = 0;

  const auto current_offset = offset;

  offset += kPredefinedMaxImageSize;

  return current_offset;
}

uint64_t ElfLoader::GetMappedFileBase() const noexcept {
  return base_virtual_addr_;
}
