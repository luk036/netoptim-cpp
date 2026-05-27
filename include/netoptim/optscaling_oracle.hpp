// -*- coding: utf-8 -*-
#pragma once

#include <algorithm>  // for std::min
#include <valarray>

#include "network_oracle.hpp"

/*!
 * @file optscaling_oracle.hpp
 * @brief Oracle for Optimal Matrix Scaling Problem
 *
 * This module implements an oracle for solving the optimal matrix scaling
 * problem as formulated by Orlin and Rothblum (1985). The problem involves
 * finding diagonal scaling factors that minimize the ratio between the
 * maximum and minimum scaled matrix entries.
 *
 * The problem formulation:
 *     min     pi/phi
 *     s.t.    phi &le; utx[i] * |aij| * utx[j]^-1 &le; pi,
 *             &forall; aij != 0,
 *             pi, phi, utx, positive
 *
 * where utx are the scaling factors, pi is the maximum scaled entry,
 * and phi is the minimum scaled entry.
 *
 * @tparam Graph Type of the directed graph (must work with digraphx NegCycleFinder)
 * @tparam Mapping Type of vertex potential mapping
 * @tparam Fn Type of the cost function: (edge_data) -> std::pair<double, double>
 */
template <typename Graph, typename Mapping, typename Fn>  //
class OptScalingOracle {
    using Vec = std::valarray<double>;
    using Cut = std::pair<Vec, double>;

    /*!
     * @brief Evaluates the scaling ratio constraint for one edge
     *
     * Computes min(pi - a_ji, a_ij - psi) where x = (pi, psi) in log scale.
     * The edge data from the graph is passed through get_cost to obtain
     * the pair (a_ij, a_ji) of log-scale matrix entries.
     */
    class Ratio {
      private:
        const Graph& _gra;
        Fn _get_cost;

      public:
        Ratio(const Graph& gra, Fn get_cost) : _gra{gra}, _get_cost{std::move(get_cost)} {}

        explicit Ratio(const Ratio&) = default;

        /*!
         * @brief Evaluate the scaling constraint for an edge
         *
         * @tparam Edge edge data type from the graph adjacency
         * @param[in] edge edge data (passed to get_cost for cost extraction)
         * @param[in] x vector (pi, psi) in log scale
         * @return double constraint value (positive if feasible)
         */
        template <typename Edge>
        auto eval(const Edge& edge, const Vec& x) const -> double {
            const auto [aij, aji] = this->_get_cost(edge);
            return std::min(x[0] - aji, aij - x[1]);
        }

        /*!
         * @brief Compute the subgradient of the ratio constraint
         *
         * Returns [1.0, 0.0] if the upper bound (pi) constraint is active,
         * or [0.0, -1.0] if the lower bound (psi) constraint is active.
         *
         * @tparam Edge edge data type from the graph adjacency
         * @param[in] edge edge data (passed to get_cost for cost extraction)
         * @param[in] x vector (pi, psi) in log scale
         * @return Vec subgradient vector
         */
        template <typename Edge>
        auto grad(const Edge& edge, const Vec& x) const -> Vec {
            const auto [aij, aji] = this->_get_cost(edge);
            if (x[0] - aji < aij - x[1]) {
                return Vec{1.0, 0.0};
            }
            return Vec{0.0, -1.0};
        }
    };

    NetworkOracle<Graph, Mapping, Ratio> _network;

  public:
    /*!
     * @brief Construct a new optscaling oracle
     *
     * @param[in] gra graph representing matrix sparsity pattern
     * @param[in,out] utx vertex scaling factors (updated during algorithm)
     * @param[in] get_cost function to extract (a_ij, a_ji) from edge data
     */
    OptScalingOracle(const Graph& gra, Mapping& utx, Fn get_cost)
        : _network(gra, utx, Ratio{gra, get_cost}) {}

    explicit OptScalingOracle(const OptScalingOracle&) = default;

    /*!
     * @brief Assess optimality and return cutting plane if needed
     *
     * First checks feasibility via the network oracle. If feasible,
     * computes the objective gap and returns a central cut.
     *
     * @param[in] x current iterate (pi, psi) in log scale
     * @param[in,out] t best-so-far optimal value (updated on improvement)
     * @return std::tuple<Cut, bool> (cut, shrunk_flag)
     */
    auto assess_optim(const Vec& x, double& t) -> std::tuple<Cut, bool> {
        const auto cut = this->_network.assess_feas(x);
        if (cut) {
            return {*cut, false};
        }
        auto s = x[0] - x[1];
        auto fj = s - t;
        if (fj < 0) {
            t = s;
            return {{Vec{1., -1.}, 0.}, true};
        }
        return {{Vec{1., -1.}, fj}, false};
    }

    /*!
     * @brief Call operator for cutting_plane_optim()
     *
     * @param[in] x (pi, phi) in log scale
     * @param[in,out] t the best-so-far optimal value
     * @return std::tuple<Cut, bool>
     */
    auto operator()(const Vec& x, double& t) -> std::tuple<Cut, bool> { return assess_optim(x, t); }
};
