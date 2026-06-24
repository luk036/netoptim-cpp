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
 */

template <typename Graph, typename Mapping, typename Fn>  //
    requires HasKeyType<Graph>
class OptScalingOracle {
    using Vec = std::valarray<double>;
    using node_t = typename Graph::key_type;
    using Cut = std::pair<Vec, double>;

    class Ratio {
      private:
        const Graph& _gra;
        Fn _get_cost;

      public:
        Ratio(const Graph& gra, Fn get_cost) : _gra{gra}, _get_cost{std::move(get_cost)} {}

        explicit Ratio(const Ratio&) = default;

        // ponytail: edge is the graph's native edge data (e.g. pair<double,double> for cost)
        // eval computes min(x[0] - aji, aij - x[1]) matching Python sibling
        auto eval(const auto& edge, const Vec& x) const -> double {
            const auto [aij, aji] = this->_get_cost(edge);
            return std::min(x[0] - aji, aij - x[1]);
        }

        auto grad(const auto& edge, const Vec& x) const -> Vec {
            const auto [aij, aji] = this->_get_cost(edge);
            return (x[0] - aji < aij - x[1]) ? Vec{1., 0.} : Vec{0., -1.};
        }
    };

    NetworkOracle<Graph, Mapping, Ratio> _network;

  public:
    OptScalingOracle(const Graph& gra, Mapping& utx, Fn get_cost)
        : _network(gra, utx, Ratio{gra, get_cost}) {}

    explicit OptScalingOracle(const OptScalingOracle&) = default;
    OptScalingOracle(OptScalingOracle&&) = default;
    OptScalingOracle& operator=(const OptScalingOracle&) = default;
    OptScalingOracle& operator=(OptScalingOracle&&) = default;
    ~OptScalingOracle() = default;

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

    auto operator()(const Vec& x, double& t) -> std::tuple<Cut, bool> { return assess_optim(x, t); }
};
