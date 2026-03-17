#include <benchmark/benchmark.h>

#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <cstdint>

#include <mysort.hpp>

// ═══════════════════════════════════════════════════════════════════════════
//  Тип элементов массива — uint32_t, чтобы удовлетворить концепт radix sort
// ═══════════════════════════════════════════════════════════════════════════

using elem_t = uint32_t;

// ═══════════════════════════════════════════════════════════════════════════
//  Степени сортированности
// ═══════════════════════════════════════════════════════════════════════════

enum class Sortedness { Sorted0, Sorted20, Sorted40, Sorted60, Sorted80, Sorted100, Random };

static const char* sortedness_label(Sortedness s) {
    switch (s) {
        case Sortedness::Sorted0:       return "reverse sorted";
        case Sortedness::Sorted20:      return "20\% sorted";
        case Sortedness::Sorted40:      return "40\% sorted";
        case Sortedness::Sorted60:      return "60\% sorted";
        case Sortedness::Sorted80:      return "80\% sorted";
        case Sortedness::Sorted100:     return "fully sorted";
        case Sortedness::Random:        return "random";
    }
    return "";
}

// ═══════════════════════════════════════════════════════════════════════════
//  Генератор входных данных
// ═══════════════════════════════════════════════════════════════════════════

static std::vector<elem_t> make_array(int n, Sortedness level, unsigned seed = 42) {
    // Заполняем случайными uint32_t значениями через mt19937
    std::mt19937 rng(seed);
    std::uniform_int_distribution<elem_t> dist(
        std::numeric_limits<elem_t>::min(),
        std::numeric_limits<elem_t>::max()
    );

    std::vector<elem_t> arr(n);
    std::generate(arr.begin(), arr.end(), [&]{ return dist(rng); });

    switch (level) {
        case Sortedness::Sorted0:
            std::sort(arr.begin(), arr.end(), std::greater<elem_t>{}); 
            break;
        
        case Sortedness::Sorted20:
        {
            // сортируем, затем делаем ~80% случайных соседних свапов
            std::sort(arr.begin(), arr.end());
            int swaps = std::max(1, n * 80 / 100);
            std::uniform_int_distribution<int> idx(0, n - 2);
            for (int i = 0; i < swaps; ++i)
                std::swap(arr[idx(rng)], arr[idx(rng) + 1]);
            break;
        }

        case Sortedness::Sorted40:
        {
            std::sort(arr.begin(), arr.end());
            int swaps = std::max(1, n * 60 / 100);
            std::uniform_int_distribution<int> idx(0, n - 2);
            for (int i = 0; i < swaps; ++i)
                std::swap(arr[idx(rng)], arr[idx(rng) + 1]);
            break;
        }

        case Sortedness::Sorted60:
        {
            std::sort(arr.begin(), arr.end());
            int swaps = std::max(1, n * 40 / 100);
            std::uniform_int_distribution<int> idx(0, n - 2);
            for (int i = 0; i < swaps; ++i)
                std::swap(arr[idx(rng)], arr[idx(rng) + 1]);
            break;
        }

        case Sortedness::Sorted80:
        {
            std::sort(arr.begin(), arr.end());
            int swaps = std::max(1, n * 20 / 100);
            std::uniform_int_distribution<int> idx(0, n - 2);
            for (int i = 0; i < swaps; ++i)
                std::swap(arr[idx(rng)], arr[idx(rng) + 1]);
            break;
        }

        case Sortedness::Sorted100:
        {
            std::sort(arr.begin(), arr.end());
            break;
        }

        case Sortedness::Random:
            // уже случайный
            break;
    }

    return arr;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Шаблон бенчмарка
//  state.range(0) — размер массива
//  state.range(1) — степень сортированности (cast to Sortedness)
// ═══════════════════════════════════════════════════════════════════════════

template <typename SortFn>
static void run_benchmark(benchmark::State& state, SortFn sort_fn) {
    const int n      = static_cast<int>(state.range(0));
    const auto level = static_cast<Sortedness>(state.range(1));

    const std::vector<elem_t> base = make_array(n, level);

    std::vector<elem_t> arr;
    for (auto _ : state) {
        state.PauseTiming();
        arr = base;
        state.ResumeTiming();
        sort_fn(arr);
    }

    state.SetComplexityN(n);
    state.SetLabel(sortedness_label(level));
}

// ═══════════════════════════════════════════════════════════════════════════
//  Вспомогательная функция регистрации
// ═══════════════════════════════════════════════════════════════════════════

static const std::vector<int> SMALL_SIZES = { 10, 100, 1000, 10000 };
static const std::vector<int> BIG_SIZES = { 10, 100, 1000, 10000, 100000, 1000000, 10000000 };

static const std::vector<std::pair<const char*, Sortedness>> LEVELS = {
    { "reverse sorted", Sortedness::Sorted0     },
    { "20\% sorted",    Sortedness::Sorted20    },
    { "40\% sorted",    Sortedness::Sorted40    },
    { "60\% sorted",    Sortedness::Sorted60    },
    { "80\% sorted",    Sortedness::Sorted80    },
    { "fully sorted",   Sortedness::Sorted100   },
    { "random",         Sortedness::Random      }
};

template <typename SortFn>
static void register_slow_sort(const char* name, SortFn sort_fn) {
    for (int sz : SMALL_SIZES) {
        for (auto& [label, level] : LEVELS) {
            benchmark::RegisterBenchmark(
                name,
                [sort_fn](benchmark::State& st) { run_benchmark(st, sort_fn); }
            )
            ->Args({ sz, static_cast<int>(level) })
            ->Unit(benchmark::kMicrosecond)
            ->Iterations(1);
        }
    }
}

template <typename SortFn>
static void register_fast_sort(const char* name, SortFn sort_fn) {
    for (int sz : BIG_SIZES) {
        for (auto& [label, level] : LEVELS) {
            benchmark::RegisterBenchmark(
                name,
                [sort_fn](benchmark::State& st) { run_benchmark(st, sort_fn); }
            )
            ->Args({ sz, static_cast<int>(level) })
            ->Unit(benchmark::kMicrosecond)
            ->Iterations(1);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  main: регистрируем сортировки, затем передаём управление Google Benchmark
// ═══════════════════════════════════════════════════════════════════════════

int main(int argc, char** argv) {
    // ── Добавьте сюда свои сортировки ─────────────────────────────────────
    register_slow_sort("BubbleSort",                 [](std::vector<elem_t>& arr) {MySort::BubbleSort(               arr.begin(), arr.end(), std::less<elem_t>{} );} );
    register_slow_sort("BubbleSortWithCondition",    [](std::vector<elem_t>& arr) {MySort::BubbleSortWithCondition(  arr.begin(), arr.end(), std::less<elem_t>{} );} );
    register_slow_sort("ShakerSort",                 [](std::vector<elem_t>& arr) {MySort::ShakerSort(               arr.begin(), arr.end(), std::less<elem_t>{} );} );
    register_slow_sort("CombSort",                   [](std::vector<elem_t>& arr) {MySort::CombSort(                 arr.begin(), arr.end(), std::less<elem_t>{} );} );
    register_fast_sort("RadixSortBase10",            [](std::vector<elem_t>& arr) {MySort::RadixSort<10>(            arr.begin(), arr.end() );} );
    register_fast_sort("RadixSortBase20",            [](std::vector<elem_t>& arr) {MySort::RadixSort<20>(            arr.begin(), arr.end() );} );
    register_fast_sort("RadixSortBase100",           [](std::vector<elem_t>& arr) {MySort::RadixSort<100>(           arr.begin(), arr.end() );} );
    register_fast_sort("StdSort",                    [](std::vector<elem_t>& arr) {std::sort(                        arr.begin(), arr.end(), std::less<elem_t>{} );});
    // ──────────────────────────────────────────────────────────────────────

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}