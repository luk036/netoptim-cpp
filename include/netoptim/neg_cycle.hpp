// -*- coing: utf-8 -*-
#pragma once

/*!
Negative cycle detection for weighed graphs.
**/
// #include <ThreadPool.h>

#include <cassert>
#include <optional>
#include <unordered_map>
#include <utility> // for pair
#include <vector>

/*!
 * @brief negative cycle
 *
 * @tparam DiGraph
 *
 * Note: Bellman-Ford's shortest-path algorithm (BF) is NOT the best way to
 *       detect negative cycles, because
 *
 *  1. BF needs a source node.
 *  2. BF detect whether there is a negative cycle at the fianl stage.
 *  3. BF restarts the solution (dist[utx]) every time.
 */
template <typename DiGraph> //
class NegCycleFinder {
  using node_t = typename DiGraph::key_type;
  using edge_t = std::pair<node_t, node_t>;
  using Cycle = std::vector<edge_t>;

private:
  std::unordered_map<node_t, node_t> _pred{};
  const DiGraph &_digraph;

public:
  /*!
   * @brief Construct a new neg Cycle Finder object
   *
   * @param[in] gra
   */
  explicit NegCycleFinder(const DiGraph &gra) : _digraph{gra} {}

  /*!
   * @brief find negative cycle
   *
   * @tparam Mapping
   * @tparam Callable
   * @param[in,out] dist
   * @param[in] get_weight
   * @return Cycle
   */
  template <typename Mapping, typename Callable>
  auto find_neg_cycle(Mapping &&dist, Callable &&get_weight) -> Cycle {
    this->_pred.clear();
    while (this->_relax(dist, get_weight)) {
      const auto vtx = this->_find_cycle();
      if (vtx) {
        assert(this->_is_negative(*vtx, dist, get_weight));
        return this->_cycle_list(*vtx);
      }
    }
    return Cycle{}; // TODO
  }

private:
  /*!
   * @brief Find a cycle on policy graph
   *
   * @return node_t a start node of the cycle
   */
  auto _find_cycle() -> std::optional<node_t> {
    auto visited = std::unordered_map<node_t, node_t>{};
    for (auto &&vtx : this->_digraph) {
      if (visited.find(vtx) != visited.end()) { // contains vtx
        continue;
      }
      auto utx = vtx;
      while (true) {
        visited[utx] = vtx;
        if (this->_pred.find(utx) == this->_pred.end()) { // not contains utx
          break;
        }
        utx = this->_pred[utx];
        if (visited.find(utx) != visited.end()) { // contains utx
          if (visited[utx] == vtx) {
            // should be "yield utx";
            // *this->_cycle_start = utx;
            // return this->_cycle_start.get();
            return utx;
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
   * @tparam Mapping
   * @tparam Callable
   * @param[in,out] dist
   * @param[in] get_weight
   * @return true
   * @return false
   */
  template <typename Mapping, typename Callable>
  auto _relax(Mapping &&dist, Callable &&get_weight) -> bool {
    auto changed = false;
    for (const auto &utx : this->_digraph) {
      for (const auto &vtx : this->_digraph.at(utx)) {
        // Allow self-loop
        const auto weight = get_weight(edge_t{utx, vtx});
        const auto distance = dist[utx] + weight;
        if (dist[vtx] > distance) {
          this->_pred[vtx] = utx;
          dist[vtx] = distance;
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
   * @return Cycle
   */
  auto _cycle_list(const node_t &handle) -> Cycle {
    auto vtx = handle;
    auto cycle = Cycle{}; // TODO
    do {
      const auto &utx = this->_pred[vtx];
      cycle.push_back(edge_t{utx, vtx});
      vtx = utx;
    } while (vtx != handle);
    return cycle;
  }

  /*!
   * @brief check if it is really a negative cycle
   *
   * @tparam Mapping
   * @tparam Callable
   * @param[in] handle
   * @param[in] dist
   * @param[in] get_weight
   * @return true
   * @return false
   */
  template <typename Mapping, typename Callable>
  auto _is_negative(const node_t &handle, const Mapping &dist,
                    Callable &&get_weight) const -> bool {
    auto vtx = handle;
    do {
      const auto utx = this->_pred.at(vtx);
      const auto weight = get_weight(edge_t{utx, vtx}); // TODO
      if (dist.at(vtx) > dist.at(utx) + weight) {
        return true;
      }
      vtx = utx;
    } while (vtx != handle);

    return false;
  }
};
