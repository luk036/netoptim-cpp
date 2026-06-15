// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <cmath>    // for log
#include <cstdint>  // for uint32_t
#include <limits>   // for infinity
#include <list>
#include <netoptim/optscaling_oracle.hpp>
#include <numbers>
#include <unordered_map>
#include <utility>   // for pair
#include <valarray>  // for valarray

TEST_CASE("Test OptScalingOracle assess_optim") {
    using CostGraph
        = std::unordered_map<uint32_t, std::list<std::pair<uint32_t, std::pair<double, double>>>>;

    const auto log10 = std::numbers::ln10;
    const auto log22 = std::log(22.0);
    const auto log125 = std::log(125.0);

    CostGraph gra{
        {0, {{{1, {log22, log125}}, {2, {log10, log10}}}}},
        {1, {{{0, {log125, log22}}, {2, {log10, log10}}}}},
        {2, {{{0, {log10, log10}}, {1, {log10, log10}}}}},
    };

    auto get_cost = [](const std::pair<double, double>& edge_data) -> std::pair<double, double> {
        return edge_data;
    };

    auto x = std::valarray<double>{log125, log10};
    std::unordered_map<uint32_t, double> dist{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto omega = OptScalingOracle(gra, dist, get_cost);
    auto gamma = std::numeric_limits<double>::infinity();

    const auto [cut, shrunk] = omega.assess_optim(x, gamma);
    CHECK(shrunk);
}
