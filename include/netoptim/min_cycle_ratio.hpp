// -*- coding: utf-8 -*-
#pragma once

#include <algorithm>
#include <py2cpp/py2cpp.hpp>

#include "parametric.hpp"  // import max_parametric

/**
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
 * cycle detection via Howard's method. Edge weights are accessed using
 * the graph's native edge data type (the "get_weight" method).
 */

/**
 * @brief Solve the minimum cost-to-time cycle ratio problem
 *
 * This function finds the cycle in a graph that minimizes the ratio of
 * total cost to total time. It uses a parametric search algorithm that
 * iteratively adjusts the ratio parameter and searches for negative cycles
 * using Howard's method.
 *
 * The algorithm works by:
 * 1. Converting the ratio problem to a parametric weight problem
 * 2. Using Howard's negative cycle detection to find violating cycles
 * 3. Adjusting the ratio based on the found cycles
 * 4. Repeating until convergence
 *
 * Given a directed graph @f$G = (V, E)@f$ with cost and time on each edge,
 * the minimum cycle ratio problem finds:
 * @f[
 *     r^* = \min_{C \in \text{cycles}(G)} \frac{\sum_{e \in C} \text{cost}(e)}{\sum_{e \in C}
 * \text{time}(e)}
 * @f]
 * The algorithm uses parametric weights @f$w_r(e) = \text{cost}(e) - r \cdot \text{time}(e)@f$
 * and searches for the smallest @f$r@f$ such that no negative cycles exist.
 *
 * @dot
 *   digraph mcr {
 *     rankdir=TB; bgcolor="transparent";
 *     node [shape=box, style=filled, fillcolor="#d4e6f1"];
 *     init [label="Initialize r0,\ndist, max_iters", fillcolor="#a9cce3"];
 *     weight [label="Compute\nw_r(e) = cost(e)\n- r * time(e)"];
 *     neg_cycle [label="Find negative\ncycle via\nHoward's method", shape=diamond,
 * fillcolor="#f9e79f"]; ratio [label="Compute\ncycle ratio\ncost / time"]; update [label="Update\nr
 * = min ratio"]; done [label="Return\noptimal cycle", fillcolor="#7fb3d8"]; init -> weight; weight
 * -> neg_cycle; neg_cycle -> ratio [label="found", color="#27ae60"]; neg_cycle -> done [label="none
 * found", color="#e74c3c"]; ratio -> update; update -> weight [style=dashed, label="iterate",
 * color="#888"];
 *   }
 * @enddot
 *
 * @tparam Graph Type of the graph, must provide key_type and edge iteration
 * @tparam T Numeric type for ratio values (e.g., double, Fraction)
 * @tparam Fn1 Type of cost function (edge_data -> cost)
 * @tparam Fn2 Type of time function (edge_data -> time)
 * @tparam Mapping Type of distance mapping (vertex -> distance)
 * @param[in] gra The input graph
 * @param[in,out] r0 Initial ratio value, updated with optimal result
 * @param[in] get_cost Function to extract cost from edge data
 * @param[in] get_time Function to extract time from edge data
 * @param[in,out] dist Distance mapping used in the algorithm
 * @param[in] max_iters Maximum number of iterations (default: 1000)
 * @return auto A cycle (vector of native edge data) with the minimum ratio
 */
template <typename Graph, typename T, typename Fn1, typename Fn2, typename Mapping>
auto min_cycle_ratio(const Graph& gra, T& r0, Fn1&& get_cost, Fn2&& get_time, Mapping&& dist,
                     size_t max_iters = 1000) {
    // ponytail: deduce Edge type using the same helpers as NegCycleFinder
    using Elem = decltype(*std::declval<const Graph&>().begin());
    using Nbrs = std::remove_cv_t<std::remove_reference_t<decltype(_get_val(
        std::declval<Elem>(), std::declval<const Graph&>()))>>;
    using NbrElem = decltype(*std::declval<const Nbrs&>().begin());
    using Edge = std::remove_cv_t<std::remove_reference_t<decltype(_get_val(
        std::declval<NbrElem>(), std::declval<const Nbrs&>()))>>;
    using edge_t = Edge;

    auto calc_ratio = [&](const auto& C) -> T {
        using cost_T = decltype(get_cost(std::declval<edge_t>()));
        using time_T = decltype(get_time(std::declval<edge_t>()));
        auto total_cost = cost_T(0);
        auto total_time = time_T(0);
        for (auto&& edge : C) {
            total_cost += get_cost(edge);
            total_time += get_time(edge);
        }
        return T(total_cost) / total_time;
    };

    auto calc_weight = [&](const T& r, const edge_t& edge) -> T {
        return get_cost(edge) - r * T(get_time(edge));
    };

    return max_parametric(gra, r0, std::move(calc_weight), std::move(calc_ratio),
                          std::forward<Mapping>(dist), max_iters);
}
