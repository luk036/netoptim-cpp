// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <list>
#include <netoptim/optscaling_oracle.hpp>
#include <numbers>
#include <unordered_map>
#include <utility>
#include <valarray>

TEST_CASE("Test OptScalingOracle assess_optim with negative cycle") {
    using CostGraph
        = std::unordered_map<uint32_t, std::list<std::pair<uint32_t, std::pair<double, double>>>>;

    // 3-cycle 0->1->2->0 with eval sum = -7 < 0 triggers negative cycle
    CostGraph gra{
        {0, {{{1, {4.0, 4.0}}, {2, {1.0, 1.0}}}}},
        {1, {{{0, {4.0, 4.0}}, {2, {4.0, 4.0}}}}},
        {2, {{{0, {1.0, 1.0}}, {1, {4.0, 4.0}}}}},
    };

    auto get_cost = [](const std::pair<double, double>& edge_data) -> std::pair<double, double> {
        return edge_data;
    };

    auto x = std::valarray<double>{0.0, 0.0};
    std::unordered_map<uint32_t, double> dist{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto omega = OptScalingOracle(gra, dist, get_cost);
    auto gamma = std::numeric_limits<double>::infinity();

    const auto [cut, shrunk] = omega.assess_optim(x, gamma);
    CHECK_FALSE(shrunk);
    const auto& [g, f] = cut;
    CHECK_EQ(g.size(), 2);
}

TEST_CASE("Test OptScalingOracle assess_optim feasible not better") {
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

    std::unordered_map<uint32_t, double> dist{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto omega = OptScalingOracle(gra, dist, get_cost);

    // First call: feasible, better than inf -> sets t
    auto x1 = std::valarray<double>{log125, log10};
    auto gamma = std::numeric_limits<double>::infinity();
    const auto [cut1, shrunk1] = omega.assess_optim(x1, gamma);
    CHECK(shrunk1);
    CHECK_EQ(gamma, doctest::Approx(log125 - log10));

    // Second call with same x: s == gamma -> fj = 0 -> not better (shrunk=false)
    auto x2 = std::valarray<double>{log125, log10};
    const auto [cut2, shrunk2] = omega.assess_optim(x2, gamma);
    CHECK_FALSE(shrunk2);
    const auto& [g2, f2] = cut2;
    CHECK_EQ(g2.size(), 2);
    CHECK_EQ(f2, doctest::Approx(0.0));
}

TEST_CASE("Test OptScalingOracle operator()") {
    using CostGraph
        = std::unordered_map<uint32_t, std::list<std::pair<uint32_t, std::pair<double, double>>>>;

    const auto log10 = std::numbers::ln10;
    const auto log125 = std::log(125.0);

    CostGraph gra{
        {0, {{{1, {log125, log125}}, {2, {log10, log10}}}}},
        {1, {{{0, {log125, log125}}, {2, {log10, log10}}}}},
        {2, {{{0, {log10, log10}}, {1, {log10, log10}}}}},
    };

    auto get_cost = [](const std::pair<double, double>& edge_data) -> std::pair<double, double> {
        return edge_data;
    };

    auto x = std::valarray<double>{log125, log10};
    std::unordered_map<uint32_t, double> dist{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto omega = OptScalingOracle(gra, dist, get_cost);
    auto gamma = std::numeric_limits<double>::infinity();

    const auto [cut, shrunk] = omega(x, gamma);
    CHECK(shrunk);
}
