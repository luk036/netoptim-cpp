// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <digraphx/neg_cycle.hpp>
#include <vector>
#include <xnetwork/generators/testcases.hpp>

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

    // ponytail: edge data is int (the weight)
    auto get_weight = [](int w) -> int { return w; };

    auto found = false;
    for ([[maybe_unused]] const auto& _ : ncf.howard(dist, get_weight)) {
        found = true;
    }
    CHECK(found);
}