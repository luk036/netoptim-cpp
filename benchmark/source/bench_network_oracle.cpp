#define ANKERL_NANOBENCH_IMPLEMENT
#include <absl/container/flat_hash_map.h>
#include <fmt/format.h>
#include <nanobench.h>

#include <cstdint>
#include <list>
#include <netoptim/network_oracle.hpp>
#include <string>
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
    const size_t sizes[] = {20000, 50000, 100000, 200000, 500000, 1000000};

    for (auto n : sizes) {
        auto gra = build_graph(n);
        size_t edge_count = 0;
        for (const auto& [u, nbrs] : gra) edge_count += nbrs.size();

        MockOracle oracle;
        flat_hash_map<uint32_t, double> dist;
        for (uint32_t i = 0; i < n; ++i) dist[i] = 0.0;
        auto network = NetworkOracle(gra, dist, oracle);
        bool found = network.assess_feas(0.0).has_value();
        fmt::print("n={:<8} edges={:<9} cut={:<3}\n", n, edge_count, found ? "yes" : "no");
    }

    ankerl::nanobench::Bench bench;
    bench.title("netoptim-cpp NetworkOracle assess_feas sweep")
        .unit("op")
        .warmup(1)
        .epochs(3)
        .minEpochIterations(3);

    for (auto n : sizes) {
        auto gra = build_graph(n);
        bench.run("assess_feas n=" + std::to_string(n), [&] {
            flat_hash_map<uint32_t, double> d;
            for (uint32_t i = 0; i < n; ++i) d[i] = 0.0;
            NetworkOracle net(gra, d, MockOracle{});
            auto cut = net.assess_feas(0.0);
            ankerl::nanobench::doNotOptimizeAway(cut);
        });
    }
    return 0;
}
