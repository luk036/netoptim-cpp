// -*- coding: utf-8 -*-
#pragma once

#include <algorithm>
#include <numeric>
#include <py2cpp/py2cpp.hpp>

#include "parametric.hpp"  // import max_parametric

/*!
 * @file min_cycle_ratio.hpp
 * @brief Minimum cost-to-time cycle ratio problem solver
 * 
 * This module implements an algorithm to find the minimum cycle ratio in a
 * weighted directed graph where each edge has both a cost and a time value.
 * The cycle ratio is defined as the total cost divided by the total time.
 * 
 * The problem formulation:
 *     max  r
 *     s.t. dist[vtx] - dist[utx] &ge; cost(utx, vtx) - r * time(utx, vtx)
 *          &forall; edge(utx, vtx) &isin; gra(V, E)
 * 
 * This is equivalent to finding the cycle with minimum cost/time ratio.
 * The algorithm uses a parametric search approach combined with negative
 * cycle detection.
 */

/*!
 * @brief Solve the minimum cost-to-time cycle ratio problem
 * 
 * This function finds the cycle in a graph that minimizes the ratio of
 * total cost to total time. It uses a parametric search algorithm that
 * iteratively adjusts the ratio parameter and searches for negative cycles.
 * 
 * The algorithm works by:
 * 1. Converting the ratio problem to a parametric weight problem
 * 2. Using negative cycle detection to find violating cycles
 * 3. Adjusting the ratio based on the found cycles
 * 4. Repeating until convergence
 * 
 * @tparam Graph Type of the graph, must provide key_type and edge iteration
 * @tparam T Numeric type for ratio values (e.g., double, float)
 * @tparam Fn1 Type of cost function (edge -> cost)
 * @tparam Fn2 Type of time function (edge -> time)
 * @tparam Mapping Type of distance mapping (vertex -> distance)
 * @param[in] gra The input graph
 * @param[in,out] r0 Initial ratio value, updated with optimal result
 * @param[in] get_cost Function to extract cost from edges
 * @param[in] get_time Function to extract time from edges
 * @param[in,out] dist Distance mapping used in the algorithm
 * @param[in] max_iters Maximum number of iterations (default: 1000)
 * @return auto A cycle (vector of edges) with the minimum ratio
 */
template <typename Graph, typename T, typename Fn1, typename Fn2, typename Mapping>
auto min_cycle_ratio(const Graph &gra, T &r0, Fn1 &&get_cost, Fn2 &&get_time, Mapping &&dist,
                     size_t max_iters = 1000) {
    using node_t = typename Graph::key_type;
    using edge_t = std::pair<node_t, node_t>;
    using cost_T = decltype(get_cost(std::declval<edge_t>()));
    using time_T = decltype(get_time(std::declval<edge_t>()));

    auto calc_ratio = [&](const auto &C) -> T {
        auto total_cost = cost_T(0);
        auto total_time = time_T(0);
        for (auto &&edge : C) {
            total_cost += get_cost(edge);
            total_time += get_time(edge);
        }
        return T(total_cost) / total_time;
    };

    auto calc_weight
        = [&](const T &r, const edge_t &edge) -> T { return get_cost(edge) - r * get_time(edge); };

    return max_parametric(gra, r0, std::move(calc_weight), std::move(calc_ratio),
                          std::forward<Mapping>(dist), max_iters);
}
