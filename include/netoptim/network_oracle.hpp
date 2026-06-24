// -*- coding: utf-8 -*-
#pragma once

#include <digraphx/neg_cycle.hpp>  // import NegCycleFinder
#include <optional>
#include <type_traits>

namespace {
    template <typename T>
    concept HasKeyType = requires { typename T::key_type; };
}  // namespace

/**
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
 * Edge weights are accessed via the actual edge data from the graph's
 * adjacency structure (the "get_weight" method), rather than synthesized
 * (u,v) node pairs. This matches the Python sibling implementation.
 */

/**
 * @brief Oracle for Parametric Network Problems
 *
 * This class implements a separation oracle for network feasibility problems.
 * It uses Howard's method for negative cycle detection to check feasibility
 * and generate cutting planes when violations are found.
 *
 * @tparam Graph Type of the directed graph
 * @tparam Mapping Type of vertex potential mapping
 * @tparam Fn Type of the constraint function h
 */
template <typename Graph, typename Mapping, typename Fn>
    requires HasKeyType<Graph>
class NetworkOracle {
    using node_t = typename Graph::key_type;

  private:
    const Graph& _gra;
    Mapping& _u;
    NegCycleFinder<Graph> _S;
    Fn _h;

  public:
    NetworkOracle(const Graph& gra, Mapping& utx, Fn h)
        : _gra{gra}, _u{utx}, _S(gra), _h{std::move(h)} {}

    explicit NetworkOracle(const NetworkOracle&) = default;

    template <typename Num> auto update(const Num& gamma) -> void { this->_h.update(gamma); }

    template <typename Arr> auto assess_feas(const Arr& xval)
        -> std::optional<std::pair<Arr, double>> {
        using Nbrs1 = decltype((*std::declval<const Graph&>().begin()).second);
        using Nbrs = std::remove_cv_t<std::remove_reference_t<Nbrs1>>;
        using Edge1 = decltype((*std::declval<const Nbrs&>().begin()).second);
        using Edge = std::remove_cv_t<std::remove_reference_t<Edge1>>;

        auto get_weight = [this, &xval](const Edge& edge) -> double {
            return this->_h.eval(edge, xval);
        };

        for (auto&& C : this->_S.howard(this->_u, get_weight)) {
            auto grad = [&]() -> Arr {
                if constexpr (std::is_arithmetic_v<Arr>) {
                    return Arr{};
                } else {
                    return Arr(xval.size());
                }
            }();
            auto fval = 0.0;
            for (auto&& edge : C) {
                fval -= this->_h.eval(edge, xval);
                grad -= this->_h.grad(edge, xval);
            }
            return std::pair{std::move(grad), fval};
        }
        return {};
    }

    template <typename Arr> auto operator()(const Arr& xvar)
        -> std::optional<std::pair<Arr, double>> {
        return this->assess_feas(xvar);
    }
};
