// -*- coding: utf-8 -*-
#include <doctest/doctest.h>  // for ResultBuilder, CHECK

#include <array>  // for array
#include <iostream>
#include <netoptim/min_cycle_ratio.hpp>       // for min_cycle_ratio
#include <py2cpp/dict.hpp>                    // for dict
#include <py2cpp/enumerate.hpp>               // for iterable_wrapper
#include <py2cpp/fractions.hpp>               // for Fraction, operator-
#include <vector>                             // for vector
#include <xnetwork/classes/digraphs.hpp>      // for DiGraphS
#include <xnetwork/generators/testcases.hpp>  // for create_test_case1,

TEST_CASE("Test Cycle Ratio") {
    const auto indices = std::array<uint32_t, 5>{0, 1, 2, 3, 4};
    auto gra = create_test_case1(indices);

    const auto cost = std::array<int, 5>{5, 1, 1, 1, 1};

    // ponytail: edge data is int (index into cost array)
    const auto get_cost = [&](int edge_idx) -> int { return cost[edge_idx]; };
    const auto get_time = [](int /*edge_idx*/) -> int { return 1; };

    auto dist = std::vector<fun::Fraction<int>>(gra.number_of_nodes(), fun::Fraction<int>(0));
    auto r = fun::Fraction<int>(5);
    const auto c = min_cycle_ratio(gra, r, get_cost, get_time, dist);
    CHECK_FALSE(c.empty());
    CHECK_EQ(c.size(), 5);
    CHECK_EQ(r, fun::Fraction<int>(9, 5));
}

TEST_CASE("Test Cycle Ratio of Timing Graph") {
    const auto indices = std::array<uint32_t, 6>{0, 1, 2, 3, 4, 5};
    auto gra = create_test_case2(indices);

    const auto cost = std::array<int, 6>{7, -1, 3, 0, 2, 4};

    // ponytail: edge data is int (index into cost array)
    const auto get_cost = [&](int edge_idx) -> int { return cost[edge_idx]; };
    const auto get_time = [](int /*edge_idx*/) -> int { return 1; };

    auto dist = std::vector<fun::Fraction<int>>(gra.number_of_nodes(), fun::Fraction<int>(0));
    auto r = fun::Fraction<int>(7);
    const auto c = min_cycle_ratio(gra, r, get_cost, get_time, dist);
    CHECK_FALSE(c.empty());
    CHECK_EQ(r, fun::Fraction<int>(1, 1));
    CHECK_EQ(c.size(), 3);
}
