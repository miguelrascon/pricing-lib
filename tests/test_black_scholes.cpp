#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cmath>
#include <limits>
#include <stdexcept>

#include "pricing/pricing.h"

// Caso de referencia de Hull: S = 100, K = 100, r = 5%, sigma = 20%, T = 1 año.
// Orden de los campos: spot, strike, rate, vol, time
static const pricing::BSParams HULL{100.0, 100.0, 0.05, 0.2, 1.0};

TEST_CASE("Black-Scholes prices match Hull", "[analytic][price]") {
    REQUIRE(pricing::bs_call(HULL) == Catch::Approx(10.4506).epsilon(1e-4));
    REQUIRE(pricing::bs_put(HULL) == Catch::Approx(5.5735).epsilon(1e-4));
}

TEST_CASE("Black-Scholes deltas match Hull", "[analytic][delta]") {
    const double call_delta = pricing::delta_call(HULL);
    const double put_delta = pricing::delta_put(HULL);

    REQUIRE(call_delta == Catch::Approx(0.6368).epsilon(1e-4));
    REQUIRE(put_delta == Catch::Approx(-0.3632).epsilon(1e-4));
    REQUIRE(call_delta - put_delta == Catch::Approx(1.0).epsilon(1e-12));
}

TEST_CASE("Put-call parity holds across strikes", "[analytic][parity]") {
    // GENERATE ejecuta el test una vez por cada valor: ITM, ATM y OTM.
    const double strike = GENERATE(60.0, 90.0, 100.0, 110.0, 150.0);
    pricing::BSParams p = HULL;
    p.strike = strike;

    const double lhs = pricing::bs_call(p) - pricing::bs_put(p);
    const double rhs = p.spot - p.strike * std::exp(-p.rate * p.time);

    // Para K = 100 el lado derecho vale ~4.88, pero para otros strikes puede
    // acercarse a 0, donde una tolerancia relativa es demasiado estricta.
    // margin() añade una tolerancia ABSOLUTA.
    REQUIRE(lhs == Catch::Approx(rhs).margin(1e-10));
}

TEST_CASE("Degenerate cases: zero time and zero volatility", "[analytic][edge]") {
    SECTION("At expiry (T = 0) the price is the intrinsic value") {
        pricing::BSParams p{110.0, 100.0, 0.05, 0.2, 0.0};
        REQUIRE(pricing::bs_call(p) == Catch::Approx(10.0));
        REQUIRE(pricing::bs_put(p) == Catch::Approx(0.0).margin(1e-14));
        REQUIRE(pricing::delta_call(p) == 1.0);
        REQUIRE(pricing::delta_put(p) == 0.0);
    }

    SECTION("Zero volatility: discounted intrinsic value on the forward") {
        pricing::BSParams p{100.0, 100.0, 0.05, 0.0, 1.0};
        const double expected = 100.0 - 100.0 * std::exp(-0.05);
        REQUIRE(pricing::bs_call(p) == Catch::Approx(expected));
        REQUIRE(pricing::bs_put(p) == Catch::Approx(0.0).margin(1e-14));
    }

    SECTION("Small volatility converges to the deterministic limit") {
        // Comprobamos continuidad: con sigma muy pequeña la fórmula de BS
        // debe acercarse al valor del caso degenerado.
        pricing::BSParams p{100.0, 100.0, 0.05, 1e-8, 1.0};
        const double limit = 100.0 - 100.0 * std::exp(-0.05);
        REQUIRE(pricing::bs_call(p) == Catch::Approx(limit).epsilon(1e-6));
    }

    SECTION("d1 is undefined in the degenerate case") {
        pricing::BSParams p{100.0, 100.0, 0.05, 0.0, 1.0};
        REQUIRE_THROWS_AS(pricing::d1(p), std::domain_error);
    }
}

TEST_CASE("Invalid inputs are rejected", "[analytic][validation]") {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    pricing::BSParams p = HULL;

    SECTION("Non-positive spot") {
        p.spot = 0.0;
        REQUIRE_THROWS_AS(pricing::bs_call(p), std::invalid_argument);
    }
    SECTION("Negative strike") {
        p.strike = -1.0;
        REQUIRE_THROWS_AS(pricing::bs_put(p), std::invalid_argument);
    }
    SECTION("Negative volatility") {
        p.vol = -0.2;
        REQUIRE_THROWS_AS(pricing::delta_call(p), std::invalid_argument);
    }
    SECTION("Negative time") {
        p.time = -1.0;
        REQUIRE_THROWS_AS(pricing::bs_call(p), std::invalid_argument);
    }
    SECTION("NaN is rejected") {
        p.vol = nan;
        REQUIRE_THROWS_AS(pricing::bs_call(p), std::invalid_argument);
    }
    SECTION("Infinite rate is rejected") {
        p.rate = inf;
        REQUIRE_THROWS_AS(pricing::bs_call(p), std::invalid_argument);
    }
    SECTION("Negative rates are allowed") {
        p.rate = -0.01;
        REQUIRE_NOTHROW(pricing::bs_call(p));
    }
}
