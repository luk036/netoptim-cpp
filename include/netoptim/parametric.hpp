// -*- coding: utf-8 -*-
#pragma once

#include <digraphx/neg_cycle.hpp>  // import NegCycleFinder
#include <type_traits>
#include <utility>
#include <vector>

#include "netoptim_detail.hpp"  // import netoptim_detail helpers

/**
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
 *
 * Edge weights are accessed via a callable that receives the actual edge
 * data object from the graph's adjacency structure (the "get_weight"
 * method), rather than synthesized (u,v) node pairs. This eliminates the
 * duplication present in the previous node-pair-based approach and
 * matches the Python sibling implementation.
 */

/**
 * @brief Solve the maximum parametric problem
 *
 * This function implements an iterative algorithm to find the maximum
 * parameter value r for which the system of constraints is feasible.
 * The algorithm uses Howard's policy iteration for negative cycle
 * detection to identify violations and adjusts the parameter accordingly.
 *
 * The algorithm proceeds as follows:
 * 1. For the current parameter value, compute edge weights via the
 *    distance callable operating on the graph's native edge data
 * 2. Find negative cycles using Howard's method
 * 3. If no negative cycles exist, the current parameter is optimal
 * 4. Otherwise, compute a new parameter value from the violating cycle
 *    using the zero_cancel callable
 * 5. Repeat until convergence or max_iters reached
 *
 * The parametric problem is defined as:
 * @f[
 *     \max \; r \quad \text{s.t.} \quad d_v - d_u \ge w(u, v, r) \; \forall (u, v) \in E
 * @f]
 * where @f$w(u, v, r)@f$ is a monotone decreasing function of @f$r@f$.
 * The algorithm finds the maximum @f$r@f$ such that no negative cycles exist.
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
auto max_parametric(const Graph& gra, T& r_opt, Fn1&& distrance, Fn2&& zero_cancel, Mapping&& dist,
                    size_t max_iters = 1000) {
    return netoptim_detail::parametric_search(gra, r_opt, std::forward<Fn1>(distrance),
                                              std::forward<Fn2>(zero_cancel),
                                              std::forward<Mapping>(dist), max_iters);
}
