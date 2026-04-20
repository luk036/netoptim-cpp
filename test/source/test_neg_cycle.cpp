// -*- coding: utf-8 -*-
#include <doctest/doctest.h>  // for ResultBuilder, TestCase

#include <array>                              // for array
#include <netoptim/neg_cycle.hpp>             // for NegCycleFinder
#include <unordered_map>                      // for unordered_map
#include <vector>                             // for vector
#include <xnetwork/classes/digraphs.hpp>      // for DiGraphS
#include <xnetwork/generators/testcases.hpp>  // for create_test_case1, crea...

/*!
 * @brief Test case runner for negative cycle detection
 *
 * This function creates a test case with the given graph and attempts to
 * find a negative cycle using the NegCycleFinder. It is used as a helper
 * to run multiple test scenarios.
 *
 * @tparam Graph Type of the directed graph
 * @param[in] gra The directed graph to test
 * @return true if a negative cycle was found, false otherwise
 */
template <typename Graph> auto do_case(const Graph& gra) -> bool {
    const auto get_weight = [&](const auto& edge) -> int {
        const auto [utx, vtx] = edge;
        return gra[utx][vtx];
    };

    auto dist = std::vector<int>(gra.number_of_nodes(), 0);
    auto ncf = NegCycleFinder<Graph>(gra);
    const auto cycle = ncf.find_neg_cycle(dist, get_weight);
    return !cycle.empty();
}

/*!
 * @brief Test negative cycle detection with test case 1
 *
 * Verifies that a negative cycle is correctly detected when all edge
 * weights are set such that a negative cycle exists in the graph.
 */
TEST_CASE("Test Negative Cycle") {
    auto weights = std::array<int, 5>{-5, 1, 1, 1, 1};
    auto gra = create_test_case1(weights);
    const auto hasNeg = do_case(gra);
    CHECK(hasNeg);
}

/*!
 * @brief Test that no negative cycle is reported when graph has none
 *
 * Verifies that the algorithm correctly reports no negative cycle when
 * all edge weights are non-negative.
 */
TEST_CASE("Test No Negative Cycle") {
    auto weights = std::array<int, 5>{2, 1, 1, 1, 1};
    auto gra = create_test_case1(weights);
    const auto hasNeg = do_case(gra);
    CHECK(!hasNeg);
}

/*!
 * @brief Test timing graph with all positive weights
 *
 * Verifies that no negative cycle is found in the timing test graph
 * when all weights are positive.
 */
TEST_CASE("Test Timing Graph") {
    auto weights = std::array<int, 6>{7, 0, 6, 4, 2, 5};
    auto gra = create_test_case_timing(weights);
    const auto hasNeg = do_case(gra);
    CHECK(!hasNeg);
}

/*!
 * @brief Test timing graph with negative cycle
 *
 * Verifies that a negative cycle is correctly detected in the timing
 * test graph when some edge weights create a negative cycle.
 */
TEST_CASE("Test Timing Graph (2)") {
    auto weights = std::array<int, 6>{3, -4, 2, 0, -2, 1};
    auto gra = create_test_case_timing(weights);
    const auto hasNeg = do_case(gra);
    CHECK(hasNeg);
}
