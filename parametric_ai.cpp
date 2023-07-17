#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

template <typename R, typename V>
using Graph = unordered_map<V, unordered_map<V, R>>;

template <typename R> class NegCycleFinder {
public:
  NegCycleFinder(const Graph<R, int> &graph) : graph_(graph) {}

  vector<int> find_neg_cycle(unordered_map<int, R> &dist,
                             function<R(pair<int, int>)> get_weight) {
    int n = graph_.size();
    vector<int> prev(n, -1);
    vector<int> cycle;
    int last = -1;
    for (int i = 0; i < n; ++i) {
      for (auto &[u, edges] : graph_) {
        for (auto &[v, _] : edges) {
          if (dist[u] + get_weight({u, v}) < dist[v]) {
            dist[v] = dist[u] + get_weight({u, v});
            prev[v] = u;
            last = v;
          }
        }
      }
    }
    if (last != -1) {
      for (int i = 0; i < n; ++i) {
        last = prev[last];
      }
      for (int v = last;; v = prev[v]) {
        cycle.push_back(v);
        if (v == last && cycle.size() > 1) {
          break;
        }
      }
      reverse(cycle.begin(), cycle.end());
    }
    return cycle;
  }

private:
  const Graph<R, int> &graph_;
};

/**
 * @brief
 *
 * @tparam R
 */
template <typename R> class Omega {
public:
  /**
   * @brief
   *
   * @param ratio
   * @param edge
   * @return R
   */
  R distance(R ratio, pair<int, int> edge) const {
    return log(ratio * exp(weights_[edge]) +
               (1 - ratio) * exp(-weights_[edge]));
  }

  /**
   * @brief
   *
   * @param cycle
   * @return R
   */
  R zero_cancel(const vector<int> &cycle) const {
    R sum = 0;
    for (int i = 0; i < cycle.size(); ++i) {
      sum += weights_.at({cycle[i], cycle[(i + 1) % cycle.size()]});
    }
    return exp(sum);
  }

  /**
   * @brief Set the weight object
   *
   * @param edge
   * @param weight
   */
  void set_weight(pair<int, int> edge, R weight) { weights_[edge] = weight; }

private:
  unordered_map<pair<int, int>, R> weights_; ///< weights
};

template <typename R, typename V>
using MutableGraph = unordered_map<V, unordered_map<V, R>>;

template <typename R, typename V>
using Mapping = unordered_map<V, unordered_map<V, R>>;

template <typename R, typename V>
using MutableMapping = unordered_map<V, unordered_map<V, R>>;

template <typename R, typename V> using Cycle = vector<pair<V, V>>;

/**
 * @brief
 *
 * @tparam R
 * @tparam V
 * @param gra
 * @param ratio
 * @param omega
 * @param dist
 * @return R
 */
template <typename R, typename V>
R max_parametric(const Mapping<R, V> &gra, R ratio, const Omega<R> &omega,
                 MutableMapping<R, V> &dist) {
  NegCycleFinder<R> ncf(gra);
  R r_min = ratio;
  Cycle<R, V> c_min;
  Cycle<R, V> cycle;
  while (true) {
    for (auto &ci : ncf.find_neg_cycle(dist, [&](pair<V, V> edge) {
           return omega.distance(ratio, edge);
         })) {
      R ri = omega.zero_cancel(ci);
      if (r_min > ri) {
        r_min = ri;
        c_min = ci;
      }
    }
    if (r_min >= ratio) {
      break;
    }
    cycle = c_min;
    ratio = r_min;
  }
  return ratio;
}
