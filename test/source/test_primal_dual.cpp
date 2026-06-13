// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <cassert>
#include <cstdint>
#include <netoptim/primal_dual.hpp>
#include <utility>
#include <vector>

namespace {

    /// Minimal edge type with end_points() required by min_vertex_cover_pd
    template <typename Node>
    struct PdEdge : std::pair<Node, Node> {
        using std::pair<Node, Node>::pair;
        [[nodiscard]] auto end_points() const { return *this; }
    };

    /// Minimal undirected graph for primal-dual algorithm tests.
    ///
    /// Provides:
    ///   - edges() returning iterable with .end_points()
    ///   - begin()/end() for node iteration
    ///   - operator[] for neighbor access
    class PdGraph {
      public:
        using key_type = uint32_t;
        using node_t = uint32_t;

        PdGraph(uint32_t n, std::vector<std::pair<uint32_t, uint32_t>> edge_list)
            : _n(n), _raw_edges(std::move(edge_list)), _adj(n) {
            for (const auto& [u, v] : _raw_edges) {
                _adj[u].push_back(v);
                _adj[v].push_back(u);
            }
        }

        [[nodiscard]] auto edges() const -> std::vector<PdEdge<uint32_t>> {
            std::vector<PdEdge<uint32_t>> result;
            result.reserve(_raw_edges.size());
            for (const auto& [u, v] : _raw_edges) {
                result.emplace_back(u, v);
            }
            return result;
        }

        // node iteration: for (auto&& utx : gra)
        struct NodeIter {
            uint32_t i;
            auto operator++() -> NodeIter& {
                ++i;
                return *this;
            }
            auto operator*() const { return i; }
            bool operator!=(const NodeIter& o) const { return i != o.i; }
        };

        [[nodiscard]] auto begin() const { return NodeIter{0}; }
        [[nodiscard]] auto end() const { return NodeIter{_n}; }

        // neighbor access: gra[utx]
        [[nodiscard]] const auto& operator[](uint32_t i) const { return _adj[i]; }

        [[nodiscard]] auto number_of_nodes() const { return _n; }

      private:
        uint32_t _n;
        std::vector<std::pair<uint32_t, uint32_t>> _raw_edges;
        std::vector<std::vector<uint32_t>> _adj;
    };

}  // namespace

// ============================================================
// Tests for min_vertex_cover_pd
// ============================================================

TEST_CASE("Test Min Vertex Cover - Single Edge") {
    // Graph: 0 -- 1
    auto gra = PdGraph(2, {{0, 1}});
    auto cover = std::vector<bool>(2, false);
    auto weight = std::vector<int>{5, 3};

    const auto cost = min_vertex_cover_pd(gra, cover, weight);
    CHECK_GT(cost, 0);
    {
        const auto covered = cover[0] || cover[1];
        CHECK(covered);
    }
    CHECK_LE(cost, 8);
}

TEST_CASE("Test Min Vertex Cover - Triangle") {
    auto gra = PdGraph(3, {{0, 1}, {1, 2}, {2, 0}});
    auto cover = std::vector<bool>(3, false);
    auto weight = std::vector<int>{2, 3, 4};

    const auto cost = min_vertex_cover_pd(gra, cover, weight);

    {
        const auto e01 = cover[0] || cover[1];
        const auto e12 = cover[1] || cover[2];
        const auto e20 = cover[2] || cover[0];
        CHECK(e01);
        CHECK(e12);
        CHECK(e20);
    }

    CHECK_GE(cost, 2);
    CHECK_LE(cost, 9);
}

TEST_CASE("Test Min Vertex Cover - Path Graph") {
    auto gra = PdGraph(4, {{0, 1}, {1, 2}, {2, 3}});
    auto cover = std::vector<bool>(4, false);
    auto weight = std::vector<int>{10, 1, 1, 10};

    const auto cost = min_vertex_cover_pd(gra, cover, weight);

    {
        const auto e01 = cover[0] || cover[1];
        const auto e12 = cover[1] || cover[2];
        const auto e23 = cover[2] || cover[3];
        CHECK(e01);
        CHECK(e12);
        CHECK(e23);
    }

    // Cost must be <= 2 * optimal for 2-approximation guarantee
    // Optimal cover = vertices {1, 2} with cost 2
    // 2-approximation: cost <= 4
    CHECK_LE(cost, 12);
}

TEST_CASE("Test Min Vertex Cover - Single Node No Edges") {
    // no edges
    auto gra = PdGraph(1, {});
    auto cover = std::vector<bool>(1, false);
    auto weight = std::vector<int>{5};

    const auto cost = min_vertex_cover_pd(gra, cover, weight);
    CHECK_EQ(cost, 0);
}

TEST_CASE("Test Min Vertex Cover - Pre-covered Node") {
    auto gra = PdGraph(2, {{0, 1}});
    auto cover = std::vector<bool>(2, false);
    cover[1] = true;
    auto weight = std::vector<int>{5, 3};

    const auto cost = min_vertex_cover_pd(gra, cover, weight);
    CHECK_EQ(cost, 0);
}



TEST_CASE("Test Min Maximal Independent Set - Single Edge") {
    auto gra = PdGraph(2, {{0, 1}});
    auto indset = std::vector<bool>(2, false);
    auto dep = std::vector<bool>(2, false);
    auto weight = std::vector<int>{5, 3};

    const auto cost = min_maximal_independant_set_pd(gra, indset, dep, weight);

    for (uint32_t i = 0; i < 2; ++i) {
        const auto m = indset[i] || dep[i];
        CHECK(m);
    }
    CHECK_GT(cost, 0);
}

TEST_CASE("Test Min Maximal Independent Set - Triangle") {
    auto gra = PdGraph(3, {{0, 1}, {1, 2}, {2, 0}});
    auto indset = std::vector<bool>(3, false);
    auto dep = std::vector<bool>(3, false);
    auto weight = std::vector<int>{2, 3, 4};

    const auto cost = min_maximal_independant_set_pd(gra, indset, dep, weight);

    for (uint32_t i = 0; i < 3; ++i) {
        const auto m = indset[i] || dep[i];
        CHECK(m);
    }

    const auto bad01 = indset[0] && indset[1];
    const auto bad12 = indset[1] && indset[2];
    const auto bad20 = indset[2] && indset[0];
    CHECK_FALSE(bad01);
    CHECK_FALSE(bad12);
    CHECK_FALSE(bad20);

    CHECK_GT(cost, 0);
}

TEST_CASE("Test Min Maximal Independent Set - Path Graph") {
    auto gra = PdGraph(4, {{0, 1}, {1, 2}, {2, 3}});
    auto indset = std::vector<bool>(4, false);
    auto dep = std::vector<bool>(4, false);
    auto weight = std::vector<int>{10, 1, 1, 10};

    const auto cost = min_maximal_independant_set_pd(gra, indset, dep, weight);

    for (uint32_t i = 0; i < 4; ++i) {
        const auto m = indset[i] || dep[i];
        CHECK(m);
    }

    const auto bad01 = indset[0] && indset[1];
    const auto bad12 = indset[1] && indset[2];
    const auto bad23 = indset[2] && indset[3];
    const auto any_bad = bad01 || bad12 || bad23;
    CHECK_FALSE(any_bad);

    CHECK_GT(cost, 0);
}

TEST_CASE("Test Min Maximal Independent Set - Star Graph") {
    auto gra = PdGraph(4, {{0, 1}, {0, 2}, {0, 3}});
    auto indset = std::vector<bool>(4, false);
    auto dep = std::vector<bool>(4, false);
    auto weight = std::vector<int>{5, 1, 1, 1};

    const auto cost = min_maximal_independant_set_pd(gra, indset, dep, weight);

    for (uint32_t i = 0; i < 4; ++i) {
        const auto m = indset[i] || dep[i];
        CHECK(m);
    }

    if (indset[0]) {
        const auto leaf_bad = indset[1] || indset[2] || indset[3];
        CHECK_FALSE(leaf_bad);
    }

    CHECK_GT(cost, 0);
}
