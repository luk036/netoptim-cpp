// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <cstdint>
#include <ellalgo/cutting_plane.hpp>
#include <ellalgo/ell.hpp>
#include <ellalgo/ell_config.hpp>
#include <limits>
#include <map>
#include <netoptim/css_oracle.hpp>
#include <utility>
#include <valarray>

namespace {

    using TimingGraph = std::map<uint32_t, std::map<uint32_t, netoptim_detail::timing_edge>>;
    using DistMap = std::map<uint32_t, double>;

    auto create_ratio_tcg() -> TimingGraph {
        return {
            {0, {{3, {6.0, 's'}}, {2, {7.0, 's'}}}},
            {1, {{2, {9.0, 's'}}, {4, {3.0, 'h'}}}},
            {2, {{0, {6.0, 'h'}}, {1, {6.0, 'h'}}, {3, {6.0, 's'}}}},
            {3, {{4, {8.0, 's'}}, {0, {6.0, 'h'}}, {2, {6.0, 'h'}}}},
            {4, {{1, {3.0, 's'}}, {3, {8.0, 'h'}}}},
        };
    }

    auto create_dist() -> DistMap { return {{0, 0.0}, {1, 0.0}, {2, 0.0}, {3, 0.0}, {4, 0.0}}; }

}  // namespace

TEST_CASE("Test ClockScheduleOracle feasible point") {
    auto gra = create_ratio_tcg();
    auto dist = create_dist();
    auto omega = ClockScheduleOracle(gra, dist);

    const auto x = std::valarray<double>{10.0, 1.0};
    auto gamma = std::numeric_limits<double>::infinity();
    const auto [cut, shrunk] = omega.assess_optim(x, gamma);

    CHECK(shrunk);
    CHECK_EQ(gamma, doctest::Approx(10.0));
    CHECK_EQ(std::get<0>(cut).size(), 2U);
}

TEST_CASE("Test ClockScheduleOracle minimize ratio") {
    auto gra = create_ratio_tcg();
    auto dist = create_dist();
    auto omega = ClockScheduleOracle(gra, dist);

    auto space = Ell{100.0, std::valarray<double>{7.5, 1.0}};
    auto gamma = std::numeric_limits<double>::infinity();
    const auto options = Options{2000, 1e-10};
    const auto [xbest, niter] = cutting_plane_optim(omega, space, gamma, options);

    CHECK_NE(xbest.size(), 0U);
    CHECK_LT(niter, options.max_iters);
    CHECK_EQ(gamma, doctest::Approx(3.040717067415985));
}
