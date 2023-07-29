#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <typename V, typename D> class NegCycleFinder {
  private:
    std::unordered_map<V, V> pred;
    std::unordered_map<V, std::unordered_map<V, D>> digraph;

  public:
    NegCycleFinder(std::unordered_map<V, std::unordered_map<V, D>> gra) {
        digraph = gra;
    }

    std::vector<V> FindCycle() {
        std::unordered_map<V, V> visited;
        std::vector<V> cycle;

        for (auto const &[v, _] : digraph) {
            if (visited.find(v) == visited.end()) {
                V u = v;
                while (true) {
                    visited[u] = v;
                    if (pred.find(u) == pred.end()) {
                        break;
                    }
                    u = pred[u];
                    if (visited.find(u) != visited.end()) {
                        if (visited[u] == v) {
                            cycle.push_back(u);
                        }
                        break;
                    }
                }
            }
        }
        return cycle;
    }
};
