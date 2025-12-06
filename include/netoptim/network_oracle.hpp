// -*- coding: utf-8 -*-
#pragma once

// #include <ellalgo/utility.hpp>
#include <optional>

#include "neg_cycle.hpp"  // import negCycleFinder

/*!
 * @file network_oracle.hpp
 * @brief Oracle for Parametric Network Problems
 *
 * This module provides an oracle implementation for solving feasibility
 * problems in network optimization. The oracle acts as a separation oracle
 * that can determine if a given point x is feasible with respect to the
 * network constraints, and if not, provides a cutting plane.
 *
 * The feasibility problem formulation:
 *     find    x, utx
 *     s.t.    utx[j] - utx[i] &le; h(edge, x)
 *             &forall; edge(i, j) &isin; E
 *
 * where h is a function that depends on the decision variables x.
 * This is commonly used in cutting-plane methods for network optimization.
 */

/*!
 * @brief Oracle for Parametric Network Problems
 *
 * This class implements a separation oracle for network feasibility problems.
 * It uses negative cycle detection to check feasibility and generate cutting
 * planes when violations are found.
 *
 * The oracle maintains:
 * - A reference to the graph structure
 * - A mapping of vertex potentials (utx)
 * - A negative cycle finder for violation detection
 * - A function h that evaluates edge constraints
 *
 * @tparam Graph Type of the directed graph
 * @tparam Mapping Type of vertex potential mapping
 * @tparam Fn Type of the constraint function h
 */
template <typename Graph, typename Mapping, typename Fn> class NetworkOracle {
    using node_t = typename Graph::node_t;
    using edge_t = std::pair<node_t, node_t>;

  private:
    const Graph &_gra;
    Mapping &_u;  // reference???
    NegCycleFinder<Graph> _S;
    Fn _h;

  public:
    /*!
     * @brief Construct a new network oracle object
     *
     * @param[in] gra a directed graph (V, E) representing the network
     * @param[in,out] utx vertex potential mapping (updated during operation)
     * @param[in] h function for constraint evaluation and gradient computation
     */
    NetworkOracle(const Graph &gra, Mapping &utx, Fn h)
        : _gra{gra}, _u{utx}, _S(gra), _h{std::move(h)} {}

    /**
     * @brief Construct a new network oracle object
     *
     */
    explicit NetworkOracle(const NetworkOracle &) = default;

    // NetworkOracle& operator=(const NetworkOracle&) = delete;
    // NetworkOracle(network_oracle&&) = default;

    /*!
     * @brief Update the oracle with a new parameter value
     *
     * This method updates the internal constraint function with a new
     * parameter value, typically used in parametric optimization where
     * the constraints depend on a parameter that changes during the algorithm.
     *
     * @param[in] gamma the new parameter value (best-so-far optimal value)
     */
    template <typename Num> auto update(const Num &gamma) -> void { this->_h.update(gamma); }

    /*!
     * @brief Assess feasibility and generate cutting plane if needed
     *
     * This is the main oracle method that checks if the current point xval
     * is feasible with respect to the network constraints. If infeasible,
     * it returns a cutting plane (gradient and function value) that separates
     * the infeasible point from the feasible region.
     *
     * The method works by:
     * 1. Computing edge weights using the constraint function
     * 2. Searching for negative cycles using these weights
     * 3. If found, computing the gradient and function value for the cut
     * 4. Returning the cutting plane or empty if feasible
     *
     * @tparam Arr Type of the input array/vector
     * @param[in] xval input values to be assessed for feasibility
     * @return std::optional<std::pair<Arr, double>> Empty if feasible,
     *         otherwise a pair containing gradient and function value for cutting plane
     */
    template <typename Arr> auto assess_feas(const Arr &xval)
        -> std::optional<std::pair<Arr, double>> {
        auto get_weight
            = [this, &xval](const edge_t &edge) -> double { return this->_h.eval(edge, xval); };

        auto C = this->_S.find_neg_cycle(this->_u, get_weight);
        if (C.empty()) {
            return {};
        }

        auto grad = zeros(xval);
        auto fval = 0.0;
        for (auto &&edge : C) {
            fval -= this->_h.eval(edge, xval);
            grad -= this->_h.grad(edge, xval);
        }
        return {{std::move(grad), fval}};
    }

    /*!
     * @brief Function call operator for cutting plane methods
     *
     * This operator makes the oracle callable, which is convenient for use
     * with cutting plane algorithms. It simply forwards to the assess_feas
     * method.
     *
     * @tparam Arr Type of the input array/vector
     * @param[in] xvar input variables to be assessed
     * @return std::optional<std::pair<Arr, double>> Same as assess_feas
     */
    template <typename Arr> auto operator()(const Arr &xvar)
        -> std::optional<std::pair<Arr, double>> {
        return this->assess_feas(xvar);
    }
};
