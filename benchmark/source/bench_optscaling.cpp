#define ANKERL_NANOBENCH_IMPLEMENT
#include <absl/container/flat_hash_map.h>
#include <nanobench.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ellalgo/cutting_plane.hpp>
#include <ellalgo/ell.hpp>
#include <limits>
#include <list>
#include <netoptim/optscaling_oracle.hpp>
#include <numbers>
#include <utility>
#include <valarray>

namespace {

    using CostGraph
        = absl::flat_hash_map<uint32_t, std::list<std::pair<uint32_t, std::pair<double, double>>>>;

    auto create_fixed_graph() -> CostGraph {
        const auto log10 = std::numbers::ln10;
        const auto log11 = std::log(11.0);
        const auto log12 = std::log(12.0);
        const auto log13 = std::log(13.0);
        const auto log14 = std::log(14.0);
        const auto log15 = std::log(15.0);
        const auto log16 = std::log(16.0);
        const auto log17 = std::log(17.0);
        const auto log18 = std::log(18.0);
        const auto log19 = std::log(19.0);
        const auto log20 = std::log(20.0);
        const auto log21 = std::log(21.0);
        const auto log22 = std::log(22.0);
        const auto log23 = std::log(23.0);
        const auto log24 = std::log(24.0);
        const auto log125 = std::log(125.0);

        return {
            {0, {{{2, {log22, log125}}, {3, {log16, log18}}, {4, {log15, log11}}}}},
            {1,
             {{{1, {log10, log10}},
               {2, {log20, log19}},
               {3, {log14, log12}},
               {4, {100.0, log21}}}}},
            {2, {{{0, {log125, log22}}, {1, {log19, log20}}, {2, {log13, log13}}}}},
            {3, {{{0, {log18, log16}}, {1, {log12, log14}}, {4, {log24, log23}}}}},
            {4,
             {{{0, {log11, log15}},
               {1, {log21, -100.0}},
               {3, {log23, log24}},
               {4, {log17, log17}}}}},
        };
    }

    auto fixed_cost = [](const std::pair<double, double>& edge_data) -> std::pair<double, double> {
        return edge_data;
    };

    struct Outcome {
        bool ok;
        std::size_t niter;
        double gamma;
    };

    auto run(const CostGraph& gra, double kappa, double tol) -> Outcome {
        const auto xinit = std::valarray<double>{std::log(125.0), std::numbers::ln10};
        auto ellip = Ell{kappa, xinit};
        absl::flat_hash_map<uint32_t, double> dist{
            {0, 0.0}, {1, 0.0}, {2, 0.0}, {3, 0.0}, {4, 0.0}};
        auto omega = OptScalingOracle(gra, dist, fixed_cost);
        auto gamma = std::numeric_limits<double>::infinity();
        const auto res = cutting_plane_optim(omega, ellip, gamma, Options{2000, tol});
        return {std::get<0>(res).size() != 0U, std::get<1>(res), gamma};
    }

}  // namespace

int main() {
    const auto gra = create_fixed_graph();
    const auto t = std::log(125.0) - std::numbers::ln10;

    const auto ref = run(gra, 200.0 * t, 1e-20);
    std::printf("reference: kappa=200t tol=1e-20 niter=%zu gamma=%.12g\n", ref.niter, ref.gamma);

    const double kmuls[] = {1.0, 2.0, 10.0, 50.0, 200.0};
    const double tols[] = {1e-8, 1e-10, 1e-12, 1e-14, 1e-16, 1e-18, 1e-20};
    std::printf("%8s %8s %7s %16s %10s\n", "kappa/t", "tol", "niter", "gamma", "dgamma");
    for (auto kmul : kmuls) {
        for (auto tol : tols) {
            const auto r = run(gra, kmul * t, tol);
            std::printf("%8g %8.0e %7zu %16.10g %10.2e\n", kmul, tol, r.niter, r.gamma,
                        std::abs(r.gamma - ref.gamma));
        }
    }

    ankerl::nanobench::Bench bench;
    bench.title("netoptim-cpp OptScalingOracle solve by tolerance")
        .unit("solve")
        .warmup(1)
        .epochs(5)
        .minEpochIterations(2);
    for (auto tol : {1e-8, 1e-10, 1e-12, 1e-20}) {
        char label[32];
        std::snprintf(label, sizeof(label), "tol=%.0e", tol);
        bench.run(label, [&] {
            const auto r = run(gra, 200.0 * t, tol);
            ankerl::nanobench::doNotOptimizeAway(r.gamma);
        });
    }
    return 0;
}
