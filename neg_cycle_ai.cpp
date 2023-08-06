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
 * @tparam DiGraph
 */
template <typename DiGraph> class NegCycleFinder {
    using node_t = typename DiGraph::key_type;
    using edge_t = std::pair<node_t, node_t>;
    using Cycle = std::vector<edge_t>;

  private:
    const DiGraph &_digraph;                     ///< graph
    std::unordered_map<node_t, node_t> _pred{};  ///< pred

  public:
    /**
     * @brief Construct a new Neg Cycle Finder object
     *
     * @param gra
     */
    NegCycleFinder(const DiGraph &gra) : _digraph{gra} {}

    /**
     * @brief
     *
     * @return Cycle
     */
    Cycle find_cycle() {
        std::unordered_map<node_t, node_t> visited;
        Cycle cycle{};
        for (auto it = this->_digraph.cbegin(); it != this->_digraph.cend(); it++) {
            int vtx = it->first;
            if (visited.find(vtx) == visited.end()) {
                int utx = vtx;
                while (true) {
                    visited[utx] = vtx;
                    if (this->_pred.find(utx) == this->_pred.end()) {
                        break;
                    }
                    utx = this->_pred[utx];
                    if (visited.find(utx) != visited.end()) {
                        if (visited[utx] == vtx) {
                            int w = utx;
                            while (true) {
                                int x = this->_pred[w];
                                cycle.push_back(make_pair(x, w));
                                w = x;
                                if (w == utx) {
                                    break;
                                }
                            }
                            // reverse(cycle.begin(), cycle.end());
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
    bool relax(std::unordered_map<node_t, node_t> &dist, function<int(edge_t)> get_weight) {
        bool changed = false;
        for (auto it = this->_digraph.begin(); it != this->_digraph.end(); it++) {
            int utx = it->first;
            for (auto jt = it->second.begin(); jt != it->second.end(); jt++) {
                int vtx = jt->first;
                int weight = get_weight(make_pair(utx, vtx));
                int distrance = dist[utx] + weight;
                if (dist[vtx] > distrance) {
                    dist[vtx] = distrance;
                    this->_pred[vtx] = utx;
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
     * @return Cycle
     */
    Cycle find_neg_cycle(std::unordered_map<node_t, node_t> &dist,
                         function<int(edge_t)> get_weight) {
        this->_pred.clear();
        Cycle cycle;
        bool found = false;
        while (!found && relax(dist, get_weight)) {
            Cycle c = find_cycle();
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
     * @return Cycle
     */
    Cycle cycle_list(int handle) {
        int vtx = handle;
        Cycle cycle;
        while (true) {
            int utx = this->_pred[vtx];
            cycle.push_back(make_pair(utx, vtx));
            vtx = utx;
            if (vtx == handle) {
                break;
            }
        }
        // reverse(cycle.begin(), cycle.end());
        return cycle;
    }
};

int main() {
    using node_t = int;
    using edge_t = std::pair<node_t, node_t>;
    using Cycle = std::vector<edge_t>;

    std::unordered_map<int, Cycle> digraph;
    digraph[0] = {{1, 1}, {2, 4}};
    digraph[1] = {{2, 2}, {3, 5}};
    digraph[2] = {{3, 1}};
    digraph[3] = {{1, -7}};
    std::unordered_map<node_t, node_t> dist;
    for (auto it = digraph.begin(); it != digraph.end(); it++) {
        dist[it->first] = numeric_limits<int>::max();
    }
    dist[0] = 0;
    NegCycleFinder ncf(digraph);
    Cycle cycle = ncf.find_neg_cycle(dist, [](edge_t p) { return p.second; });
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
