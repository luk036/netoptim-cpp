// -*- coding: utf-8 -*-
#include <doctest/doctest.h> // for ResultBuilder, TestCase

#include <cstdint>                // for uint32_t
#include <netoptim/neg_cycle.hpp> // for NegCycleFinder
#include <py2cpp/dict.hpp>        // for dict

/*!
 * @brief
 *
 */
TEST_CASE("Test Negative Cycle 2") {
    py::dict<uint32_t, py::dict<uint32_t, int>> gra{
        {0, {{1, 7}, {2, 5}}}, {1, {{0, 0}, {2, 3}}}, {2, {{1, 1}, {0, 2}}}};

    const auto get_weight = [&](const auto &edge) -> int {
        const auto [utx, vtx] = edge;
        return gra.at(utx).at(vtx);
    };
    auto dist = py::dict<uint32_t, int>{{0, 0}, {1, 0}, {2, 0}};
    NegCycleFinder ncf(gra);
    const auto cycle = ncf.find_neg_cycle(dist, get_weight);
    CHECK(cycle.empty());
}

#include <py2cpp/lict.hpp> // for Lict

/*!
 * @brief
 *
 */
TEST_CASE("Test Negative Cycle (Lict)") {
    py::Lict<py::dict<size_t, int>> gra{
        {{{1, 7}, {2, 5}}, {{0, 0}, {2, 3}}, {{1, 1}, {0, 2}}}};

    const auto get_weight = [&](const auto &edge) -> int {
        const auto [utx, vtx] = edge;
        return gra.at(utx).at(vtx);
    };
    auto dist = py::Lict<int>{std::vector<int>{0, 0, 0}};
    NegCycleFinder ncf(gra);
    const auto cycle = ncf.find_neg_cycle(dist, get_weight);
    CHECK(cycle.empty());
}
