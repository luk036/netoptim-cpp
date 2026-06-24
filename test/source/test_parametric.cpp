// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <array>
#include <netoptim/parametric.hpp>
#include <py2cpp/dict.hpp>
#include <vector>
#include <xnetwork/classes/digraphs.hpp>
#include <xnetwork/generators/testcases.hpp>

TEST_CASE("Test max_parametric with positive weights (no neg cycles)") {
    const auto indices = std::array<uint32_t, 5>{0, 1, 2, 3, 4};
    auto gra = create_test_case1(indices);

    const auto cost = std::array<int, 5>{2, 1, 1, 1, 1};
    const auto get_cost = [&](int edge_idx) -> int { return cost[edge_idx]; };

    auto dist = std::vector<int>(gra.number_of_nodes(), 0);
    auto r = 0;

    auto calc_weight = [&](const int& r, int edge_idx) -> int { return get_cost(edge_idx) - r * 1; };
    auto calc_ratio = [&](const auto& /*C*/) -> int { return 0; };

    const auto c = max_parametric(gra, r, std::move(calc_weight), std::move(calc_ratio), dist);
    CHECK(c.empty());
    CHECK_EQ(r, 0);
}

TEST_CASE("Test max_parametric terminates at max_iters") {
    py::dict<uint32_t, py::dict<uint32_t, int>> gra{
        {0, {{1, -1}, {2, -1}}},
        {1, {{2, -1}}},
        {2, {{0, -1}}},
    };

    auto dist = py::dict<uint32_t, int>{{0, 0}, {1, 0}, {2, 0}};
    auto r = 0;

    // ponytail: edge data is int (the weight itself)
    auto calc_weight = [&](const int& /*r*/, int /*edge*/) -> int { return -1; };
    auto calc_ratio = [&](const auto& /*C*/) -> int { return r - 1; };

    const auto c = max_parametric(gra, r, std::move(calc_weight), std::move(calc_ratio), dist, 2);
    CHECK_LT(r, 0);
}
