// -*- coding: utf-8 -*-
#pragma once

#include <digraphx/neg_cycle.hpp>  // import NegCycleFinder
#include <limits>
#include <optional>
#include <tuple>
#include <valarray>

#include "netoptim_detail.hpp"  // import netoptim_detail helpers

namespace netoptim_detail {

    /** @brief Edge for the clock-skew scheduling oracle: delay and type. */
    struct timing_edge {
        double delay = 0.0;
        char type = 's';  ///< 's' setup, 'h' hold
    };

}  // namespace netoptim_detail

/**
 * @brief Oracle for minimum-period / maximum-yield clock skew scheduling.
 *
 * Node potentials are the clock arrival times; the iterate is
 * @f$x = (\mathrm{TCP}, \beta)@f$ and the objective is
 * @f$\min \mathrm{TCP} / \beta@f$ subject to the timing constraint graph being
 * feasible. The graph's edges are @ref netoptim_detail::timing_edge; a
 * negative cycle, if any, supplies the separating cutting plane.
 *
 * A negative cycle at @f$x@f$ gives the cut @f$(g, f)@f$ with
 * @f$f = -\sum_{e \in C} w(e)@f$ and
 * @f$g = (-\#\{\text{setup edges}\}, |C|)@f$.
 *
 * @tparam Graph Type of the directed graph (edge = netoptim_detail::timing_edge)
 * @tparam Mapping Type of the vertex potential mapping
 */
template <typename Graph, typename Mapping> class ClockScheduleOracle {
    using Vec = std::valarray<double>;
    using Cut = std::pair<Vec, double>;
    using Edge = typename netoptim_detail::graph_traits<Graph>::Edge;

    Mapping& _dist;
    NegCycleFinder<Graph> _S;

  public:
    /** @brief Construct a new clock-schedule oracle
     * @param[in] gra a directed graph (V, E) representing the timing constraints
     * @param[in,out] dist vertex potential mapping (updated during operation) */
    ClockScheduleOracle(const Graph& gra, Mapping& dist) : _dist{dist}, _S{gra} {}

    /** @brief Assess optimality at x = (TCP, beta) (cutting-plane interface)
     * @param[in] x the current iterate (TCP, beta)
     * @param[in,out] gamma best-so-far value of TCP / beta
     * @return tuple of (cutting plane, whether gamma improved) */
    auto assess_optim(const Vec& x, double& gamma) -> std::tuple<Cut, bool> {
        const auto tcp = x[0];
        const auto beta = x[1];
        if (tcp < 0.0) {
            return {Cut{Vec{-1.0, 0.0}, -tcp}, false};
        }
        if (const auto fj = tcp - gamma * beta; fj > 0.0) {
            return {Cut{Vec{1.0, -gamma}, fj}, false};
        }
        const auto get_weight = [tcp, beta](const Edge& edge) -> double {
            return edge.type == 's' ? tcp - edge.delay - beta : edge.delay - beta;
        };
        // Howard may yield spurious non-negative cycles (verification is per-edge),
        // so keep only the most-violated cycle and cut only if it is negative.
        auto best = std::optional<Cut>{};
        auto min_weight = 0.0;
        for (auto&& C : this->_S.howard(this->_dist, get_weight)) {
            auto weight_sum = 0.0;
            auto setup = 0.0;
            for (auto&& edge : C) {
                weight_sum += get_weight(edge);
                setup += edge.type == 's' ? 1.0 : 0.0;
            }
            if (weight_sum < min_weight) {
                min_weight = weight_sum;
                best = Cut{Vec{-setup, static_cast<double>(C.size())}, -weight_sum};
            }
        }
        if (best) {
            return {std::move(*best), false};
        }
        gamma = tcp / beta;
        return {Cut{Vec{1.0, -1.0}, 0.0}, true};
    }

    /** @brief Function call operator for cutting_plane_optim()
     * @param[in] x the current iterate (TCP, beta)
     * @param[in,out] gamma best-so-far value of TCP / beta
     * @return tuple of (cutting plane, whether gamma improved) */
    auto operator()(const Vec& x, double& gamma) -> std::tuple<Cut, bool> {
        return assess_optim(x, gamma);
    }
};
