// -*- coding: utf-8 -*-
#include <absl/container/flat_hash_map.h>
#include <doctest/doctest.h>

#include <cmath>
#include <cstdint>
#include <list>
#include <netoptim/optscaling_oracle.hpp>
#include <numbers>
#include <utility>
#include <valarray>

#include <ellalgo/cutting_plane.hpp>  // for cutting_plane_optim
#include <ellalgo/ell.hpp>            // for Ell

namespace {

    using CostGraph
        = absl::flat_hash_map<uint32_t, std::list<std::pair<uint32_t, std::pair<double, double>>>>;

    // Mirrors py/netoptim/tests/test_optscaling.py::create_fixed_graph
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
            {1, {{{1, {log10, log10}}, {2, {log20, log19}}, {3, {log14, log12}}, {4, {100.0, log21}}}}},
            {2, {{{0, {log125, log22}}, {1, {log19, log20}}, {2, {log13, log13}}}}},
            {3, {{{0, {log18, log16}}, {1, {log12, log14}}, {4, {log24, log23}}}}},
            {4, {{{0, {log11, log15}}, {1, {log21, -100.0}}, {3, {log23, log24}}, {4, {log17, log17}}}}},
        };
    }

}  // namespace

TEST_CASE("OptScalingOracle fixed graph via cutting_plane_optim") {
    auto gra = create_fixed_graph();

    // Mirrors py/netoptim/tests/test_optscaling.py::test_optscaling (fixed graph branch):
    // xinit = [cmax, cmin] = [log(125), log(10)], t = cmax - cmin,
    // ellip = Ell(200 * t, xinit), gamma = inf
    const auto log10 = std::numbers::ln10;
    const auto log125 = std::log(125.0);

    const auto xinit = std::valarray<double>{log125, log10};
    const auto t = log125 - log10;
    auto ellip = Ell{200.0 * t, xinit};

    auto get_cost = [](const std::pair<double, double>& edge_data) -> std::pair<double, double> {
        return edge_data;
    };
    absl::flat_hash_map<uint32_t, double> dist{{0, 0.0}, {1, 0.0}, {2, 0.0}, {3, 0.0}, {4, 0.0}};
    auto omega = OptScalingOracle(gra, dist, get_cost);

    auto gamma = std::numeric_limits<double>::infinity();
    const auto [xbest, num_iters] = cutting_plane_optim(omega, ellip, gamma);

    REQUIRE_NE(xbest.size(), 0U);
    CHECK_LE(num_iters, 2000U);
}
