#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "pricing/pricing.h"

TEST_CASE("Black-Scholes Call Price matches Hull", "[analytic]"){

    REQUIRE(math::norm_pdf(0.0) == Catch::Approx(0.39894).epsilon(0.0001));
    
    // Test the normal CDF at x = 0 (should be exactly 0.5)
    REQUIRE(math::norm_cdf(0.0) == Catch::Approx(0.5).epsilon(0.0001));

    // params: spot, strike, rate, vol, time
    pricing::BSParams params = {100.0, 100.0, 0.05, 0.2, 1.0};
    
    double call_price = pricing::bs_call(params);

    REQUIRE(call_price == Catch::Approx(10.4506).epsilon(0.0001));

    double put_price = pricing::bs_put(params);
    REQUIRE(put_price == Catch::Approx(5.5735).epsilon(0.0001));

    // Put call parity: C - P = S - K * exp(-r * T)
    REQUIRE(call_price - put_price == Catch::Approx(params.spot - params.strike * std::exp(-params.rate * params.time)).epsilon(0.0001));

}

TEST_CASE("Black-Scholes Delta matches Hull", "[analytic]"){
    
    pricing::BSParams params = {100.0, 100.0, 0.05, 0.2, 1.0};

    double call_delta = pricing::delta_call(params);
    REQUIRE(call_delta == Catch::Approx(0.6368).epsilon(0.0001));

    double put_delta = pricing::delta_put(params);
    REQUIRE(put_delta == Catch::Approx(-0.3632).epsilon(0.0001));

    REQUIRE(call_delta - put_delta == Catch::Approx(1.0).epsilon(0.0001));

}