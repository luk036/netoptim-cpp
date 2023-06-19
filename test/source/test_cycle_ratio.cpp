// -*- coding: utf-8 -*-
#include <doctest/doctest.h> // for ResultBuilder, CHECK

#include <array>   // for array
#include <cstdint> // for uint32_t
#include <iostream>
#include <netoptim/min_cycle_ratio.hpp>      // for min_cycle_ratio
#include <py2cpp/dict.hpp>                   // for dict
#include <py2cpp/enumerate.hpp>              // for iterable_wrapper
#include <py2cpp/fractions.hpp>              // for Fraction, operator-
#include <unordered_map>                     // for operator!=
#include <vector>                            // for vector
#include <xnetwork/classes/digraphs.hpp>     // for DiGraphS
#include <xnetwork/generators/testcases.hpp> // for create_test_case1,

TEST_CASE("Test Cycle Ratio") {
  const auto indices = std::array<size_t, 5>{0, 1, 2, 3, 4};
  auto gra = create_test_case1(indices);

  const auto cost = std::array<int, 5>{5, 1, 1, 1, 1};

  const auto get_cost = [&](const auto &edge) -> int {
    const auto [utx, vtx] = edge;
    return cost[size_t(gra[utx][vtx])];
  };
  const auto get_time = [&](const auto & /*edge*/) -> int { return 1; };

  auto dist = std::vector<fun::Fraction<int>>(gra.number_of_nodes(),
                                              fun::Fraction<int>(0));
  auto r = fun::Fraction<int>(5);
  const auto c = min_cycle_ratio(gra, r, get_cost, get_time, dist);
  CHECK(!c.empty());
  CHECK(c.size() == 5);
  CHECK(r == fun::Fraction<int>(9, 5));
}

TEST_CASE("Test Cycle Ratio of Timing Graph") {
  // make sure no parallel edges!!!

  const auto indices = std::array<size_t, 6>{0, 1, 2, 3, 4, 5};
  auto gra = create_test_case2(indices);

  const auto cost = std::array<int, 6>{7, -1, 3, 0, 2, 4};

  const auto get_cost = [&](const auto &edge) -> int {
    const auto [utx, vtx] = edge;
    return cost[size_t(gra[utx][vtx])];
  };
  const auto get_time = [&](const auto & /*edge*/) -> int { return 1; };

  auto dist = std::vector<fun::Fraction<int>>(gra.number_of_nodes(),
                                              fun::Fraction<int>(0));
  auto r = fun::Fraction<int>(7);
  const auto c = min_cycle_ratio(gra, r, get_cost, get_time, dist);
  CHECK(!c.empty());
  CHECK(r == fun::Fraction<int>(1, 1));
  CHECK(c.size() == 3);
}
