#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "pricing/pricing.h"

TEST_CASE("Black-Scholes Call Price matches Hull", "[analytic]"){

    REQUIRE(math::norm_pdf(0.0) == Catch::Approx(0.39894).epsilon(0.0001));
    
    // Test the normal CDF at x = 0 (should be exactly 0.5)
    REQUIRE(math::norm_cdf(0.0) == Catch::Approx(0.5).epsilon(0.0001));

    pricing::BSParams params = {100.0, 100.0, 0.05, 0.2, 1.0};
    
    double call_price = pricing::bs_call(params);

    REQUIRE(call_price == Catch::Approx(10.4506).epsilon(0.0001));
}