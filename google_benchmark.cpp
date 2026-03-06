#include <benchmark/benchmark.h>
#include <random>
#include "fast_list.h"  // Include your OrderBook header

// =========================================================
// BENCHMARK: OrderBook::Fast_list randomized
// =========================================================

static void BM_OrderBook_Add(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        {
            OrderBook warmup;
            for (int i = 0; i < N; i++) warmup.addOrder(i);
        } // destroyed here, memory freed cleanly
        state.ResumeTiming();

        OrderBook list;
        for (int i = 0; i < N; i++)
            list.addOrder(i);
        benchmark::DoNotOptimize(list);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_OrderBook_Remove(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        OrderBook list;
        for (int i = 0; i < N; i++) list.addOrder(i);
        state.ResumeTiming();

        for (int i = 0; i < N; i += 2)
            list.removeOrder(i);
        benchmark::DoNotOptimize(list);
    }
    state.SetItemsProcessed(state.iterations() * (N / 2));
}

static void BM_OrderBook_Consume(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        OrderBook list;
        for (int i = 0; i < N; i++) list.addOrder(i);
        state.ResumeTiming();

        while (list.book.count > 0)
            list.consumeOrder();
        benchmark::DoNotOptimize(list);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_OrderBook_Mixed(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        OrderBook list;
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> op_dist(0, 2);
        std::uniform_int_distribution<int> val_dist(0, N);
        state.ResumeTiming();

        for (int i = 0; i < N; ++i) {
            int op = op_dist(rng);
            if (op == 0)
                list.addOrder(val_dist(rng));
            else if (op == 1)
                list.removeOrder(val_dist(rng));
            else
                list.consumeOrder();
        }
        benchmark::DoNotOptimize(list);
    }
    state.SetItemsProcessed(state.iterations() * N);
}

BENCHMARK(BM_OrderBook_Add)    ->Arg(1000000)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_OrderBook_Remove) ->Arg(1000000)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_OrderBook_Consume)->Arg(1000000)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_OrderBook_Mixed)  ->Arg(1000000)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();