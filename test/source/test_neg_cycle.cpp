// -*- coding: utf-8 -*-
#include <doctest/doctest.h>  // for ResultBuilder, TestCase

#include <array>                              // for array
#include <netoptim/neg_cycle.hpp>             // for NegCycleFinder
#include <unordered_map>                      // for unordered_map
#include <vector>                             // for vector
#include <xnetwork/classes/digraphs.hpp>      // for DiGraphS
#include <xnetwork/generators/testcases.hpp>  // for create_test_case1, crea...

/*!
 * @brief
 *
 * @tparam Graph
 * @param[in] gra
 * @return true
 * @return false
 */
template <typename Graph> auto do_case(const Graph &gra) -> bool {
    const auto get_weight = [&](const auto &edge) -> int {
        const auto [utx, vtx] = edge;
        return gra[utx][vtx];
    };

    auto dist = std::vector<int>(gra.number_of_nodes(), 0);
    auto ncf = NegCycleFinder<Graph>(gra);
    const auto cycle = ncf.find_neg_cycle(dist, get_weight);
    return !cycle.empty();
}

/*!
 * @brief
 *
 */
TEST_CASE("Test Negative Cycle") {
    auto weights = std::array<int, 5>{-5, 1, 1, 1, 1};
    auto gra = create_test_case1(weights);
    const auto hasNeg = do_case(gra);
    CHECK(hasNeg);
}

/*!
 * @brief
 *
 */
TEST_CASE("Test No Negative Cycle") {
    auto weights = std::array<int, 5>{2, 1, 1, 1, 1};
    auto gra = create_test_case1(weights);
    const auto hasNeg = do_case(gra);
    CHECK(!hasNeg);
}

/*!
 * @brief
 *
 */
TEST_CASE("Test Timing Graph") {
    auto weights = std::array<int, 6>{7, 0, 6, 4, 2, 5};
    auto gra = create_test_case_timing(weights);
    const auto hasNeg = do_case(gra);
    CHECK(!hasNeg);
}

/*!
 * @brief
 *
 */
TEST_CASE("Test Timing Graph (2)") {
    auto weights = std::array<int, 6>{3, -4, 2, 0, -2, 1};
    auto gra = create_test_case_timing(weights);
    const auto hasNeg = do_case(gra);
    CHECK(hasNeg);
}
