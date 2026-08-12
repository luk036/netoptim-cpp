#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <cstdint>
#include <digraphx/neg_cycle.hpp>
#include <py2cpp/dict.hpp>
#include <string>
#include <vector>

auto create_cycle_graph(uint32_t n) -> py::dict<uint32_t, py::dict<uint32_t, int>> {
    py::dict<uint32_t, py::dict<uint32_t, int>> gra;
    for (uint32_t i = 0; i < n; ++i) {
        py::dict<uint32_t, int> nbrs;
        uint32_t j = (i + 1) % n;
        int w = (j == 0) ? -3 : 1;  // last edge back to 0 is negative
        nbrs[j] = w;
        gra[i] = std::move(nbrs);
    }
    return gra;
}

int main() {
    auto sizes = {10u, 50u, 100u, 200u};
    auto get_weight = [](int w) -> int { return w; };

    for (auto n : sizes) {
        auto gra = create_cycle_graph(n);
        auto dist0 = py::dict<uint32_t, int>{};
        for (uint32_t i = 0; i < n; ++i) dist0[i] = 0;
        auto ncf0 = NegCycleFinder(gra);
        bool found = false;
        for ([[maybe_unused]] const auto& _ : ncf0.howard(dist0, get_weight)) {
            found = true;
        }
        std::printf("n=%u found=%s\n", n, found ? "yes" : "no");
    }

    ankerl::nanobench::Bench bench;
    bench.title("C++ NegCycleFinder (howard) by graph size")
        .unit("op")
        .warmup(100)
        .epochs(50)
        .minEpochIterations(1000);

    for (auto n : sizes) {
        auto gra = create_cycle_graph(n);
        bench.run("howard n=" + std::to_string(n), [&] {
            auto dist = py::dict<uint32_t, int>{};
            for (uint32_t i = 0; i < n; ++i) dist[i] = 0;
            auto ncf = NegCycleFinder(gra);
            bool cyc = false;
            for ([[maybe_unused]] const auto& _ : ncf.howard(dist, get_weight)) {
                cyc = true;
            }
            ankerl::nanobench::doNotOptimizeAway(cyc);
        });
    }

    return 0;
}
