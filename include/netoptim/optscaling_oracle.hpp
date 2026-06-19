// -*- coding: utf-8 -*-
#pragma once

#include <cassert>
// #include <xtensor/xarray.hpp>

#include <valarray>

#include "network_oracle.hpp"

/**
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
 */

/**
 * @brief Oracle for Optimal Matrix Scaling
 *
 * This class implements a separation oracle for the optimal matrix scaling
 * problem. It uses a network oracle internally to handle the feasibility
 * constraints and provides cutting planes for optimization.
 *
 * The oracle maintains:
 * - A reference to the underlying graph structure
 * - A cost function for matrix entries
 * - A Ratio helper class for constraint evaluation
 * - Network oracle for feasibility checking
 *
 * @tparam Graph Type of the directed graph representing matrix sparsity
 * @tparam Mapping Type of vertex potential mapping (scaling factors)
 * @tparam Fn Type of the cost function (edge -> matrix entry)
 */
template <typename Graph, typename Mapping, typename Fn>  //
    requires HasKeyType<Graph>
class OptScalingOracle {
    using Vec = std::valarray<double>;
    using node_t = typename Graph::key_type;
    using edge_t = std::pair<node_t, node_t>;
    using Cut = std::pair<Vec, double>;

    /** @brief Helper class for evaluating scaling ratio constraints
     * @details Provides constraint evaluation and gradient computation
     * functions needed by the network oracle for the matrix scaling problem. */
    class Ratio {
      private:
        const Graph& _gra;
        Fn _get_cost;

      public:
        /** @brief Construct a new Ratio object
         * @param[in] gra graph representing matrix sparsity pattern
         * @param[in] get_cost function to extract matrix entries */
        Ratio(const Graph& gra, Fn get_cost) : _gra{gra}, _get_cost{std::move(get_cost)} {}

        /** @brief Copy constructor */
        explicit Ratio(const Ratio&) = default;

        /** @brief Evaluate the constraint function for an edge
         * @param[in] edge the matrix entry (edge) to evaluate
         * @param[in] x vector containing (pi, phi) in log scale
         * @return constraint value (negative if violated) */
        auto eval(const edge_t& edge, const Vec& x) const -> double {
            const auto cost = this->_get_cost(edge);
            const auto [utx, vtx] = edge;
            assert(utx != vtx);
            return (utx < vtx) ? x[0] - cost.first : cost.second - x[1];
        }

        /** @brief Compute the gradient of the constraint function
         * @param[in] edge the matrix entry (edge) for gradient computation
         * @return gradient vector [d/d(pi), d/d(phi)] */
        auto grad(const edge_t& edge, const Vec& /* x */) const -> Vec {
            // const auto [utx, vtx] = this->_gra.end_points(edge);
            const auto [utx, vtx] = edge;
            assert(utx != vtx);
            return (utx < vtx) ? Vec{1., 0.} : Vec{0., -1.};
        }
    };

    NetworkOracle<Graph, Mapping, Ratio> _network;

  public:
    /** @brief Construct a new optscaling oracle object
     * @param[in] gra The graph representing matrix sparsity
     * @param[in,out] utx Vertex potential mapping (scaling factors)
     * @param[in] get_cost Function to extract matrix entries */
    OptScalingOracle(const Graph& gra, Mapping& utx, Fn get_cost)
        : _network(gra, utx, Ratio{gra, get_cost}) {}

    /** @brief Copy constructor */
    explicit OptScalingOracle(const OptScalingOracle&) = default;
    OptScalingOracle(OptScalingOracle&&) = default;
    OptScalingOracle& operator=(const OptScalingOracle&) = default;
    OptScalingOracle& operator=(OptScalingOracle&&) = default;
    ~OptScalingOracle() = default;

    /** @brief Assess optimality at point x (cutting plane interface)
     * @param[in] x (pi, phi) in log scale
     * @param[in,out] t the best-so-far optimal value
     * @return tuple of (cut, whether gamma was updated)
     * @see cutting_plane_optim */
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

    /** @brief Function call operator for cutting_plane_optim()
     * @param[in] x (pi, phi) in log scale
     * @param[in,out] t the best-so-far optimal value
     * @return tuple of (cut, whether gamma was updated)
     * @see cutting_plane_optim */
    auto operator()(const Vec& x, double& t) -> std::tuple<Cut, bool> { return assess_optim(x, t); }
};
