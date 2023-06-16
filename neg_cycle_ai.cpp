#include <algorithm>
#include <iostream>
#include <limits>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

/**
 * @brief 
 * 
 */
class NegCycleFinder {
private:
  unordered_map<int, vector<pair<int, int>>> gra;
  unordered_map<int, int> pred;

public:
  /**
   * @brief Construct a new Neg Cycle Finder object
   * 
   * @param gra 
   */
  NegCycleFinder(unordered_map<int, vector<pair<int, int>>> gra) {
    this->gra = gra;
  }

  /**
   * @brief 
   * 
   * @return vector<pair<int, int>> 
   */
  vector<pair<int, int>> find_cycle() {
    unordered_map<int, int> visited;
    vector<pair<int, int>> cycle;
    for (auto it = gra.begin(); it != gra.end(); it++) {
      int v = it->first;
      if (visited.find(v) == visited.end()) {
        int u = v;
        while (true) {
          visited[u] = v;
          if (pred.find(u) == pred.end()) {
            break;
          }
          u = pred[u];
          if (visited.find(u) != visited.end()) {
            if (visited[u] == v) {
              int w = u;
              while (true) {
                int x = pred[w];
                cycle.push_back(make_pair(x, w));
                w = x;
                if (w == u) {
                  break;
                }
              }
              reverse(cycle.begin(), cycle.end());
              return cycle;
            }
            break;
          }
        }
      }
    }
    return cycle;
  }

  /**
   * @brief 
   * 
   * @param dist 
   * @param get_weight 
   * @return true 
   * @return false 
   */
  bool relax(unordered_map<int, int> &dist,
             function<int(pair<int, int>)> get_weight) {
    bool changed = false;
    for (auto it = gra.begin(); it != gra.end(); it++) {
      int u = it->first;
      for (auto jt = it->second.begin(); jt != it->second.end(); jt++) {
        int v = jt->first;
        int wt = get_weight(make_pair(u, v));
        int d = dist[u] + wt;
        if (dist[v] > d) {
          dist[v] = d;
          pred[v] = u;
          changed = true;
        }
      }
    }
    return changed;
  }

  /**
   * @brief 
   * 
   * @param dist 
   * @param get_weight 
   * @return vector<pair<int, int>> 
   */
  vector<pair<int, int>>
  find_neg_cycle(unordered_map<int, int> &dist,
                 function<int(pair<int, int>)> get_weight) {
    pred.clear();
    vector<pair<int, int>> cycle;
    bool found = false;
    while (!found && relax(dist, get_weight)) {
      vector<pair<int, int>> c = find_cycle();
      if (!c.empty()) {
        found = true;
        cycle = cycle_list(c[0].first);
      }
    }
    return cycle;
  }

  /**
   * @brief 
   * 
   * @param handle 
   * @return vector<pair<int, int>> 
   */
  vector<pair<int, int>> cycle_list(int handle) {
    int v = handle;
    vector<pair<int, int>> cycle;
    while (true) {
      int u = pred[v];
      cycle.push_back(make_pair(u, v));
      v = u;
      if (v == handle) {
        break;
      }
    }
    reverse(cycle.begin(), cycle.end());
    return cycle;
  }
};

int main() {
  unordered_map<int, vector<pair<int, int>>> gra;
  gra[0] = {{1, 1}, {2, 4}};
  gra[1] = {{2, 2}, {3, 5}};
  gra[2] = {{3, 1}};
  gra[3] = {{1, -7}};
  unordered_map<int, int> dist;
  for (auto it = gra.begin(); it != gra.end(); it++) {
    dist[it->first] = numeric_limits<int>::max();
  }
  dist[0] = 0;
  NegCycleFinder ncf(gra);
  vector<pair<int, int>> cycle =
      ncf.find_neg_cycle(dist, [](pair<int, int> p) { return p.second; });
  if (cycle.empty()) {
    cout << "No negative cycle found." << endl;
  } else {
    cout << "Negative cycle found: ";
    for (auto it = cycle.begin(); it != cycle.end(); it++) {
      cout << "(" << it->first << ", " << it->second << ") ";
    }
    cout << endl;
  }
  return 0;
}
