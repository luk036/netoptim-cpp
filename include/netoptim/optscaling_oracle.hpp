// -*- coding: utf-8 -*-
#pragma once

#include <cassert>
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
 * @f[
 *     \min \frac{\pi}{\phi} \quad \text{s.t.} \quad
 *     \phi \le u_i |a_{ij}| u_j^{-1} \le \pi \; \forall a_{ij} \neq 0,
 * \quad \pi, \phi, u_i > 0
 * @f]
 *
 * where @f$ u_i @f$ are the scaling factors, @f$ \pi @f$ is the maximum scaled entry,
 * and @f$ \phi @f$ is the minimum scaled entry.
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
 * @tparam Fn Type of the cost function (edge -> matrix entry pair)
 */
template <typename Graph, typename Mapping, typename Fn>  //
    requires HasKeyType<Graph>
class OptScalingOracle {
    using Vec = std::valarray<double>;
    using node_t = typename Graph::key_type;
    using Cut = std::pair<Vec, double>;

    /** @brief Helper class for evaluating scaling ratio constraints
     * @details Provides constraint evaluation and gradient computation
     * functions needed by the network oracle for the matrix scaling problem.
     * Operates on the graph's native edge data (e.g., pair<double, double>
     * representing (a_ij, a_ji) matrix entries). */
    class Ratio {
      private:
        const Graph& _gra;
        Fn _get_cost;

      public:
        /** @brief Construct a new Ratio object
         * @param[in] gra graph representing matrix sparsity pattern
         * @param[in] get_cost function to extract matrix entry pairs */
        Ratio(const Graph& gra, Fn get_cost) : _gra{gra}, _get_cost{std::move(get_cost)} {}

        /** @brief Copy constructor */
        explicit Ratio(const Ratio&) = default;

        /** @brief Evaluate the constraint function for an edge
         * @details Computes min(pi - a_ji, a_ij - psi) where x = (pi, psi)
         * in log scale. A positive result indicates the constraint is satisfied.
         * The edge parameter is the graph's native edge data (e.g., the
         * (a_ij, a_ji) cost pair from the adjacency structure).
         *
         * The constraint function for edge @f$(i, j)@f$ with matrix entries @f$(a_{ij}, a_{ji})@f$:
         * @f[
         *     h_{ij}(x) = \min(\pi - a_{ji},\; a_{ij} - \psi)
         * @f]
         * where @f$x = (\pi, \psi)@f$ in log scale.
         *
         * @dot
         *   digraph ratio_eval {
         *     bgcolor="transparent";
         *     node [shape=box, style=filled, fillcolor="#d4e6f1"];
         *     input [label="Edge (a_ij, a_ji)\nx = (pi, psi)", fillcolor="#a9cce3"];
         *     compute [label="pi - a_ji\nvs\na_ij - psi"];
         *     result [label="min(pi - a_ji,\na_ij - psi)", fillcolor="#7fb3d8"];
         *     input -> compute -> result;
         *   }
         * @enddot
         *
         * @param[in] edge the matrix entry (edge) to evaluate
         * @param[in] x vector containing (pi, psi) in log scale
         * @return constraint value (positive if satisfied) */
        auto eval(const auto& edge, const Vec& x) const -> double {
            const auto [aij, aji] = this->_get_cost(edge);
            return std::min(x[0] - aji, aij - x[1]);
        }

        /** @brief Compute the gradient of the constraint function
         * @details Returns a subgradient vector indicating which bound is
         * active: [1.0, 0.0] if the upper bound (pi) is active, or
         * [0.0, -1.0] if the lower bound (psi) is active.
         *
         * The subgradient of the constraint function:
         * @f[
         *     \nabla h_{ij}(x) =
         *     \begin{cases}
         *         (1, 0) & \text{if } \pi - a_{ji} < a_{ij} - \psi \\
         *         (0, -1) & \text{otherwise}
         *     \end{cases}
         * @f]
         *
         * @dot
         *   digraph ratio_grad {
         *     bgcolor="transparent";
         *     node [shape=box, style=filled, fillcolor="#d4e6f1"];
         *     input [label="Edge (a_ij, a_ji)\nx = (pi, psi)", fillcolor="#a9cce3"];
         *     branch [label="pi - a_ji <\na_ij - psi?", shape=diamond, fillcolor="#f9e79f"];
         *     g1 [label="(1, 0)\npi bound active", fillcolor="#d5f5e3"];
         *     g2 [label="(0, -1)\npsi bound active", fillcolor="#fadbd8"];
         *     input -> branch;
         *     branch -> g1 [label="Yes", color="#27ae60"];
         *     branch -> g2 [label="No", color="#e74c3c"];
         *   }
         * @enddot
         *
         * @param[in] edge the matrix entry (edge) for gradient computation
         * @param[in] x vector containing (pi, psi) in log scale
         * @return gradient vector [d/d(pi), d/d(psi)] */
        auto grad(const auto& edge, const Vec& x) const -> Vec {
            const auto [aij, aji] = this->_get_cost(edge);
            return (x[0] - aji < aij - x[1]) ? Vec{1., 0.} : Vec{0., -1.};
        }
    };

    NetworkOracle<Graph, Mapping, Ratio> _network;

  public:
    /** @brief Construct a new optscaling oracle object
     * @param[in] gra The graph representing matrix sparsity
     * @param[in,out] utx Vertex potential mapping (scaling factors)
     * @param[in] get_cost Function to extract matrix entry pairs from edge data */
    OptScalingOracle(const Graph& gra, Mapping& utx, Fn get_cost)
        : _network(gra, utx, Ratio{gra, get_cost}) {}

    /** @brief Copy constructor */
    explicit OptScalingOracle(const OptScalingOracle&) = default;
    OptScalingOracle(OptScalingOracle&&) = default;
    OptScalingOracle& operator=(const OptScalingOracle&) = default;
    OptScalingOracle& operator=(OptScalingOracle&&) = default;
    ~OptScalingOracle() = default;

    /** @brief Assess optimality at point x (cutting plane interface)
     *
     * The optimal scaling problem minimizes the ratio @f$\pi / \psi@f$.
     * In log scale, the objective is @f$s = \pi - \psi@f$.
     * The algorithm maintains @f$t@f$ as the best-so-far optimal value:
     * @f[
     *     f(x) = s - t = (\pi - \psi) - t
     * @f]
     * If @f$f(x) < 0@f$, the solution improves; otherwise a cutting plane @f$(1, -1)@f$ is
     * returned.
     *
     * @dot
     *   digraph assess_optim {
     *     rankdir=LR; bgcolor="transparent";
     *     node [shape=box, style=filled, fillcolor="#d4e6f1"];
     *     x [label="x = (pi, psi)", fillcolor="#a9cce3"];
     *     feas [label="Check\nfeasibility\nvia network", shape=diamond, fillcolor="#f9e79f"];
     *     cut [label="Return\ncutting plane\n(g, f)", fillcolor="#fadbd8"];
     *     obj [label="s = pi - psi\nfj = s - t"];
     *     improve [label="Improve:\nt = s", fillcolor="#d5f5e3"];
     *     opt_cut [label="Return\n(1,-1), 0", fillcolor="#7fb3d8"];
     *     done [label="Return\n(1,-1), fj", fillcolor="#7fb3d8"];
     *     x -> feas;
     *     feas -> cut [label="infeasible", color="#e74c3c"];
     *     feas -> obj [label="feasible", color="#27ae60"];
     *     obj -> improve [label="fj < 0", color="#27ae60"];
     *     obj -> done [label="fj >= 0", color="#e74c3c"];
     *     improve -> opt_cut;
     *   }
     * @enddot
     *
     * @param[in] x (pi, psi) in log scale
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
     * @param[in] x (pi, psi) in log scale
     * @param[in,out] t the best-so-far optimal value
     * @return tuple of (cut, whether gamma was updated) */
    auto operator()(const Vec& x, double& t) -> std::tuple<Cut, bool> { return assess_optim(x, t); }
};
