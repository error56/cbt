#ifndef OPTION_H
#define OPTION_H

#include <algorithm>

template <typename T>
class Option {
 private:
  bool has_value_;
  T value_;

 public:
  Option() : has_value_(false) {}
  explicit Option(const T& value) : has_value_(true), value_(value) {}
  explicit Option(T&& value) : has_value_(true), value_(std::move(value)) {}

  static Option Some(T value) { return *(new Option(value)); }
  static Option None() { return Option(); }

  ~Option() {
    if (has_value_) {
      value_.~T();
    }
  }

  [[nodiscard]] bool IsSome() const noexcept { return has_value_; }

  [[nodiscard]] bool IsNone() const noexcept { return !has_value_; }

  [[nodiscard]] T GetValue() const noexcept {
    return std::move(value_);
  }
};

#endif  // OPTION_H
