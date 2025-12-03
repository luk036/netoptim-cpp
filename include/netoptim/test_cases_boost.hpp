/*!
 * @file test_cases_boost.hpp
 * @brief Test case generators using Boost Graph Library
 * 
 * This module provides factory functions for creating test graphs
 * using the Boost Graph Library (BGL) adjacency list implementation.
 * These test cases are designed for testing network optimization algorithms.
 * 
 * The graphs created are wrapped in py::GraphAdaptor for compatibility
 * with the network optimization algorithms.
 */

#pragma once

// -*- coding: utf-8 -*-
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <py2cpp/nx2bgl.hpp>
#include <utility>  // for std::pair

/// Type alias for Boost adjacency list with edge weights and indices
using graph_t = boost::adjacency_list<
    boost::listS, boost::vecS, boost::directedS, boost::no_property,
    boost::property<boost::edge_weight_t, int, boost::property<boost::edge_index_t, int>>>;
/// Type alias for vertex descriptor
using Vertex = boost::graph_traits<graph_t>::vertex_descriptor;
/// Type alias for edge iterator
using Edge_it = boost::graph_traits<graph_t>::edge_iterator;

/*!
 * @brief Create a simple 5-node cycle test graph
 * 
 * This function creates a simple directed cycle with 5 vertices.
 * The graph structure is: A -> B -> C -> D -> E -> A
 * This is useful for testing basic cycle detection and ratio algorithms.
 * 
 * @tparam Mapping Type of weight mapping (edge -> weight)
 * @param[in] weights weights for the edges in order of creation
 * @return py::GraphAdaptor<graph_t> wrapped Boost graph
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
 * @brief Create a 3-node timing test graph with parallel edges
 * 
 * This function creates a more complex graph with 3 vertices and multiple
 * parallel edges between each pair. This is useful for testing algorithms
 * that need to handle graphs with parallel edges and more complex cycle
 * structures.
 * 
 * The graph includes:
 * - Bidirectional edges between A and B
 * - Multiple parallel edges between B and C
 * - Bidirectional edges between C and A
 * 
 * @tparam Mapping Type of weight mapping (edge -> weight)
 * @param[in] weights weights for the edges in order of creation
 * @return py::GraphAdaptor<graph_t> wrapped Boost graph
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
