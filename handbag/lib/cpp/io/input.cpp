#include "lib/cpp/io/input.h"

#include <string>
#include <vector>
#include <cstddef>

#include "absl/status/statusor.h"
#include "absl/status/status.h"

namespace handbag::io {
absl::Status readAll(IInputSteam& input, std::string& dst) noexcept {
    std::vector<std::byte> buffer;
    // Page size seem to be a good default.
    buffer.reserve(4ULL * 1024ULL);

    const auto dst_initial_size = dst.size();
    do {
        auto result = input.read(buffer.data(), buffer.size());
        if (result.ok()) {
            dst.append(reinterpret_cast<const char*>(buffer.data()), result.value());
        } else if (absl::IsResourceExhausted(result)) {
            return OkStatus();
        } else {
            dst.resize(dst_initial_size);
            return std::move(result).status();
        }
    } while (true);

    ABSL_INTERNAL_UNREACHABLE;
}

absl::StatusOr<std::string> readAll(IInputStream& input) noexcept {
    std::string buffer;
    auto result = readAll(input, buffer);
    if (result.ok()) {
        return buffer;
    }

    return std::move(result).status();
}
}
