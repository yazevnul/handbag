#pragma once

#include <cstring>
#include <limits>
#include <type_traits>

#include "absl/base/optimization.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "lib/cpp/io/input.h"

namespace handbag::io::internal {
template <typename T>
class InMemoryInputStream : public IInputStream {
 public:
  explicit InMemoryInputStream(T data) noexcept(
      std::is_nothrow_move_constructible_v<T>)
      : data_(std::move(data)) {}
  InMemoryInputStream(const InMemoryInputStream&) = delete;
  InMemoryInputStream& operator=(const InMemoryInputStream&) = delete;
  InMemoryInputStream(InMemoryInputStream&&) = default;
  InMemoryInputStream& operator=(InMemoryInputStream&&) = default;

  absl::StatusOr<size_t> read(void* const dst,
                              const size_t dst_capacity) noexcept override {
    const size_t data_size = data_.size();
    if (ABSL_PREDICT_FALSE(isClosed())) {
      return absl::FailedPreconditionError("Closed");
    } else if (ABSL_PREDICT_FALSE(cursor_ >= data_size)) {
      return absl::ResourceExhaustedError("EOF");
    }

    const auto bytes_available = data_size - cursor_;
    const auto bytes_to_read =
        bytes_available < dst_capacity ? bytes_available : dst_capacity;
    std::memmove(dst, data_.data() + cursor_, bytes_to_read);
    cursor_ += bytes_to_read;

    return bytes_to_read;
  }

  absl::Status close() noexcept override {
    if (ABSL_PREDICT_FALSE(isClosed())) {
      return absl::FailedPreconditionError("Already closed.");
    }

    // no-op
    return absl::OkStatus();
  }

 private:
  bool isClosed() const noexcept {
    return cursor_ == std::numeric_limits<size_t>::max();
  }

 private:
  T data_;
  size_t cursor_ = 0;
};
}  // namespace handbag::io::internal
