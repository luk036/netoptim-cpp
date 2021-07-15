// -*- coding: utf-8 -*-
#include <array>
#include <doctest/doctest.h>
#include <netoptim/neg_cycle.hpp> // import negCycleFinder
#include <vector>
// #include <xnetwork/classes/digraphs.hpp>
#include <xnetwork/generators/testcases.hpp>


/*!
 * @brief
 *
 * @tparam Graph
 * @param G
 * @return true
 * @return false
 */
template <typename Graph>
auto do_case(const Graph& G) -> bool
{
    const auto get_weight = [&](const auto& edge) -> int
    {
        const auto [u, v] = G.end_points(edge);
        return G[u][v];
    };

    auto dist = std::vector<int>(G.number_of_nodes(), 0);
    auto N = negCycleFinder<Graph>(G);
    const auto cycle = N.find_neg_cycle(dist, get_weight);
    return !cycle.empty();
}


/*!
 * @brief
 *
 */
TEST_CASE("Test Negative Cycle")
{
    auto weights = std::array<int, 5> {-5, 1, 1, 1, 1};
    auto G = create_test_case1(weights);
    const auto hasNeg = do_case(G);
    CHECK(hasNeg);
}

/*!
 * @brief
 *
 */
TEST_CASE("Test No Negative Cycle")
{
    auto weights = std::array<int, 5> {2, 1, 1, 1, 1};
    auto G = create_test_case1(weights);
    const auto hasNeg = do_case(G);
    CHECK(!hasNeg);
}

/*!
 * @brief
 *
 */
TEST_CASE("Test Timing Graph")
{
    auto weights = std::array<int, 6> {7, 0, 6, 4, 2, 5};
    auto G = create_test_case_timing(weights);
    const auto hasNeg = do_case(G);
    CHECK(!hasNeg);
}

/*!
 * @brief
 *
 */
TEST_CASE("Test Timing Graph (2)")
{
    auto weights = std::array<int, 6> {3, -4, 2, 0, -2, 1};
    auto G = create_test_case_timing(weights);
    const auto hasNeg = do_case(G);
    CHECK(hasNeg);
}
