#include "benchmark/benchmark.h"
#include "lib/cpp/singleton/singleton.h"

namespace handbag {
namespace {

void BM_POD(benchmark::State& state) {
  for (const auto& x : state) {
    (void)x;

    const auto& foo = Singleton<int>();
    benchmark::DoNotOptimize(foo);
  }
}

BENCHMARK(BM_POD);
}  // namespace
}  // namespace handbag
