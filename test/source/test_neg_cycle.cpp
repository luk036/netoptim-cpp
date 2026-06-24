// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <array>
#include <digraphx/neg_cycle.hpp>
#include <vector>
#include <xnetwork/classes/digraphs.hpp>
#include <xnetwork/generators/testcases.hpp>

// ponytail: get_weight operates on the graph's native edge data (int weights)
template <typename Graph> auto do_case(const Graph& gra) -> bool {
    auto dist = std::vector<int>(gra.number_of_nodes(), 0);
    auto ncf = NegCycleFinder<Graph>(gra);
    auto get_weight = [](int w) -> int { return w; };
    for ([[maybe_unused]] const auto& _ : ncf.howard(dist, get_weight)) {
        return true;
    }
    return false;
}

TEST_CASE("Test Negative Cycle") {
    auto weights = std::array<int, 5>{-5, 1, 1, 1, 1};
    auto gra = create_test_case1(weights);
    const auto hasNeg = do_case(gra);
    CHECK(hasNeg);
}

TEST_CASE("Test No Negative Cycle") {
    auto weights = std::array<int, 5>{2, 1, 1, 1, 1};
    auto gra = create_test_case1(weights);
    const auto hasNeg = do_case(gra);
    CHECK_FALSE(hasNeg);
}

TEST_CASE("Test Timing Graph") {
    auto weights = std::array<int, 6>{7, 0, 6, 4, 2, 5};
    auto gra = create_test_case_timing(weights);
    const auto hasNeg = do_case(gra);
    CHECK_FALSE(hasNeg);
}

TEST_CASE("Test Timing Graph (2)") {
    auto weights = std::array<int, 6>{3, -4, 2, 0, -2, 1};
    auto gra = create_test_case_timing(weights);
    const auto hasNeg = do_case(gra);
    CHECK(hasNeg);
}
