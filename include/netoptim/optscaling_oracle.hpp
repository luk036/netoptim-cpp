// -*- coding: utf-8 -*-
#pragma once

#include <cassert>
#include <xtensor/xarray.hpp>

#include "network_oracle.hpp"

/*!
 * @brief Oracle for Optimal Matrix Scaling.
 *
 *    This example is taken from [Orlin and Rothblum, 1985]:
 *
 *        min     \pi/\phi
 *        s.t.    \phi \le u[i] * |aij| * u[j]^-1 \le \pi,
 *                \forall aij != 0,
 *                \pi, \phi, u, positive
 *
 * @tparam Graph
 * @tparam Container
 * @tparam Fn
 */
template <typename Graph, typename Container, typename Fn> //
class OptScalingOracle {
  using Arr = xt::xarray<double, xt::layout_type::row_major>;
  using edge_t = typename Graph::edge_t;
  using Cut = std::pair<Arr, double>;

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
    Ratio(const Graph &gra, Fn get_cost)
        : _gra{gra}, _get_cost{std::move(get_cost)} {}

    /**
     * @brief Construct a new Ratio object (only explicitly)
     *
     */
    explicit Ratio(const Ratio &) = default;

    /*!
     * @brief Evaluate function
     *
     * @param[in] e
     * @param[in] x $(\pi, \phi)$ in log scale
     * @return double
     */
    auto eval(const edge_t &e, const Arr &x) const -> double {
      const auto [u, v] = this->_gra.end_points(e);
      const auto cost = this->_get_cost(e);
      assert(u != v);
      return (u < v) ? x(0) - cost : cost - x(1);
    }

    /*!
     * @brief Gradient function
     *
     * @param[in] e
     * @param[in] x $(\pi, \phi)$ in log scale
     * @return Arr
     */
    auto grad(const edge_t &e, const Arr &) const -> Arr {
      const auto [u, v] = this->_gra.end_points(e);
      assert(u != v);
      return (u < v) ? Arr{1., 0.} : Arr{0., -1.};
    }
  };

  NetworkOracle<Graph, Container, Ratio> _network;

public:
  /*!
   * @brief Construct a new optscaling oracle object
   *
   * @param[in] gra
   * @param[in,out] u
   * @param[in] get_cost
   */
  OptScalingOracle(const Graph &gra, Container &u, Fn get_cost)
      : _network(gra, u, Ratio{gra, get_cost}) {}

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
   * @param[in] t the best-so-far optimal value
   * @return std::tuple<Cut, double>
   *
   * @see cutting_plane_optim
   */
  auto assess_optim(const Arr &x, double &t) -> std::tuple<Cut, bool> {
    const auto cut = this->_network(x);
    if (cut) {
      return {*cut, false};
    }
    auto s = x(0) - x(1);
    auto fj = s - t;
    if (fj < 0) {
      t = s;
      return {{Arr{1., -1.}, 0.}, true};
    }
    return {{Arr{1., -1.}, fj}, false};
  }

  /*!
   * @brief Make object callable for cutting_plane_optim()
   *
   * @param[in] x (\pi, \phi) in log scale
   * @param[in] t the best-so-far optimal value
   * @return std::tuple<Cut, double>
   *
   * @see cutting_plane_optim
   */
  auto operator()(const Arr &x, double &t) -> std::tuple<Cut, bool> {
    return assess_optim(x, t);
  }
};
