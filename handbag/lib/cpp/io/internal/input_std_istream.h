#pragma once

#include <cstddef>
#include <istream>

#include "absl/base/optimization.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "lib/cpp/io/input.h"

namespace handbag::io::internal {
class InputStreamIstreamWrapper : public IInputStream {
 public:
  explicit InputStreamIstreamWrapper(std::istream& wrappee)
      : wrappee_(&wrappee) {}
  InputStreamIstreamWrapper(const InputStreamIstreamWrapper&) = delete;
  InputStreamIstreamWrapper& operator=(const InputStreamIstreamWrapper&) =
      delete;
  InputStreamIstreamWrapper(InputStreamIstreamWrapper&&) = default;
  InputStreamIstreamWrapper& operator=(InputStreamIstreamWrapper&&) = default;
  ~InputStreamIstreamWrapper() override { /* CHECK(isClosed()); */
  }

  absl::StatusOr<size_t> read(void* dst,
                              const size_t dst_capacity) noexcept final {
    if (ABSL_PREDICT_FALSE(isClosed())) {
      return absl::FailedPreconditionError("Closed");
    } else if (wrappee_->eof()) {
      return absl::ResourceExhaustedError("EOF");
    }
    /// .....
  }

 private:
  bool isClosed() const noexcept {
    auto res = wrappee_ == nullptr;
    return res;
  }

 private:
  std::istream* wrappee_ = nullptr;
};
}  // namespace handbag::io::internal
