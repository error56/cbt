#ifndef RESULT_H
#define RESULT_H

#include <assert.h>

#include <algorithm>

template <typename T, typename E>
class Result {

  bool is_ok_;
  union {
    T value;
    E error;
  };

 public:
  Result() = delete;

  explicit Result(const T& value) : is_ok_(true), value(value) {}
  explicit Result(T&& value) : is_ok_(true), value(std::move(value)) {}

  explicit Result(const E& error) : is_ok_(false), error(error) {}
  explicit Result(E&& error) : is_ok_(false), error(std::move(error)) {}

  static Result Ok(const T& value) { return Result(value); }
  static Result Ok(T&& value) { return Result(std::move(value)); }
  static Result Error(const E& error) { return Result(error); }
  static Result Error(E&& error) { return Result(std::move(error)); }

  ~Result() {
    if (is_ok_) {
      value.~T();
    } else {
      error.~E();
    }
  }

  [[nodiscard]] bool IsOk() const noexcept { return is_ok_; }
  [[nodiscard]] bool IsErr() const noexcept { return !is_ok_; }

  [[nodiscard]] T GetValue() const noexcept {
    assert(is_ok_);

    return value;
  }

  [[nodiscard]] E GetError() const noexcept {
    assert(!is_ok_);

    return error;
  }
};

#endif  // RESULT_H
