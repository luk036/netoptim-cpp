// -*- coding: utf-8 -*-
#pragma once

#include "neg_cycle.hpp" // import NegCycleFinder
#include <tuple>
#include <vector>

/*!
 * @brief maximum parametric problem
 *
 *    This function solves the following network parametric problem:
 *
 *        max  r
 *        s.t. dist[vtx] - dist[utx] \ge distrance(utx, vtx, r)
 *             \forall edge(utx, vtx) \in gra(V, E)
 *
 * @tparam Graph
 * @tparam T
 * @tparam Fn1
 * @tparam Fn2
 * @tparam Mapping
 * @param[in] gra directed graph
 * @param[in,out] r_opt parameter to be maximized, initially a large number
 * @param[in] distrance monotone decreasing function w.r.t. r
 * @param[in] zero_cancel
 * @param[in,out] dist
 * @return optimal r and the critical cycle
 */

template <typename Graph, typename T, typename Fn1, typename Fn2,
          typename Mapping>
auto max_parametric(const Graph &gra, T &r_opt, Fn1 &&distrance,
                    Fn2 &&zero_cancel, Mapping &&dist,
                    size_t max_iters = 1000) {
    using node_t = typename Graph::key_type;
    using edge_t = std::pair<node_t, node_t>;

    auto get_weight = [&](const edge_t &edge) -> T { // int???
        return distrance(r_opt, edge);
    };

    auto ncf = NegCycleFinder<Graph>(gra);
    auto c_opt = std::vector<edge_t>{}; // should initial outside

    for (auto niter = 0U; niter != max_iters; ++niter) {
        const auto &c_min = ncf.find_neg_cycle(std::forward<Mapping>(dist),
                                               std::move(get_weight));
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
