#include <absl/container/flat_hash_map.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <list>
#include <netoptim/network_oracle.hpp>
#include <utility>
#include <vector>

using absl::flat_hash_map;
using std::list;
using std::pair;

using BenchGraph = flat_hash_map<uint32_t, list<pair<uint32_t, double>>>;

struct MockOracle {
    auto eval(double edge, double) const -> double { return edge; }
    auto grad(double, double) const -> double { return 0.0; }
    void update(double) {}
};

static auto build_graph(size_t n_nodes, int k = 3) -> BenchGraph {
    BenchGraph g;
    for (uint32_t i = 0; i < n_nodes; ++i) {
        list<pair<uint32_t, double>> nbrs;
        for (int d = 1; d <= k; ++d) {
            auto j = static_cast<uint32_t>((i + d) % n_nodes);
            double w = static_cast<double>(((i + 1) * 7 + (j + 1) * 13) % 100 + 1);
            nbrs.emplace_back(j, w);
        }
        g[i] = std::move(nbrs);
    }
    if (n_nodes > 2) {
        g[0].emplace_back(1, -5.0);
        g[1].emplace_back(2, -5.0);
        g[2].emplace_back(0, -5.0);
    }
    return g;
}

int main() {
    std::printf("=== netoptim-cpp: NetworkOracle (assess_feas) ===\n");
    std::printf("%-12s %-10s %-6s %-12s\n", "Nodes", "Edges", "Cut?", "Avg(ms)");
    const size_t sizes[] = {20000, 50000, 100000, 200000, 500000, 1000000};
    const int n_runs = 5;
    double ref_ms = 0.0;
    for (auto n : sizes) {
        auto gra = build_graph(n);
        size_t edge_count = 0;
        for (const auto& [u, nbrs] : gra) edge_count += nbrs.size();

        MockOracle oracle;
        flat_hash_map<uint32_t, double> dist;
        for (uint32_t i = 0; i < n; ++i) dist[i] = 0.0;
        auto network = NetworkOracle(gra, dist, oracle);

        // Warmup
        bool found = network.assess_feas(0.0).has_value();

        double total_ms = 0.0;
        for (int run = 0; run < n_runs; ++run) {
            flat_hash_map<uint32_t, double> d;
            for (uint32_t i = 0; i < n; ++i) d[i] = 0.0;
            NetworkOracle net(gra, d, MockOracle{});
            auto start = std::chrono::high_resolution_clock::now();
            auto cut = net.assess_feas(0.0);
            auto end = std::chrono::high_resolution_clock::now();
            total_ms += std::chrono::duration<double, std::milli>(end - start).count();
            if (run == 0) found = cut.has_value();
        }
        double avg = total_ms / n_runs;
        if (ref_ms == 0.0) ref_ms = avg;
        std::printf("%-12zu %-10zu %-6s %-12.2f\n",
                    n, edge_count, found ? "yes" : "no", avg);
    }
    return 0;
}
