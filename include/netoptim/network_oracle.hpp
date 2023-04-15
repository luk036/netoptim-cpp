// -*- coding: utf-8 -*-
#pragma once

// #include <ellalgo/utility.hpp>
#include <optional>

#include "neg_cycle.hpp" // import negCycleFinder

/*!
 * @brief Oracle for Parametric Network Problem.
 *
 *    This oracle solves the following feasibility problem:
 *
 *        find    x, u
 *        s.t.    u[j] - u[i] \le h(e, x)
 *                \forall e(i, j) \in E
 *
 * @tparam Graph
 * @tparam Container
 * @tparam Fn
 */
template <typename Graph, typename Container, typename Fn> class NetworkOracle {
  using edge_t = typename Graph::edge_t;

private:
  const Graph &_gra;
  Container &_u; // reference???
  NegCycleFinder<Graph> _S;
  Fn _h;

public:
  /*!
   * @brief Construct a new network oracle object
   *
   * @param[in] gra a directed graph (V, E)
   * @param[in,out] u list or dictionary
   * @param[in] h function evaluation and gradient
   */
  NetworkOracle(const Graph &gra, Container &u, Fn h)
      : _gra{gra}, _u{u}, _S(gra), _h{std::move(h)} {}

  /**
   * @brief Construct a new network oracle object
   *
   */
  explicit NetworkOracle(const NetworkOracle &) = default;

  // NetworkOracle& operator=(const NetworkOracle&) = delete;
  // NetworkOracle(network_oracle&&) = default;

  /*!
   * @brief
   *
   * @param[in] target the best-so-far optimal value
   */
  template <typename Num> auto update(const Num &target) -> void {
    this->_h.update(target);
  }

  /*!
   * @brief Make object callable for cutting_plane_feas()
   *
   * @tparam T
   * @param[in] x
   * @return std::optional<std::tuple<T, double>>
   */
  template <typename Arr>
  auto assess_feas(const Arr &xval) -> std::optional<std::pair<Arr, double>> {
    auto get_weight = [this, &xval](const edge_t &edge) -> double {
      return this->_h.eval(edge, xval);
    };

    auto C = this->_S.find_neg_cycle(this->_u, get_weight);
    if (C.empty()) {
      return {};
    }

    auto grad = zeros(xval);
    auto fval = 0.;
    for (auto &&edge : C) {
      fval -= this->_h.eval(edge, xval);
      grad -= this->_h.grad(edge, xval);
    }
    return {{std::move(grad), fval}};
  }

  /*!
   * @brief Make object callable for cutting_plane_feas()
   *
   * @tparam T
   * @param[in] x
   * @return std::optional<std::tuple<T, double>>
   */
  template <typename Arr>
  auto operator()(const Arr &xvar) -> std::optional<std::pair<Arr, double>> {
    return this->assess_feas(xvar);
  }
};
