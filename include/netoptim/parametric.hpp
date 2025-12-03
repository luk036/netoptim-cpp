// -*- coding: utf-8 -*-
#pragma once

#include <tuple>
#include <vector>

#include "neg_cycle.hpp"  // import NegCycleFinder

/*!
 * @file parametric.hpp
 * @brief Maximum parametric problem solver for network optimization
 * 
 * This module implements a general algorithm for solving parametric network
 * optimization problems. The algorithm finds the maximum parameter value r
 * such that a system of inequalities holds, which is equivalent to finding
 * the most violated cycle in a weighted graph.
 * 
 * The problem formulation:
 *     max  r
 *     s.t. dist[vtx] - dist[utx] &ge; distance(utx, vtx, r)
 *          &forall; edge(utx, vtx) &isin; gra(V, E)
 * 
 * This is a fundamental building block for many network optimization
 * algorithms including minimum cycle ratio, minimum mean cycle, and
 * other parametric flow problems.
 */

/*!
 * @brief Solve the maximum parametric problem
 * 
 * This function implements an iterative algorithm to find the maximum
 * parameter value r for which the system of constraints is feasible.
 * The algorithm uses negative cycle detection to identify violations
 * and adjusts the parameter accordingly.
 * 
 * The algorithm proceeds as follows:
 * 1. For the current parameter value, compute edge weights
 * 2. Find negative cycles using these weights
 * 3. If no negative cycles exist, the current parameter is optimal
 * 4. Otherwise, compute a new parameter value from the violating cycle
 * 5. Update distances and repeat until convergence
 * 
 * @tparam Graph Type of the directed graph
 * @tparam T Numeric type for the parameter r
 * @tparam Fn1 Type of the distance function (parameter, edge) -> weight
 * @tparam Fn2 Type of the zero-canceling function (cycle) -> parameter
 * @tparam Mapping Type of distance mapping (vertex -> distance)
 * @param[in] gra directed graph containing the network structure
 * @param[in,out] r_opt parameter to be maximized, updated with optimal value
 * @param[in] distrance monotone decreasing function of parameter r
 * @param[in] zero_cancel function to compute new parameter from cycle
 * @param[in,out] dist distance mapping used in the algorithm
 * @param[in] max_iters maximum number of iterations (default: 1000)
 * @return auto the critical cycle that determines the optimal parameter
 */

template <typename Graph, typename T, typename Fn1, typename Fn2, typename Mapping>
auto max_parametric(const Graph &gra, T &r_opt, Fn1 &&distrance, Fn2 &&zero_cancel, Mapping &&dist,
                    size_t max_iters = 1000) {
    using node_t = typename Graph::key_type;
    using edge_t = std::pair<node_t, node_t>;

    auto get_weight = [&](const edge_t &edge) -> T {  // int???
        return distrance(r_opt, edge);
    };

    auto ncf = NegCycleFinder<Graph>(gra);
    auto c_opt = std::vector<edge_t>{};  // should initial outside

    for (auto niter = 0U; niter != max_iters; ++niter) {
        const auto &c_min = ncf.find_neg_cycle(std::forward<Mapping>(dist), std::move(get_weight));
        if (c_min.empty()) {
            break;
        }

        const auto &r_min = zero_cancel(c_min);
        if (r_min >= r_opt) {
            break;
        }

        c_opt = c_min;
        r_opt = r_min;

        // update ???
        for (auto &&edge : c_opt) {
            const auto [utx, vtx] = edge;
            dist[utx] = dist[vtx] - get_weight(edge);
        }
    }

    return c_opt;
}
