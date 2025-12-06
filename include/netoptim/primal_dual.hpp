// -*- coding: utf-8 -*-
#pragma once

#include <algorithm>
#include <numeric>
#include <py2cpp/py2cpp.hpp>

/*!
 * @file primal_dual.hpp
 * @brief Primal-dual approximation algorithms for graph problems
 *
 * This module implements primal-dual approximation algorithms for two
 * fundamental graph optimization problems:
 * 1. Minimum weighted vertex cover
 * 2. Minimum maximal independent set
 *
 * Both algorithms use the primal-dual paradigm which provides a
 * 2-approximation guarantee for the vertex cover problem and good
 * approximation ratios for the independent set problem.
 */

/*!
 * @brief Minimum weighted vertex cover using primal-dual algorithm
 *
 * This function implements a 2-approximation algorithm for the minimum
 * weighted vertex cover problem. The algorithm maintains dual variables
 * (gap values) and greedily selects vertices to cover edges.
 *
 * The algorithm guarantees:
 * - Primal cost &le; 2 &times; Dual cost
 * - 2-approximation ratio for the optimal solution
 *
 * @tparam Graph Type of the graph, must provide edges() and edge iteration
 * @tparam C1 Type of cover mapping (vertex -> bool)
 * @tparam C2 Type of weight mapping (vertex -> weight)
 * @param[in] gra input graph
 * @param[in,out] cover vertex cover mapping (updated with solution)
 * @param[in] weight vertex weight mapping
 * @return auto total cost of the vertex cover
 */
template <typename Graph, typename C1, typename C2>
auto min_vertex_cover_pd(const Graph &gra, C1 &cover, const C2 &weight) {
    using T = typename weight::value_type;

    [[maybe_unused]] auto total_dual_cost = T(0);
    auto total_primal_cost = T(0);
    auto gap = weight;
    for (auto &&edge : gra.edges()) {
        auto [utx, vtx] = edge.end_points();
        if (cover[utx] || cover[vtx]) {
            continue;
        }
        if (gap[utx] < gap[vtx]) {
            std::swap(utx, vtx);
        }
        cover[vtx] = true;
        total_dual_cost += gap[vtx];
        total_primal_cost += weight[vtx];
        gap[utx] -= gap[vtx];
        gap[vtx] = T(0);
    }

    assert(total_dual_cost <= total_primal_cost);
    assert(total_primal_cost <= 2 * total_dual_cost);
    return total_primal_cost;
}

/*!
 * @brief Minimum maximal independent set using primal-dual algorithm
 *
 * This function implements a primal-dual approximation algorithm for the
 * minimum maximal independent set problem. The algorithm builds an
 * independent set while maintaining maximal property by covering dependent
 * vertices.
 *
 * The algorithm works by:
 * 1. Maintaining gap values (dual variables)
 * 2. Selecting vertices with minimum gap values
 * 3. Marking selected vertices as independent
 * 4. Marking neighbors as dependent
 * 5. Updating gap values for remaining vertices
 *
 * @tparam Graph Type of the graph, must provide vertex iteration
 * @tparam C1 Type of independent/dependent set mapping (vertex -> bool)
 * @tparam C2 Type of weight mapping (vertex -> weight)
 * @param[in] gra input graph
 * @param[in,out] indset independent set mapping (updated with solution)
 * @param[in,out] dep dependent set mapping (updated during algorithm)
 * @param[in] weight vertex weight mapping
 * @return auto total cost of the independent set
 */
template <typename Graph, typename C1, typename C2>
auto min_maximal_independant_set_pd(const Graph &gra, C1 &indset, C1 &dep, const C2 &weight) {
    using T = typename C2::value_type;

    auto cover = [&](const auto &utx) {
        dep[utx] = true;
        for (auto &&vtx : gra[utx]) {
            dep[vtx] = true;
        }
    };

    auto gap = weight;
    [[maybe_unused]] auto total_dual_cost = T(0);
    auto total_primal_cost = T(0);
    for (auto &&utx : gra) {
        if (dep[utx]) {
            continue;
        }
        if (indset[utx]) {  // pre-define independant
            cover(utx);
            continue;
        }
        auto min_val = gap[utx];
        auto min_vtx = utx;
        for (auto &&vtx : gra[utx]) {
            if (dep[vtx]) {
                continue;
            }
            if (min_val > gap[vtx]) {
                min_val = gap[vtx];
                min_vtx = vtx;
            }
        }
        cover(min_vtx);
        indset[min_vtx] = true;
        total_primal_cost += weight[min_vtx];
        total_dual_cost += min_val;
        if (min_vtx == utx) {
            continue;
        }
        for (auto &&vtx : gra[utx]) {
            gap[vtx] -= min_val;
        }
    }
    return total_primal_cost;
}
