// -*- coding: utf-8 -*-
#pragma once

#include <cstddef>
#include <ellalgo/cutting_plane.hpp>  // for cutting_plane_feas / cutting_plane_optim
#include <ellalgo/ell_config.hpp>     // for Options
#include <tuple>

/**
 * @file solve.hpp
 * @brief Convenience facades for driving netoptim oracles with the ellipsoid
 *        cutting-plane method.
 *
 * Mirrors the Python sibling's `solve_network_feas` / `solve_opt_scaling`
 * facades: both forward an `Options` object (defaulting to a sensible
 * tolerance) to the ellalgo drivers, and the feasibility facade uses the
 * feasibility driver (`cutting_plane_feas`).
 */

/** @brief Default convergence tolerance used by the solver facades.
 *
 * Looser than ellalgo's built-in 1e-20, which is far below machine precision
 * relative to typical objective magnitudes and only inflates the iteration
 * count. Pass an explicit Options to override. */
inline constexpr double kDefaultTolerance = 1e-8;

/** @brief Build an Options using the library's default tolerance. */
inline auto default_options() -> Options { return Options{2000, kDefaultTolerance}; }

/**
 * @brief Find a feasible point for a network feasibility problem.
 *
 * @tparam O feasibility-oracle type (e.g. NetworkOracle)
 * @tparam S search-space type (e.g. Ell)
 * @param[in,out] omega   feasibility oracle
 * @param[in,out] space   search space; its center is the starting iterate
 * @param[in]     options algorithm control parameters
 * @return tuple (x_best, num_iters) as returned by cutting_plane_feas
 */
template <typename O, typename S>
inline auto solve_network_feas(O& omega, S& space, const Options& options = default_options())
    -> std::tuple<CuttingPlaneArrayType<S>, size_t> {
    return cutting_plane_feas(omega, space, options);
}

/**
 * @brief Maximize the scaling ratio for an optimal matrix scaling problem.
 *
 * @tparam O optimality-oracle type (e.g. OptScalingOracle)
 * @tparam S search-space type (e.g. Ell)
 * @tparam N numeric best-so-far type
 * @param[in,out] omega   optimality oracle
 * @param[in,out] space   search space; its center is the starting iterate
 * @param[in,out] gamma   best-so-far optimal value, updated in place
 * @param[in]     options algorithm control parameters
 * @return tuple (x_best, num_iters) as returned by cutting_plane_optim
 */
template <typename O, typename S, typename N>
inline auto solve_opt_scaling(O& omega, S& space, N& gamma,
                              const Options& options = default_options())
    -> std::tuple<CuttingPlaneArrayType<S>, size_t> {
    return cutting_plane_optim(omega, space, gamma, options);
}
