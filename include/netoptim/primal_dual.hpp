// -*- coding: utf-8 -*-
#pragma once

#include <algorithm>
#include <numeric>
#include <py2cpp/py2cpp.hpp>

/*!
 * @brief minimum weighted vertex cover problem
 *
 *    This function solves minimum vertex cover problem using primal-dual
 *    approximation algorithm:
 *
 * @tparam Graph
 * @tparam Container
 * @param[in] G
 * @param[in,out] cover
 * @param[in] weight
 * @return auto
 */
template <typename Graph, typename C1, typename C2>
auto min_vertex_cover_pd(const Graph& G, C1& cover, const C2& weight)
{
    using T = typename weight::value_type;

    [[maybe_unused]] auto total_dual_cost = T(0);
    auto total_primal_cost = T(0);
    auto gap = weight;
    for (auto&& e : G.edges())
    {
        auto [u, v] = e.end_points();
        if (cover[u] || cover[v])
        {
            continue;
        }
        if (gap[u] < gap[v])
        {
            std::swap(u, v);
        }
        cover[v] = true;
        total_dual_cost += gap[v];
        total_primal_cost += weight[v];
        gap[u] -= gap[v];
        gap[v] = T(0);
    }

    assert(total_dual_cost <= total_primal_cost);
    assert(total_primal_cost <= 2 * total_dual_cost);
    return total_primal_cost;
}

/*!
 * @brief minimum maximal independant set problem
 *
 *    This function solves minimum maximal independant set problem
 *    using primal-dual approximation algorithm:
 *
 * @tparam Graph
 * @tparam Container
 * @param[in] G
 * @param[in,out] cover
 * @param[in] weight
 * @return auto
 */
template <typename Graph, typename C1, typename C2>
auto min_maximal_independant_set_pd(
    const Graph& G, C1& indset, C1& dep, const C2& weight)
{
    auto cover = [&](const auto& u)
    {
        dep[u] = true;
        for (auto&& v : G[u])
        {
            dep[v] = true;
        }
    };

    auto gap = weight;
    [[maybe_unused]] total_dual_cost = T(0);
    total_primal_cost = T(0);
    for (auto&& u : G)
    {
        if (dep[u])
        {
            continue;
        }
        if (indset[u])
        { // pre-define independant
            cover(u);
            continue;
        }
        auto min_val = gap[u];
        auto min_vtx = u;
        for (auto&& v : G[u])
        {
            if (dep[v])
            {
                continue;
            }
            if (min_val > gap[v])
            {
                min_val = gap[v];
                min_vtx = v;
            }
        }
        cover(min_vtx);
        indset[min_vtx] = true;
        total_primal_cost += weight[min_vtx];
        total_dual_cost += min_val;
        if (min_vtx == u)
        {
            continue;
        }
        for (auto&& v : G[u])
        {
            gap[v] -= min_val;
        }
    }
    return total_primal_cost;
}
