// -*- coding: utf-8 -*-
#include <doctest/doctest.h>  // for ResultBuilder, CHECK

#include <boost/graph/adjacency_list.hpp>         // for vecS (ptr only)
#include <boost/graph/detail/adjacency_list.hpp>  // for vertices, get, out_...
#include <boost/graph/graph_selectors.hpp>        // for directedS
#include <boost/graph/graph_traits.hpp>           // for graph_traits, graph...
#include <boost/graph/properties.hpp>             // for edge_property_tag
#include <boost/iterator/iterator_facade.hpp>     // for operator!=, operator==
#include <boost/move/utility_core.hpp>            // for move
#include <boost/pending/property.hpp>             // for no_property, property
#include <boost/property_map/property_map.hpp>    // for get, iterator_prope...
#include <cstddef>                                // for size_t
#include <netoptim/min_cycle_ratio.hpp>           // for min_cycle_ratio
#include <ostream>                                // for operator<<
#include <py2cpp/fractions.hpp>                   // for Fraction, operator-
#include <py2cpp/nx2bgl.hpp>                      // for grAdaptor, EdgeView
#include <type_traits>                            // for move
#include <utility>                                // for pair
#include <vector>                                 // for vector
// from fractions import Fraction

namespace boost {

    enum edge_id_tag_t { id_tag };  // a unique #
    BOOST_INSTALL_PROPERTY(edge, id_tag);

}  // namespace boost

using graph_t
    = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::no_property,
                            boost::property<boost::edge_id_tag_t, size_t>>;
using Vertex = typename boost::graph_traits<graph_t>::vertex_descriptor;
// using Edge_it = typename boost::graph_traits<graph_t>::edge_iterator;

static auto create_test_case1() {
    using Edge = std::pair<int, int>;

    const auto num_nodes = 5;
    enum nodes { A, B, C, D, E };
    static Edge edge_array[] = {Edge{A, B}, Edge{B, C}, Edge{C, D}, Edge{D, E}, Edge{E, A}};
    size_t indices[] = {0, 1, 2, 3, 4};
    int num_arcs = sizeof(edge_array) / sizeof(Edge);
    auto g = graph_t(edge_array, edge_array + num_arcs, indices, num_nodes);
    return py::grAdaptor<graph_t>{std::move(g)};
}

static auto create_test_case_timing() {
    using Edge = std::pair<int, int>;

    const auto num_nodes = 3;
    enum nodes { A, B, C };
    Edge edge_array[] = {Edge{A, B}, Edge{B, A}, Edge{B, C}, Edge{C, B},
                         Edge{B, C}, Edge{C, B}, Edge{C, A}, Edge{A, C}};
    size_t indices[] = {0, 1, 2, 3, 4, 5, 6, 7};
    int num_arcs = sizeof(edge_array) / sizeof(Edge);

    auto g = graph_t(edge_array, edge_array + num_arcs, indices, num_nodes);
    return py::grAdaptor<graph_t>{std::move(g)};
}

TEST_CASE("Test Parametric (boost)") {
    using EdgeIndexMap = typename boost::property_map<graph_t, boost::edge_id_tag_t>::type;
    using IterMap = boost::iterator_property_map<int*, EdgeIndexMap, int, int&>;

    auto G = create_test_case1();
    int cost[] = {5, 1, 1, 1, 1};
    EdgeIndexMap edge_id = boost::get(boost::id_tag, G);
    IterMap cost_pa(cost, edge_id);

    auto get_cost = [&](const auto& e) -> int { return boost::get(cost_pa, e); };
    auto get_time = [&](const auto&) -> int { return 1; };

    auto dist = std::vector<fun::Fraction<int>>(G.number_of_nodes(), fun::Fraction<int>(0));
    auto r = fun::Fraction<int>(5);
    const auto c = min_cycle_ratio(G, r, get_cost, get_time, dist);
    CHECK(!c.empty());
    CHECK(c.size() == 5);
    CHECK(r == fun::Fraction<int>(9, 5));
    // print(r);
    // print(c);
    // print(dist.items());
}

TEST_CASE("Test Parametric of Timing Graph (boost)") {
    using EdgeIndexMap = typename boost::property_map<graph_t, boost::edge_id_tag_t>::type;
    using IterMap = boost::iterator_property_map<int*, EdgeIndexMap, int, int&>;

    auto G = create_test_case_timing();
    int cost[] = {7, -1, 5, 4, 3, 0, 2, 4};
    EdgeIndexMap edge_id = boost::get(boost::id_tag, G);
    IterMap cost_pa(cost, edge_id);

    const auto get_cost = [&](const auto& e) -> int { return boost::get(cost_pa, e); };
    const auto get_time = [&](const auto& /*e*/) -> int { return 1; };

    auto dist = std::vector<fun::Fraction<int>>(G.number_of_nodes(), fun::Fraction<int>(0));
    auto r = fun::Fraction<int>(7);
    const auto c = min_cycle_ratio(G, r, get_cost, get_time, dist);
    CHECK(!c.empty());
    CHECK(r == fun::Fraction<int>(1, 1));
    CHECK(c.size() == 3);
    // print(r);
    // print(c);
    // print(dist.items());
}
