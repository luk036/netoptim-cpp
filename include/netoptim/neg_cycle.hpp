// -*- coing: utf-8 -*-
#pragma once

/*!
 * @file neg_cycle.hpp
 * @brief Negative cycle detection for weighted directed graphs
 *
 * This module provides an efficient algorithm for detecting negative cycles
 * in weighted directed graphs. It implements a cycle detection method that
 * is superior to Bellman-Ford for this specific purpose.
 *
 * The algorithm works by:
 * 1. Maintaining a predecessor map to track paths
 * 2. Performing edge relaxations iteratively
 * 3. Detecting cycles in the predecessor graph
 * 4. Verifying that detected cycles are indeed negative
 *
 * @tparam DiGraph Type of the directed graph
 */
// #include <ThreadPool.h>

#include <cassert>
#include <optional>
#include <unordered_map>
#include <utility>  // for pair
#include <vector>

/*!
 * @brief Negative cycle finder for weighted directed graphs
 *
 * This class implements an efficient algorithm for detecting negative cycles
 * in weighted directed graphs. Unlike Bellman-Ford algorithm, this approach:
 * - Does not require a source node
 * - Can detect negative cycles during the relaxation process
 * - Maintains distance information across iterations
 * - Provides the actual negative cycle path
 *
 * The algorithm is particularly useful in network optimization problems
 * where negative cycle detection is a core operation.
 *
 * @tparam DiGraph Type of the directed graph, must provide:
 *                 - key_type for vertex identification
 *                 - begin()/end() for vertex iteration
 *                 - at(vertex) for adjacency list access
 */
template <typename DiGraph>
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
     * @brief Find a negative cycle in the graph
     *
     * This is the main method that searches for negative cycles. It performs
     * edge relaxations repeatedly and checks for cycles in the predecessor
     * graph after each relaxation phase.
     *
     * The algorithm continues until either:
     * - No more relaxations are possible (no negative cycles)
     * - A negative cycle is found and returned
     *
     * @tparam Mapping Type of distance mapping (vertex -> distance)
     * @tparam Callable Type of weight function (edge -> weight)
     * @param[in,out] dist Distance mapping that gets updated during relaxation
     * @param[in] get_weight Function to get edge weights
     * @return Cycle A vector of edges forming the negative cycle, empty if none found
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
        return Cycle{};  // TODO
    }

  private:
    /*!
     * @brief Find a cycle in the predecessor graph
     *
     * This method searches for cycles in the predecessor map that represents
     * the current relaxation policy. It uses a visited map to detect when
     * we encounter a vertex that's already part of the current search path.
     *
     * @return std::optional<node_t> A vertex that's part of a cycle, empty if no cycle
     */
    auto _find_cycle() -> std::optional<node_t> {
        auto visited = std::unordered_map<node_t, node_t>{};
        for (auto &&vtx : this->_digraph) {
            if (visited.find(vtx) != visited.end()) {  // contains vtx
                continue;
            }
            auto utx = vtx;
            while (true) {
                visited[utx] = vtx;
                if (this->_pred.find(utx) == this->_pred.end()) {  // not contains utx
                    break;
                }
                utx = this->_pred[utx];
                if (visited.find(utx) != visited.end()) {  // contains utx
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
     * @brief Perform one iteration of edge relaxation
     *
     * This method attempts to improve distances by relaxing all edges in the graph.
     * For each edge (u,v), it checks if dist[v] > dist[u] + weight(u,v) and updates
     * the distance and predecessor if true.
     *
     * @tparam Mapping Type of distance mapping
     * @tparam Callable Type of weight function
     * @param[in,out] dist Distance mapping to be updated
     * @param[in] get_weight Function to get edge weights
     * @return true if any distance was updated, false otherwise
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
     * @brief Extract the cycle edges starting from a given vertex
     *
     * This method reconstructs the complete cycle by following the predecessor
     * map starting from the given handle vertex until it returns to the start.
     *
     * @param[in] handle A vertex that is part of the cycle
     * @return Cycle A vector of edges forming the complete cycle
     */
    auto _cycle_list(const node_t &handle) -> Cycle {
        auto vtx = handle;
        auto cycle = Cycle{};  // TODO
        do {
            const auto &utx = this->_pred[vtx];
            cycle.push_back(edge_t{utx, vtx});
            vtx = utx;
        } while (vtx != handle);
        return cycle;
    }

    /*!
     * @brief Verify that the detected cycle is indeed negative
     *
     * This method checks if the sum of edge weights in the cycle is negative
     * by verifying the distance property for each edge in the cycle.
     *
     * @tparam Mapping Type of distance mapping
     * @tparam Callable Type of weight function
     * @param[in] handle A vertex that is part of the cycle
     * @param[in] dist Distance mapping for verification
     * @param[in] get_weight Function to get edge weights
     * @return true if the cycle is negative, false otherwise
     */
    template <typename Mapping, typename Callable>
    auto _is_negative(const node_t &handle, const Mapping &dist, Callable &&get_weight) const
        -> bool {
        auto vtx = handle;
        do {
            const auto utx = this->_pred.at(vtx);
            const auto weight = get_weight(edge_t{utx, vtx});  // TODO
            if (dist.at(vtx) > dist.at(utx) + weight) {
                return true;
            }
            vtx = utx;
        } while (vtx != handle);

        return false;
    }
};
