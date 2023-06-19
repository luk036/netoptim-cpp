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
 * @param[in] gra
 * @param[in,out] cover
 * @param[in] weight
 * @return auto
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
 * @brief minimum maximal independant set problem
 *
 *    This function solves minimum maximal independant set problem
 *    using primal-dual approximation algorithm:
 *
 * @tparam Graph
 * @tparam Container
 * @param[in] gra
 * @param[in,out] cover
 * @param[in] weight
 * @return auto
 */
template <typename Graph, typename C1, typename C2>
auto min_maximal_independant_set_pd(const Graph &gra, C1 &indset, C1 &dep,
                                    const C2 &weight) {
  auto cover = [&](const auto &utx) {
    dep[utx] = true;
    for (auto &&vtx : gra[utx]) {
      dep[vtx] = true;
    }
  };

  auto gap = weight;
  [[maybe_unused]] total_dual_cost = T(0);
  total_primal_cost = T(0);
  for (auto &&utx : gra) {
    if (dep[utx]) {
      continue;
    }
    if (indset[utx]) { // pre-define independant
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
