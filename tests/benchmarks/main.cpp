#include <benchmark/benchmark.h>
#include <TimeSeries.h>
#include <numeric>
#include <vector>

static void BM_TS_STD_IOTA(benchmark::State& state) {
    TimeSeries::ScalarTs ts(state.range(0));
    for (auto _ : state)
        std::iota(std::begin(ts), std::end(ts),0.);
}
BENCHMARK(BM_TS_STD_IOTA)->RangeMultiplier(2)->Range(128, 1<<20)->Complexity();

static void BM_TS_STD_IOTA_ON_STD_VECT(benchmark::State& state) {
    std::vector<double> vec(state.range(0));
    for (auto _ : state)
        std::iota(std::begin(vec), std::end(vec),0.);
}
BENCHMARK(BM_TS_STD_IOTA_ON_STD_VECT)->RangeMultiplier(2)->Range(128, 1<<20)->Complexity();

BENCHMARK_MAIN();
