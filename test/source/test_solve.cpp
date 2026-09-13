// -*- coding: utf-8 -*-
#include <absl/container/flat_hash_map.h>
#include <doctest/doctest.h>

#include <cmath>
#include <cstdint>
#include <ellalgo/ell.hpp>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <netoptim/network_oracle.hpp>
#include <netoptim/optscaling_oracle.hpp>
#include <netoptim/solve.hpp>
#include <numbers>
#include <tuple>
#include <utility>
#include <valarray>

namespace {

    using TestEdge = std::pair<uint32_t, uint32_t>;
    using TestGraph = absl::flat_hash_map<uint32_t, std::list<std::pair<uint32_t, TestEdge>>>;
    using CostGraph
        = absl::flat_hash_map<uint32_t, std::list<std::pair<uint32_t, std::pair<double, double>>>>;

    auto create_cycle_graph() -> TestGraph {
        return {
            {0, {{{1, {0, 1}}, {2, {0, 2}}}}},
            {1, {{{2, {1, 2}}}}},
            {2, {{{0, {2, 0}}}}},
        };
    }

    // Mirrors py/netoptim/tests/test_optscaling.py::create_fixed_graph
    auto create_fixed_graph() -> CostGraph {
        const auto log10 = std::numbers::ln10;
        const auto log11 = std::log(11.0);
        const auto log12 = std::log(12.0);
        const auto log13 = std::log(13.0);
        const auto log14 = std::log(14.0);
        const auto log15 = std::log(15.0);
        const auto log16 = std::log(16.0);
        const auto log17 = std::log(17.0);
        const auto log18 = std::log(18.0);
        const auto log19 = std::log(19.0);
        const auto log20 = std::log(20.0);
        const auto log21 = std::log(21.0);
        const auto log22 = std::log(22.0);
        const auto log23 = std::log(23.0);
        const auto log24 = std::log(24.0);
        const auto log125 = std::log(125.0);

        return {
            {0, {{{2, {log22, log125}}, {3, {log16, log18}}, {4, {log15, log11}}}}},
            {1,
             {{{1, {log10, log10}},
               {2, {log20, log19}},
               {3, {log14, log12}},
               {4, {100.0, log21}}}}},
            {2, {{{0, {log125, log22}}, {1, {log19, log20}}, {2, {log13, log13}}}}},
            {3, {{{0, {log18, log16}}, {1, {log12, log14}}, {4, {log24, log23}}}}},
            {4,
             {{{0, {log11, log15}},
               {1, {log21, -100.0}},
               {3, {log23, log24}},
               {4, {log17, log17}}}}},
        };
    }

    auto get_fixed_cost
        = [](const std::pair<double, double>& edge_data) -> std::pair<double, double> {
        return edge_data;
    };

    class MockOracle {
      public:
        std::map<TestEdge, double> values;
        std::map<TestEdge, double> grads;

        template <typename Arr> auto eval(const TestEdge& edge, const Arr&) const -> double {
            auto it = values.find(edge);
            return it != values.end() ? it->second : 0.0;
        }

        template <typename Arr> auto grad(const TestEdge& edge, const Arr&) const -> double {
            auto it = grads.find(edge);
            return it != grads.end() ? it->second : 0.0;
        }

        void update(double /*gamma*/) {}
    };

    class HookOracle : public MockOracle {
      public:
        std::shared_ptr<int> prepare_calls = std::make_shared<int>(0);

        template <typename Arr> auto make_weight_fn(const Arr&) const {
            ++(*prepare_calls);
            const auto& vals = values;
            return [&vals](const TestEdge& edge) -> double {
                auto it = vals.find(edge);
                return it != vals.end() ? it->second : 0.0;
            };
        }
    };

}  // namespace

TEST_CASE("NetworkOracle uses the make_weight_fn hook") {
    auto gra = create_cycle_graph();

    MockOracle plain;
    plain.values = {{{0, 1}, 1.0}, {{1, 2}, 1.0}, {{2, 0}, -3.0}};
    plain.grads = {{{0, 1}, 1.0}, {{1, 2}, 1.0}, {{2, 0}, -1.0}};
    absl::flat_hash_map<uint32_t, double> dist_plain{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto net_plain = NetworkOracle(gra, dist_plain, plain);
    const auto cut_plain = net_plain.assess_feas(0.0);

    HookOracle hooked;
    hooked.values = plain.values;
    hooked.grads = plain.grads;
    absl::flat_hash_map<uint32_t, double> dist_hooked{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto net_hooked = NetworkOracle(gra, dist_hooked, hooked);
    const auto cut_hooked = net_hooked.assess_feas(0.0);

    CHECK_EQ(*hooked.prepare_calls, 1);
    REQUIRE(cut_plain.has_value());
    REQUIRE(cut_hooked.has_value());
    CHECK_EQ(cut_hooked->first, doctest::Approx(cut_plain->first));
    CHECK_EQ(cut_hooked->second, doctest::Approx(cut_plain->second));
}

TEST_CASE("Ratio make_weight_fn matches eval") {
    auto gra = create_fixed_graph();
    auto ratio = netoptim_detail::Ratio<CostGraph, decltype(get_fixed_cost)>{gra, get_fixed_cost};
    const auto x = std::valarray<double>{1.25, -0.5};
    const auto weight = ratio.make_weight_fn(x);

    auto checked = 0;
    for (const auto& entry : gra) {
        for (const auto& nbr : entry.second) {
            const auto& edge = nbr.second;
            CHECK_EQ(weight(edge), doctest::Approx(ratio.eval(edge, x)));
            ++checked;
        }
    }
    CHECK(checked > 0);
}

TEST_CASE("solve_opt_scaling facade forwards Options") {
    auto gra = create_fixed_graph();
    const auto log10 = std::numbers::ln10;
    const auto log125 = std::log(125.0);
    const auto xinit = std::valarray<double>{log125, log10};
    const auto t = log125 - log10;

    auto run = [&](const Options& options) {
        auto ellip = Ell{200.0 * t, xinit};
        absl::flat_hash_map<uint32_t, double> dist{
            {0, 0.0}, {1, 0.0}, {2, 0.0}, {3, 0.0}, {4, 0.0}};
        auto omega = OptScalingOracle(gra, dist, get_fixed_cost);
        auto gamma = std::numeric_limits<double>::infinity();
        return solve_opt_scaling(omega, ellip, gamma, options);
    };

    const auto [x_default, niter_default] = run(default_options());
    const auto tight = Options{2000, 1e-20};
    const auto [x_tight, niter_tight] = run(tight);

    CHECK_EQ(kDefaultTolerance, doctest::Approx(1e-8));
    CHECK_NE(x_default.size(), 0U);
    CHECK_NE(x_tight.size(), 0U);
    CHECK_LE(niter_default, niter_tight);
}

TEST_CASE("solve_network_feas facade returns a feasible point") {
    auto gra = create_cycle_graph();

    MockOracle oracle;
    oracle.values = {{{0, 1}, 1.0}, {{1, 2}, 1.0}, {{2, 0}, 1.0}};
    oracle.grads = {{{0, 1}, 1.0}, {{1, 2}, 1.0}, {{2, 0}, 1.0}};
    absl::flat_hash_map<uint32_t, double> dist{{0, 0.0}, {1, 0.0}, {2, 0.0}};
    auto network = NetworkOracle(gra, dist, oracle);
    auto space = Ell{10.0, std::valarray<double>{0.0, 0.0}};

    const auto [xbest, niter] = solve_network_feas(network, space);

    CHECK_NE(xbest.size(), 0U);
    CHECK_EQ(niter, 0U);
}
