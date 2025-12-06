/*!
 * @file test_cases2_boost.hpp
 * @brief Alternative test case generators with edge IDs
 *
 * This module provides test case generators similar to test_cases_boost.hpp
 * but with additional edge ID properties. The edge IDs are useful for
 * algorithms that need to uniquely identify edges beyond just their
 * endpoints.
 *
 * This version uses custom edge properties to store unique identifiers,
 * which can be helpful for debugging, tracking, and algorithm implementation.
 */

#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <py2cpp/nx2bgl.hpp>
#include <utility>  // for std::pair

namespace boost {

    /// Custom edge tag for storing unique edge identifiers
    enum edge_id_tag_t { id_tag };  // a unique #
    /// Install the custom edge property
    BOOST_INSTALL_PROPERTY(edge, id_tag);

}  // namespace boost

/// Type alias for Boost adjacency list with custom edge ID properties
using graph_t
    = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::no_property,
                            boost::property<boost::edge_id_tag_t, size_t>>;
/// Type alias for vertex descriptor
using Vertex = typename boost::graph_traits<graph_t>::vertex_descriptor;
/// Type alias for edge iterator
using edge_t = typename boost::graph_traits<graph_t>::edge_iterator;

/*!
 * @brief Create a simple 5-node cycle test graph with edge IDs
 *
 * This function creates the same 5-node cycle as in test_cases_boost.hpp
 * but with additional edge ID properties. The graph structure is:
 * A -> B -> C -> D -> E -> A
 *
 * The edge IDs are automatically assigned during graph construction
 * and can be used to uniquely identify edges in algorithms.
 *
 * @tparam Mapping Type of weight mapping (edge -> weight)
 * @param[in] weights weights for the edges in order of creation
 * @return py::GraphAdaptor<graph_t> wrapped Boost graph with edge IDs
 */
template <typename Mapping> inline auto create_test_case1(const Mapping &weights)
    -> py::GraphAdaptor<graph_t> {
    using edge_t = std::pair<int, int>;
    const auto num_nodes = 5;
    enum nodes { A, B, C, D, E };
    static edge_t edge_array[]
        = {edge_t{A, B}, edge_t{B, C}, edge_t{C, D}, edge_t{D, E}, edge_t{E, A}};
    // int weights[] = {-5, 1, 1, 1, 1};
    int num_arcs = sizeof(edge_array) / sizeof(edge_t);
    auto g = graph_t(edge_array, edge_array + num_arcs, weights, num_nodes);
    return py::GraphAdaptor<graph_t>{std::move(g)};
}

/*!
 * @brief Create a 3-node timing test graph with parallel edges and edge IDs
 *
 * This function creates the same complex 3-node graph as in test_cases_boost.hpp
 * but with additional edge ID properties. The graph includes multiple parallel
 * edges and bidirectional connections.
 *
 * The edge IDs are particularly useful here since there are multiple edges
 * between the same pair of vertices, making endpoint-based identification
 * ambiguous.
 *
 * @tparam Mapping Type of weight mapping (edge -> weight)
 * @param[in] weights weights for the edges in order of creation
 * @return py::GraphAdaptor<graph_t> wrapped Boost graph with edge IDs
 */
template <typename Mapping> inline auto create_test_case_timing(const Mapping &weights)
    -> py::GraphAdaptor<graph_t> {
    using edge_t = std::pair<int, int>;
    constexpr auto num_nodes = 3;
    enum nodes { A, B, C };
    static edge_t edge_array[] = {edge_t{A, B}, edge_t{B, A}, edge_t{B, C}, edge_t{C, B},
                                  edge_t{B, C}, edge_t{C, B}, edge_t{C, A}, edge_t{A, C}};
    // int weights[] = {7, 0, 3, 1, 6, 4, 2, 5};
    constexpr int num_arcs = sizeof(edge_array) / sizeof(edge_t);
    auto g = graph_t(edge_array, edge_array + num_arcs, weights, num_nodes);
    return py::GraphAdaptor<graph_t>{std::move(g)};
}
