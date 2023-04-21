// -*- coing: utf-8 -*-
#pragma once

/*!
Negative cycle detection for weighed graphs.
**/
// #include <ThreadPool.h>

#include <cassert>
#include <memory> // for unique_ptr
#include <optional>
#include <unordered_map>
#include <vector>

/*!
 * @brief negative cycle
 *
 * @tparam Graph
 *
 * Note: Bellman-Ford's shortest-path algorithm (BF) is NOT the best way to
 *       detect negative cycles, because
 *
 *  1. BF needs a source node.
 *  2. BF detect whether there is a negative cycle at the fianl stage.
 *  3. BF restarts the solution (dist[u]) every time.
 */
template <typename Graph> //
class NegCycleFinder {
  using node_t = typename Graph::node_t;
  using edge_t = typename Graph::edge_t;

  std::unordered_map<node_t, node_t> _pred{};
  std::unordered_map<node_t, edge_t> _edge{};

private:
  const Graph &_gra; // const???
  // std::unique_ptr<node_t> _cycle_start;

public:
  /*!
   * @brief Construct a new neg Cycle Finder object
   *
   * @param[in] gra
   */
  explicit NegCycleFinder(const Graph &gra)
      // : _gra{gra}, _cycle_start{std::make_unique<node_t>()} {}
      : _gra{gra} {}

  /*!
   * @brief find negative cycle
   *
   * @tparam Container
   * @tparam WeightFn
   * @param[in,out] dist
   * @param[in] get_weight
   * @return std::vector<edge_t>
   */
  template <typename Container, typename WeightFn>
  auto find_neg_cycle(Container &&dist, WeightFn &&get_weight)
      -> std::vector<edge_t> {
    this->_pred.clear();
    this->_edge.clear();

    while (this->_relax(dist, get_weight)) {
      const auto v = this->_find_cycle();
      if (v) {
        assert(this->_is_negative(*v, dist, get_weight));
        return this->_cycle_list(*v);
      }
    }
    return std::vector<edge_t>{}; // TODO
  }

private:
  /*!
   * @brief Find a cycle on policy graph
   *
   * @return node_t a start node of the cycle
   */
  auto _find_cycle() -> std::optional<node_t> {
    auto visited = std::unordered_map<node_t, node_t>{};

    for (auto &&v : this->_gra) {
      if (visited.find(v) != visited.end()) { // contains v
        continue;
      }
      auto u = v;
      while (true) {
        visited[u] = v;
        if (this->_pred.find(u) == this->_pred.end()) { // not contains u
          break;
        }
        u = this->_pred[u];
        if (visited.find(u) != visited.end()) { // contains u
          if (visited[u] == v) {
            // if (this->_is_negative(u)) {
            // should be "yield u";
            // *this->_cycle_start = u;
            // return this->_cycle_start.get();
            return u;
            // }
          }
          break;
        }
      }
    }

    return {};
  }

  /*!
   * @brief Perform one relaxation
   *
   * @tparam Container
   * @tparam WeightFn
   * @param[in,out] dist
   * @param[in] get_weight
   * @return true
   * @return false
   */
  template <typename Container, typename WeightFn>
  auto _relax(Container &&dist, WeightFn &&get_weight) -> bool {
    auto changed = false;
    for (const auto &u : this->_gra) {
      for (const auto &v : this->_gra.successors(u)) {
        // Allow self-loop
        // assert(u != v);
        const auto e = edge_t{u, v};
        const auto wt = get_weight(e);
        // assume it takes a long time
        const auto d = dist[u] + wt;
        if (dist[v] > d) {
          this->_pred[v] = u;
          this->_edge[v] = e; // TODO
          dist[v] = d;
          changed = true;
        }
      }
    }
    return changed;
  }

  /*!
   * @brief generate a cycle list
   *
   * @param[in] handle
   * @return std::vector<edge_t>
   */
  auto _cycle_list(const node_t &handle) -> std::vector<edge_t> {
    auto v = handle;
    auto cycle = std::vector<edge_t>{}; // TODO
    do {
      const auto &u = this->_pred[v];
      cycle.push_back(this->_edge[v]); // TODO
      v = u;
    } while (v != handle);
    return cycle;
  }

  /*!
   * @brief check if it is really a negative cycle
   *
   * @tparam Container
   * @tparam WeightFn
   * @param[in] handle
   * @param[in] dist
   * @param[in] get_weight
   * @return true
   * @return false
   */
  template <typename Container, typename WeightFn>
  auto _is_negative(const node_t &handle, const Container &dist,
                    WeightFn &&get_weight) -> bool {
    auto v = handle;
    do {
      const auto u = this->_pred[v];
      const auto wt = get_weight(this->_edge[v]); // TODO
      if (dist[v] > dist[u] + wt) {
        return true;
      }
      v = u;
    } while (v != handle);

    return false;
  }
};
