#ifndef CONTEXTSWITCHGENERATOR_H
#define CONTEXTSWITCHGENERATOR_H
#include <cstdint>
#include <cstdlib>

#include "../emulator/syscall/memory/MunmapSyscallEmulator.h"

struct ProcessorState;

// Class responsible for generating the context switch assembly stub, that
// performs lossless change of registers between guest/host or host/guest.
//
// Generation is split into two different functions, because the code is
// extensive, and it's easy to make a mistake while programming, because the
// code is mostly the same, the single thing that changes is direction of
// transfers.
class ContextSwitchGenerator {
  // Reference to structure containing entire CPU state. It's the structure
  // that's going to be written to and read from while saving/restoring state.
  ProcessorState& cpu_state_;

  // Allocated memory size
  uint64_t context_switch_from_translator_to_guest_allocation_size_{0};

  // Allocated memory size
  uint64_t context_switch_from_guest_to_translator_allocation_size_{0};

  // Generates context switch stub responsible for transition from the
  // translator to the guest.
  void GenerateContextSwitchFromTranslatorToGuest() noexcept;

  // Generates context switch stub responsible for transition from the guest to
  // the translator.
  void GenerateContextSwitchFromGuestToTranslator() noexcept;

 public:
  void* context_switch_from_translator_to_guest{};
  void* context_switch_from_guest_to_translator{};

  explicit ContextSwitchGenerator(ProcessorState& cpu_state)
      : cpu_state_(cpu_state) {
    this->GenerateContextSwitchFromGuestToTranslator();
    this->GenerateContextSwitchFromTranslatorToGuest();
  };

  ~ContextSwitchGenerator() {
    if (context_switch_from_guest_to_translator != nullptr) {
      munmap(context_switch_from_guest_to_translator,
             context_switch_from_guest_to_translator_allocation_size_);
    }
    if (context_switch_from_translator_to_guest != nullptr) {
      munmap(context_switch_from_translator_to_guest,
             context_switch_from_translator_to_guest_allocation_size_);
    }
  }
};

#endif  // CONTEXTSWITCHGENERATOR_H
