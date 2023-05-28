// -*- coding: utf-8 -*-
#pragma once

#include <algorithm>
#include <numeric>
#include <py2cpp/py2cpp.hpp>

#include "parametric.hpp" // import max_parametric

/*!
 * @brief minimum cost-to-time cycle ratio problem
 *
 *    This function solves the following network parametric problem:
 *
 *        max  r
 *        s.t. dist[v] - dist[u] \ge cost(u, v) - r * time(u, v)
 *             \forall e(u, v) \in gra(V, E)
 *
 * @tparam Graph
 * @tparam Fn1
 * @tparam Fn2
 * @tparam Container
 * @param[in] gra
 * @param[in,out] r0
 * @param[in] get_cost
 * @param[in] get_time
 * @param[in,out] dist
 * @return auto
 */
template <typename Graph, typename T, typename Fn1, typename Fn2,
          typename Container>
auto min_cycle_ratio(const Graph &gra, T &r0, Fn1 &&get_cost, Fn2 &&get_time,
                     Container &&dist, size_t max_iters = 1000) {
  using node_t = typename Graph::node_t;
  using Edge = std::pair<node_t, node_t>;
  using cost_T = decltype(get_cost(std::declval<Edge>()));
  using time_T = decltype(get_time(std::declval<Edge>()));

  auto calc_ratio = [&](const auto &C) -> T {
    auto total_cost = cost_T(0);
    auto total_time = time_T(0);
    for (auto &&e : C) {
      total_cost += get_cost(e);
      total_time += get_time(e);
    }
    return T(total_cost) / total_time;
  };

  auto calc_weight = [&](const T &r, const Edge &e) -> T {
    return get_cost(e) - r * get_time(e);
  };

  return max_parametric(gra, r0, std::move(calc_weight), std::move(calc_ratio),
                        std::forward<Container>(dist), max_iters);
}
