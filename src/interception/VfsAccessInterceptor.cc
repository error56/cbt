#include "VfsAccessInterceptor.h"

bool VfsAccessInterceptor::RegisterOpenInterceptor(
    const VfsOpenInterceptor &func) noexcept {
  if (IsOpenInterceptorRegistered()) {
    return false;
  }

  open_present_ = true;
  open_ = func;

  return true;
}

bool VfsAccessInterceptor::RegisterReadInterceptor(
    const VfsReadInterceptor &func) noexcept {
  if (IsReadInterceptorRegistered()) {
    return false;
  }

  read_present_ = true;
  read_ = func;

  return true;
}

bool VfsAccessInterceptor::RegisterWriteInterceptor(
    const VfsWriteInterceptor& func) noexcept {
  if (IsWriteInterceptorRegistered()) {
    return false;
  }

  write_present_ = true;
  write_ = func;

  return true;
}

bool VfsAccessInterceptor::RegisterDeleteInterceptor(
    const VfsDeleteInterceptor& func) noexcept {
  if (IsDeleteInterceptorRegistered()) {
    return false;
  }

  delete_present_ = true;
  delete_ = func;

  return true;
}

bool VfsAccessInterceptor::IsOpenInterceptorRegistered() const noexcept {
  return open_present_;
}

bool VfsAccessInterceptor::IsReadInterceptorRegistered() const noexcept {
  return read_present_;
}

bool VfsAccessInterceptor::IsWriteInterceptorRegistered() const noexcept {
  return write_present_;
}

bool VfsAccessInterceptor::IsDeleteInterceptorRegistered() const noexcept {
  return delete_present_;
}

VfsOpenInterceptor VfsAccessInterceptor::GetOpenInterceptor() const noexcept {
  return open_;
}

VfsReadInterceptor VfsAccessInterceptor::GetReadInterceptor() const noexcept {
  return read_;
}

VfsWriteInterceptor VfsAccessInterceptor::GetWriteInterceptor() const noexcept {
  return write_;
}

VfsDeleteInterceptor VfsAccessInterceptor::GetDeleteInterceptor()
    const noexcept {
  return delete_;
}
