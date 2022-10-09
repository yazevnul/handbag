#include "lib/cpp/io/input.h"

#include <cstddef>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace handbag::io {
absl::Status readAll(IInputStream& input, std::string& dst) noexcept {
  std::vector<std::byte> buffer;
  // Page size seem to be a good default.
  buffer.resize(4ULL * 1024ULL);

  const auto dst_initial_size = dst.size();
  do {
    auto result = input.read(buffer.data(), buffer.size());
    if (result.ok()) {
      dst.append(reinterpret_cast<const char*>(buffer.data()), result.value());
    } else if (absl::IsResourceExhausted(result.status())) {
      return absl::OkStatus();
    } else {
      dst.resize(dst_initial_size);
      return std::move(result).status();
    }
  } while (true);

  ABSL_INTERNAL_UNREACHABLE;
}

absl::StatusOr<std::string> readAll(IInputStream& input) noexcept {
  std::string buffer;
  auto status = readAll(input, buffer);
  if (status.ok()) {
    return buffer;
  }

  return status;
}
}  // namespace handbag::io
