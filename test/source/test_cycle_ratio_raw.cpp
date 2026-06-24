// -*- coding: utf-8 -*-
#include <doctest/doctest.h>  // for ResultBuilder, CHECK

#include <cstdint>  // for uint32_t
#include <iostream>
#include <netoptim/min_cycle_ratio.hpp>  // for min_cycle_ratio
#include <py2cpp/dict.hpp>               // for dict
#include <py2cpp/enumerate.hpp>          // for iterable_wrapper
#include <py2cpp/fractions.hpp>          // for Fraction, operator-
#include <vector>                        // for vector

TEST_CASE("Test Cycle Ratio (raw, dict)") {
    py::dict<uint32_t, py::dict<uint32_t, int>> gra{
        {0, {{1, 5}, {2, 1}}}, {1, {{0, 1}, {2, 1}}}, {2, {{1, 1}, {0, 1}}}};

    // ponytail: edge data is int (the weight itself)
    const auto get_cost = [](int w) -> int { return w; };

    const auto get_time = [](int /*edge_idx*/) -> int { return 1; };

    auto dist = std::vector<fun::Fraction<int>>(gra.size(), fun::Fraction<int>(0));
    auto r = fun::Fraction<int>(5);
    const auto c = min_cycle_ratio(gra, r, get_cost, get_time, dist);
    CHECK_FALSE(c.empty());
    CHECK_EQ(r, fun::Fraction<int>(1, 1));
}
