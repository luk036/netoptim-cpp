// -*- coding: utf-8 -*-
#pragma once

#include <digraphx/neg_cycle.hpp>  // import NegCycleFinder
#include <type_traits>
#include <vector>

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
 * Edge weights are accessed via a callable that receives the actual edge
 * data object from the graph's adjacency structure (the "get_weight"
 * method), rather than synthesized (u,v) node pairs.
 */

/**
 * @brief Solve the maximum parametric problem
 *
 * Uses Howard's policy iteration for negative cycle detection. The distance
 * and zero_cancel callables operate on the graph's native edge data type.
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
    // ponytail: deduce Edge type using the same helpers as NegCycleFinder
    using Elem = decltype(*std::declval<const Graph&>().begin());
    using Nbrs = std::remove_cv_t<std::remove_reference_t<
        decltype(_get_val(std::declval<Elem>(), std::declval<const Graph&>()))>>;
    using NbrElem = decltype(*std::declval<const Nbrs&>().begin());
    using Edge = std::remove_cv_t<std::remove_reference_t<
        decltype(_get_val(std::declval<NbrElem>(), std::declval<const Nbrs&>()))>>;
    using Cycle = std::vector<Edge>;

    auto get_weight = [&distrance, &r_opt](const Edge& edge) -> T {
        return static_cast<T>(distrance(r_opt, edge));
    };

    auto ncf = NegCycleFinder<Graph>(gra);
    auto r_min = r_opt;
    auto c_min = Cycle{};
    auto c_opt = Cycle{};

    for (auto niter = 0U; niter != max_iters; ++niter) {
        for (auto&& ci : ncf.howard(dist, get_weight)) {
            auto ri = static_cast<T>(zero_cancel(ci));
            if (r_min > ri) {
                r_min = ri;
                c_min = std::move(ci);
            }
        }
        if (r_min >= r_opt) break;
        c_opt = std::move(c_min);
        r_opt = r_min;
    }
    return c_opt;
}
