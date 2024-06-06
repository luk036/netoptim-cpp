#pragma once

// -*- coding: utf-8 -*-
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <py2cpp/nx2bgl.hpp>
#include <utility>  // for std::pair

using graph_t = boost::adjacency_list<
    boost::listS, boost::vecS, boost::directedS, boost::no_property,
    boost::property<boost::edge_weight_t, int, boost::property<boost::edge_index_t, int>>>;
using Vertex = boost::graph_traits<graph_t>::vertex_descriptor;
using Edge_it = boost::graph_traits<graph_t>::edge_iterator;

template <typename Mapping>
inline auto create_test_case1(const Mapping &weights) -> py::GraphAdaptor<graph_t> {
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

template <typename Mapping>
inline auto create_test_case_timing(const Mapping &weights) -> py::GraphAdaptor<graph_t> {
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
