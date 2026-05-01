// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <vector>
#include <xnetwork/generators/testcases.hpp>

#include "netoptim/neg_cycle.hpp"

/*!
 * @brief Stress test for negative cycle detection on large graph
 *
 * Tests the negative cycle finder on a large graph with 100 nodes
 * to ensure the algorithm works correctly under stress conditions.
 * The graph contains a single negative weight edge that creates
 * a negative cycle.
 */
TEST_CASE("Test Stress Negative Cycle") {
    auto weights = std::vector<int>();
    weights.resize(100, 1);
    weights[0] = -100;
    auto gra = create_test_case1(weights);
    auto dist = std::vector<int>(gra.number_of_nodes(), 0);
    auto ncf = NegCycleFinder(gra);

    const auto get_weight = [&](const auto& edge) -> int {
        const auto [utx, vtx] = edge;
        return gra[utx][vtx];
    };

    auto cycle = ncf.find_neg_cycle(dist, get_weight);
    CHECK_FALSE(cycle.empty());
}