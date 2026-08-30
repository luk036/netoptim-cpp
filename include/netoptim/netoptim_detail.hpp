// -*- coding: utf-8 -*-
#pragma once

/**
 * @file netoptim_detail.hpp
 * @brief Internal implementation helpers shared by the netoptim algorithms
 *
 * This header centralises the graph type trait (Adapter pattern), the
 * parametric search skeleton (Template Method pattern), and the oracle
 * concepts (Strategy contract) used across the public headers. It is an
 * implementation detail of the library; the public API is unaffected.
 */

#include <digraphx/neg_cycle.hpp>  // import NegCycleFinder, _get_key / _get_val
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace netoptim_detail {

    template <typename T>
    concept HasKeyType = requires { typename T::key_type; };

    /**
     * @brief Type trait deriving Node / Edge / Cycle from any supported graph container.
     *
     * @tparam Graph a container of containers (map-of-maps, list-of-lists,
     *         MapAdapter-wrapped, ...)
     */
    template <typename Graph> struct graph_traits {
        using Elem = decltype(*std::declval<const Graph&>().begin());
        using Nbrs = std::remove_cv_t<std::remove_reference_t<decltype(_get_val(
            std::declval<Elem>(), std::declval<const Graph&>()))>>;
        using NbrElem = decltype(*std::declval<const Nbrs&>().begin());
        using Edge = std::remove_cv_t<std::remove_reference_t<decltype(_get_val(
            std::declval<NbrElem>(), std::declval<const Nbrs&>()))>>;
        using Cycle = std::vector<Edge>;
    };

    /**
     * @brief Template Method: parametric max-problem search skeleton.
     *
     * The iteration structure is fixed here; the distance function and the
     * zero-canceling function are injected as strategies. Returns the critical
     * cycle that determines the optimal parameter (empty if none).
     *
     * @tparam Graph directed graph type
     * @tparam T numeric type for the parameter r
     * @tparam Fn1 distance callable (parameter, edge) -> weight
     * @tparam Fn2 zero-canceling callable (cycle) -> parameter
     * @tparam Mapping distance mapping (vertex -> distance)
     * @param[in] gra directed graph containing the network structure
     * @param[in,out] r_opt parameter to be maximized, updated with optimal value
     * @param[in] distrance monotone decreasing function of parameter r
     * @param[in] zero_cancel function to compute new parameter from cycle
     * @param[in,out] dist distance mapping used in the algorithm
     * @param[in] max_iters maximum number of iterations (default: 1000)
     * @return Cycle the critical cycle that determines the optimal parameter
     */
    template <typename Graph, typename T, typename Fn1, typename Fn2, typename Mapping>
    auto parametric_search(const Graph& gra, T& r_opt, Fn1&& distrance, Fn2&& zero_cancel,
                           Mapping&& dist, size_t max_iters) {
        using Edge = typename graph_traits<Graph>::Edge;
        using Cycle = std::vector<Edge>;

        auto get_weight = [&distrance, &r_opt](const Edge& edge) -> T {
            return static_cast<T>(distrance(r_opt, edge));
        };

        auto ncf = NegCycleFinder<Graph>(gra);
        auto r_min = r_opt;
        auto c_min = Cycle{};
        auto c_opt = Cycle{};

        for (auto niter = 0U; niter != max_iters; ++niter) {
            for (auto&& ci : ncf.howard(dist, get_weight)) {
                auto ri = static_cast<T>(zero_cancel(ci));
                if (r_min > ri) {
                    r_min = ri;
                    c_min = std::move(ci);
                }
            }
            if (r_min >= r_opt) break;
            c_opt = std::move(c_min);
            r_opt = r_min;
        }
        return c_opt;
    }

    // --- Strategy contract for the cutting-plane oracles -------------------

    /**
     * @brief Feasibility oracle: assess x and produce a cutting plane if infeasible.
     *
     * @tparam O the oracle type
     * @tparam A the point / array type
     */
    template <typename O, typename A>
    concept OracleFeas = requires(O& o, const A& x) {
        { o.assess_feas(x) };
    };

    /**
     * @brief Optimality oracle: assess x against a best-so-far value g.
     *
     * @tparam O the oracle type
     * @tparam A the point / array type
     * @tparam N the best-so-far value type
     */
    template <typename O, typename A, typename N>
    concept OracleOptim = requires(O& o, const A& x, N& g) {
        { o.assess_optim(x, g) };
    };

}  // namespace netoptim_detail
