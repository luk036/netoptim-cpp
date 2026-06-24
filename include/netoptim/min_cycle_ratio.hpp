// -*- coding: utf-8 -*-
#pragma once

#include <algorithm>
#include <py2cpp/py2cpp.hpp>

#include "parametric.hpp"  // import max_parametric

template <typename Graph, typename T, typename Fn1, typename Fn2, typename Mapping>
auto min_cycle_ratio(const Graph& gra, T& r0, Fn1&& get_cost, Fn2&& get_time, Mapping&& dist,
                     size_t max_iters = 1000) {
    // ponytail: deduce Edge type using the same helpers as NegCycleFinder
    using Elem = decltype(*std::declval<const Graph&>().begin());
    using Nbrs = std::remove_cv_t<std::remove_reference_t<
        decltype(_get_val(std::declval<Elem>(), std::declval<const Graph&>()))>>;
    using NbrElem = decltype(*std::declval<const Nbrs&>().begin());
    using Edge = std::remove_cv_t<std::remove_reference_t<
        decltype(_get_val(std::declval<NbrElem>(), std::declval<const Nbrs&>()))>>;
    using edge_t = Edge;

    auto calc_ratio = [&](const auto& C) -> T {
        using cost_T = decltype(get_cost(std::declval<edge_t>()));
        using time_T = decltype(get_time(std::declval<edge_t>()));
        auto total_cost = cost_T(0);
        auto total_time = time_T(0);
        for (auto&& edge : C) {
            total_cost += get_cost(edge);
            total_time += get_time(edge);
        }
        return T(total_cost) / total_time;
    };

    auto calc_weight
        = [&](const T& r, const edge_t& edge) -> T { return get_cost(edge) - r * T(get_time(edge)); };

    return max_parametric(gra, r0, std::move(calc_weight), std::move(calc_ratio),
                          std::forward<Mapping>(dist), max_iters);
}
