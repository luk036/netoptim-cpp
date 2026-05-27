// -*- coding: utf-8 -*-
#pragma once

#include <optional>

#include <digraphx/neg_cycle.hpp>  // for NegCycleFinder (Howard's algorithm)

/*!
 * @brief Helper to create a zero-initialized value of type T
 *
 * This function is used by NetworkOracle to initialize gradient arrays.
 * It returns a default-constructed value of the given type, which for
 * numeric types and standard containers produces a zero-valued object.
 *
 * @tparam T Type of the value to zero-initialize
 * @return T A default-constructed value of type T
 */
template <typename T> constexpr auto zeros(const T&) noexcept -> T { return T{}; }

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
 *
 * Graph requirements (for digraphx NegCycleFinder):
 * - Iterable: for (const auto& [node, neighbors] : graph)
 * - Neighbors iterable: for (const auto& [target, edge_data] : neighbors)
 * - Supports .size() for reserve hint (optional)
 *
 * @tparam Graph Type of the directed graph
 * @tparam Mapping Type of vertex potential mapping
 * @tparam Fn Type of the constraint function h (must provide eval, grad, update)
 */
template <typename Graph, typename Mapping, typename Fn> class NetworkOracle {
  private:
    const Graph& _gra;
    Mapping& _u;
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
    NetworkOracle(const Graph& gra, Mapping& utx, Fn h)
        : _gra{gra}, _u{utx}, _S(gra), _h{std::move(h)} {}

    explicit NetworkOracle(const NetworkOracle&) = default;

    /*!
     * @brief Update the oracle with a new parameter value
     *
     * Forwards the gamma value to the underlying constraint function's
     * update method.
     *
     * @param[in] gamma the new parameter value (best-so-far optimal value)
     */
    template <typename Num> auto update(const Num& gamma) -> void { this->_h.update(gamma); }

    /*!
     * @brief Assess feasibility and generate cutting plane if needed
     *
     * Uses Howard's algorithm via digraphx NegCycleFinder to detect
     * negative cycles. If a negative cycle exists, computes the cutting
     * plane (gradient, intercept) from the cycle's edges.
     *
     * @tparam Arr Type of the input array/vector
     * @param[in] xval input values to be assessed for feasibility
     * @return std::optional<std::pair<Arr, double>> Empty if feasible,
     *         otherwise a pair containing gradient and function value
     */
    template <typename Arr> auto assess_feas(const Arr& xval)
        -> std::optional<std::pair<Arr, double>> {
        using EdgeData = decltype(
            (*std::declval<Graph>().begin()).second.begin()->second);

        auto get_weight = [this, &xval](const EdgeData& edge) -> double {
            return this->_h.eval(edge, xval);
        };

        for (auto&& C : this->_S.howard(this->_u, std::move(get_weight))) {
            auto grad = zeros(xval);
            auto fval = 0.0;
            for (auto&& edge : C) {
                fval -= this->_h.eval(edge, xval);
                grad -= this->_h.grad(edge, xval);
            }
            return {{std::move(grad), fval}};
        }

        return {};
    }

    /*!
     * @brief Function call operator for cutting plane methods
     *
     * Makes the oracle callable for use with ellipsoid algorithms.
     *
     * @tparam Arr Type of the input array/vector
     * @param[in] xvar input variables to be assessed
     * @return std::optional<std::pair<Arr, double>> Same as assess_feas
     */
    template <typename Arr> auto operator()(const Arr& xvar)
        -> std::optional<std::pair<Arr, double>> {
        return this->assess_feas(xvar);
    }
};
