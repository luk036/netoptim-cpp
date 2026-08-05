// -*- coding: utf-8 -*-
// Benchmark: NegCycleFinder::howard across all three languages
// Compile with xmake from netoptim-cpp root

#include <chrono>
#include <cstdint>
#include <digraphx/neg_cycle.hpp>
#include <iomanip>
#include <iostream>
#include <py2cpp/dict.hpp>
#include <string>
#include <vector>

// Create a cycle graph with a negative edge at the end.
// Same topology as the Rust benchmark: 0->1->2->...->(n-1)->0 with one negative edge.
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

template <typename F> double time_howard(const py::dict<uint32_t, py::dict<uint32_t, int>>& gra,
                                         uint32_t n, F get_weight, int iterations) {
    // Warmup
    {
        auto dist = py::dict<uint32_t, int>{};
        for (uint32_t i = 0; i < n; ++i) dist[i] = 0;
        auto ncf = NegCycleFinder(gra);
        for ([[maybe_unused]] const auto& _ : ncf.howard(dist, get_weight)) {
        }
    }

    auto total = 0.0;
    for (int iter = 0; iter < iterations; ++iter) {
        auto dist = py::dict<uint32_t, int>{};
        for (uint32_t i = 0; i < n; ++i) dist[i] = 0;
        auto ncf = NegCycleFinder(gra);

        auto start = std::chrono::high_resolution_clock::now();
        for ([[maybe_unused]] const auto& _ : ncf.howard(dist, get_weight)) {
        }
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed = std::chrono::duration<double, std::nano>(end - start).count();
        total += elapsed;
    }
    return total / iterations;
}

int main() {
    std::cout << "=== C++ NegCycleFinder Benchmark ===" << std::endl;
    std::cout << std::left << std::setw(12) << "Nodes" << std::setw(20) << "Time (ns)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    auto sizes = {10u, 50u, 100u, 200u};
    auto get_weight = [](int w) -> int { return w; };

    for (auto n : sizes) {
        auto gra = create_cycle_graph(n);
        // More iterations for smaller graphs to get stable measurements
        int iters = (n <= 10) ? 50000 : (n <= 50) ? 10000 : 5000;
        auto avg_ns = time_howard(gra, n, get_weight, iters);
        std::cout << std::left << std::setw(12) << n << std::fixed << std::setprecision(2)
                  << std::setw(20) << avg_ns << std::endl;
    }

    return 0;
}
