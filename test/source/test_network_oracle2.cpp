// -*- coding: utf-8 -*-
#include <doctest/doctest.h>

#include <list>
#include <map>
#include <memory>
#include <netoptim/network_oracle.hpp>
#include <unordered_map>
#include <utility>

namespace {

    class MockOracle {
      public:
        struct State {
            double t = 0.0;
        };
        std::shared_ptr<State> state;
        std::map<std::pair<uint32_t, uint32_t>, double> values;
        std::map<std::pair<uint32_t, uint32_t>, double> grads;

        MockOracle() : state{std::make_shared<State>()} {}

        auto eval(const std::pair<uint32_t, uint32_t>& edge, double /*x*/) const -> double {
            auto it = values.find(edge);
            return it != values.end() ? it->second : 0.0;
        }

        auto grad(const std::pair<uint32_t, uint32_t>& edge, double /*x*/) const -> double {
            auto it = grads.find(edge);
            return it != grads.end() ? it->second : 0.0;
        }

        void update(double gamma) { state->t = gamma; }
    };

    using TestGraph
        = std::unordered_map<uint32_t,
                             std::list<std::pair<uint32_t, std::pair<uint32_t, uint32_t>>>>;

    auto create_cycle_graph() -> TestGraph {
        return {
            {0, {{{1, {0, 1}}, {2, {0, 2}}}}},
            {1, {{{2, {1, 2}}}}},
            {2, {{{0, {2, 0}}}}},
        };
    }

}  // namespace

TEST_CASE("Test NetworkOracle operator() with negative cycle") {
    auto gra = create_cycle_graph();

    MockOracle oracle;
    oracle.values = {{{0, 1}, 1.0}, {{1, 2}, 1.0}, {{2, 0}, -3.0}};
    oracle.grads = {{{0, 1}, 1.0}, {{1, 2}, 1.0}, {{2, 0}, -1.0}};

    std::unordered_map<uint32_t, double> dist{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto network = NetworkOracle(gra, dist, oracle);

    const auto xval = 0.0;
    const auto cut = network(xval);
    CHECK(cut.has_value());

    const auto& [g, f] = *cut;
    CHECK_EQ(f, doctest::Approx(3.0));
    CHECK_EQ(g, doctest::Approx(1.0));
}

TEST_CASE("Test NetworkOracle operator() no negative cycle") {
    auto gra = create_cycle_graph();

    MockOracle oracle;
    oracle.values = {{{0, 1}, 1.0}, {{1, 2}, 1.0}, {{2, 0}, 1.0}};
    oracle.grads = {{{0, 1}, 1.0}, {{1, 2}, 1.0}, {{2, 0}, 1.0}};

    std::unordered_map<uint32_t, double> dist{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto network = NetworkOracle(gra, dist, oracle);

    const auto xval = 0.0;
    const auto cut = network(xval);
    CHECK_FALSE(cut.has_value());
}

TEST_CASE("Test NetworkOracle update then assess") {
    auto gra = create_cycle_graph();

    MockOracle oracle;
    oracle.values = {{{0, 1}, 1.0}, {{1, 2}, 1.0}, {{2, 0}, -3.0}};

    std::unordered_map<uint32_t, double> dist{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto network = NetworkOracle(gra, dist, oracle);

    network.update(2.0);
    CHECK_EQ(oracle.state->t, doctest::Approx(2.0));

    const auto xval = 0.0;
    const auto cut = network.assess_feas(xval);
    CHECK(cut.has_value());
}
