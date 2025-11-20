// -*- coding: utf-8 -*-
#include <doctest/doctest.h>
#include <vector>
#include <xnetwork/generators/testcases.hpp>
#include "netoptim/neg_cycle.hpp"

TEST_CASE("Test Stress Negative Cycle") {
    auto weights = std::vector<int>();
    weights.resize(100, 1);
    weights[0] = -100;
    auto gra = create_test_case1(weights);
    auto dist = std::vector<int>(gra.number_of_nodes(), 0);
    auto ncf = NegCycleFinder(gra);

    const auto get_weight = [&](const auto &edge) -> int {
        const auto [utx, vtx] = edge;
        return gra[utx][vtx];
    };

    auto cycle = ncf.find_neg_cycle(dist, get_weight);
    CHECK(!cycle.empty());
}