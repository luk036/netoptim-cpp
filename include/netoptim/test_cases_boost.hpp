#pragma once

// -*- coding: utf-8 -*-
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <py2cpp/nx2bgl.hpp>
#include <utility> // for std::pair

using graph_t = boost::adjacency_list<boost::listS, boost::vecS,
    boost::directedS, boost::no_property,
    boost::property<boost::edge_weight_t, int,
        boost::property<boost::edge_index_t, int>>>;
using Vertex = boost::graph_traits<graph_t>::vertex_descriptor;
using Edge_it = boost::graph_traits<graph_t>::edge_iterator;

template <typename Container>
inline auto create_test_case1(const Container& weights) -> xn::grAdaptor<graph_t>
{
    using Edge = std::pair<int, int>;
    const auto num_nodes = 5;
    enum nodes
    {
        A,
        B,
        C,
        D,
        E
    };
    static Edge edge_array[] = {
        Edge {A, B}, Edge {B, C}, Edge {C, D}, Edge {D, E}, Edge {E, A}};
    // int weights[] = {-5, 1, 1, 1, 1};
    int num_arcs = sizeof(edge_array) / sizeof(Edge);
    auto g = graph_t(edge_array, edge_array + num_arcs, weights, num_nodes);
    return xn::grAdaptor<graph_t> {std::move(g)};
}

template <typename Container>
inline auto create_test_case_timing(const Container& weights)
    -> xn::grAdaptor<graph_t>
{
    using Edge = std::pair<int, int>;
    constexpr auto num_nodes = 3;
    enum nodes
    {
        A,
        B,
        C
    };
    static Edge edge_array[] = {Edge {A, B}, Edge {B, A}, Edge {B, C},
        Edge {C, B}, Edge {B, C}, Edge {C, B}, Edge {C, A}, Edge {A, C}};
    // int weights[] = {7, 0, 3, 1, 6, 4, 2, 5};
    constexpr int num_arcs = sizeof(edge_array) / sizeof(Edge);
    auto g = graph_t(edge_array, edge_array + num_arcs, weights, num_nodes);
    return xn::grAdaptor<graph_t> {std::move(g)};
}
