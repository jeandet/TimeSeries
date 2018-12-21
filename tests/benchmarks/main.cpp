#include <benchmark/benchmark.h>
#include <TimeSeries.h>
#include <numeric>
#include <vector>
#include <random>

USER_DECLARE_TS(MyTimeSerie, double);

static void BM_TS_STD_IOTA(benchmark::State& state) {
    MyTimeSerie ts(state.range(0));
    for (auto _ : state)
        std::iota(std::begin(ts), std::end(ts),0.);
}
BENCHMARK(BM_TS_STD_IOTA)->RangeMultiplier(2)->Range(128, 1<<20)->Complexity();

static void BM_TS_STD_SORT(benchmark::State& state) {
    MyTimeSerie ts(state.range(0));
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0., 1e10);
    std::generate(std::begin(ts), std::end(ts), [i=0., dis, gen]()mutable
        {
            return std::pair<double,double>{i++,dis(gen)};
        }
    );
    for (auto _ : state)
        std::sort(std::begin(ts), std::end(ts));
}
BENCHMARK(BM_TS_STD_SORT)->RangeMultiplier(2)->Range(128, 4096)->Complexity();


static void BM_TS_STD_IOTA_ON_STD_VECT(benchmark::State& state) {
    std::vector<double> vec(state.range(0));
    for (auto _ : state)
        std::iota(std::begin(vec), std::end(vec),0.);
}
BENCHMARK(BM_TS_STD_IOTA_ON_STD_VECT)->RangeMultiplier(2)->Range(128, 1<<20)->Complexity();

static void BM_TS_STD_SORT_ON_STD_VECT(benchmark::State& state) {
    std::vector<double> vec(state.range(0));
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0., 1e10);
    std::generate(std::begin(vec), std::end(vec), [i=0., dis, gen]()mutable
        {
            return dis(gen);
        }
    );
    for (auto _ : state)
        std::sort(std::begin(vec), std::end(vec));
}
BENCHMARK(BM_TS_STD_SORT_ON_STD_VECT)->RangeMultiplier(2)->Range(128, 4096)->Complexity();


BENCHMARK_MAIN();
