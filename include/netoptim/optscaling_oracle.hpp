// -*- coding: utf-8 -*-
#pragma once

#include <cassert>
// #include <xtensor/xarray.hpp>

#include <valarray>>

#include "network_oracle.hpp"

/*!
 * @brief Oracle for Optimal Matrix Scaling.
 *
 *    This example is taken from [Orlin and Rothblum, 1985]:
 *
 *        min     \pi/\phi
 *        s.t.    \phi \le utx[i] * |aij| * utx[j]^-1 \le \pi,
 *                \forall aij != 0,
 *                \pi, \phi, utx, positive
 *
 * @tparam Graph
 * @tparam Mapping
 * @tparam Fn
 */
template <typename Graph, typename Mapping, typename Fn>  //
class OptScalingOracle {
    // using Vec = xt::xarray<double, xt::layout_type::row_major>;
    using Vec = std::valarray<double>;
    using node_t = typename Graph::node_t;
    using edge_t = std::pair<node_t, node_t>;
    using Cut = std::pair<Vec, double>;

    /**
     * @brief Ratio
     *
     */
    class Ratio {
      private:
        const Graph &_gra;
        Fn _get_cost;

      public:
        /*!
         * @brief Construct a new Ratio object
         *
         * @param[in] gra
         * @param[in] get_cost
         */
        Ratio(const Graph &gra, Fn get_cost) : _gra{gra}, _get_cost{std::move(get_cost)} {}

        /**
         * @brief Construct a new Ratio object (only explicitly)
         *
         */
        explicit Ratio(const Ratio &) = default;

        /*!
         * @brief Evaluate function
         *
         * @param[in] edge
         * @param[in] x $(\pi, \phi)$ in log scale
         * @return double
         */
        auto eval(const edge_t &edge, const Vec &x) const -> double {
            // const auto [utx, vtx] = this->_gra.end_points(edge);
            const auto cost = this->_get_cost(edge);
            const auto [utx, vtx] = edge;
            assert(utx != vtx);
            return (utx < vtx) ? x[0] - cost : cost - x[1];
        }

        /*!
         * @brief Gradient function
         *
         * @param[in] edge
         * @param[in] x $(\pi, \phi)$ in log scale
         * @return Vec
         */
        auto grad(const edge_t &edge, const Vec &) const -> Vec {
            // const auto [utx, vtx] = this->_gra.end_points(edge);
            const auto [utx, vtx] = edge;
            assert(utx != vtx);
            return (utx < vtx) ? Vec{1., 0.} : Vec{0., -1.};
        }
    };

    NetworkOracle<Graph, Mapping, Ratio> _network;

  public:
    /*!
     * @brief Construct a new optscaling oracle object
     *
     * @param[in] gra
     * @param[in,out] utx
     * @param[in] get_cost
     */
    OptScalingOracle(const Graph &gra, Mapping &utx, Fn get_cost)
        : _network(gra, utx, Ratio{gra, get_cost}) {}

    /**
     * @brief Construct a new optscaling oracle object
     *
     */
    explicit OptScalingOracle(const OptScalingOracle &) = default;

    // OptScalingOracle& operator=(const OptScalingOracle&) = delete;
    // OptScalingOracle(optscaling_oracle&&) = default;

    /*!
     * @brief Make object callable for cutting_plane_optim()
     *
     * @param[in] x (\pi, \phi) in log scale
     * @param[in,out] t the best-so-far optimal value
     * @return std::tuple<Cut, double>
     *
     * @see cutting_plane_optim
     */
    auto assess_optim(const Vec &x, double &t) -> std::tuple<Cut, bool> {
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
     * @brief Make object callable for cutting_plane_optim()
     *
     * @param[in] x (\pi, \phi) in log scale
     * @param[in,out] t the best-so-far optimal value
     * @return std::tuple<Cut, double>
     *
     * @see cutting_plane_optim
     */
    auto operator()(const Vec &x, double &t) -> std::tuple<Cut, bool> { return assess_optim(x, t); }
};
