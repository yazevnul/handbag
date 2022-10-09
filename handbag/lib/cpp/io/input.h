#pragma once

#include <cstddef>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "lib/cpp/io/fwd.h"

namespace handbag::io {
struct IInputStream {
  virtual ~IInputStream() = default;

  virtual absl::StatusOr<size_t> read(void* dst,
                                      size_t dst_capacity) noexcept = 0;

  virtual absl::Status close() noexcept = 0;
};

absl::Status readAll(IInputStream& input, std::string& dst) noexcept;
absl::StatusOr<std::string> readAll(IInputStream& input) noexcept;

}  // namespace handbag::io
