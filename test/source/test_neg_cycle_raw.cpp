// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <cstdint>
#include <digraphx/neg_cycle.hpp>
#include <py2cpp/dict.hpp>

TEST_CASE("Test Negative Cycle 2") {
    py::dict<uint32_t, py::dict<uint32_t, int>> gra{
        {0, {{1, 7}, {2, 5}}}, {1, {{0, 0}, {2, 3}}}, {2, {{1, 1}, {0, 2}}}};

    // ponytail: edge data is int (the weight itself)
    auto dist = py::dict<uint32_t, int>{{0, 0}, {1, 0}, {2, 0}};
    auto ncf = NegCycleFinder(gra);
    auto get_weight = [](int w) -> int { return w; };
    // howard yields nothing for this graph (no negative cycle)
    for ([[maybe_unused]] const auto& _ : ncf.howard(dist, get_weight)) {
        CHECK(false);  // should not reach here
    }
}
