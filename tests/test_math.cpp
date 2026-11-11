#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "pricing/math.h"

TEST_CASE("norm_pdf known values", "[math]") {
    // phi(0) = 1 / sqrt(2*pi)
    REQUIRE(math::norm_pdf(0.0) == Catch::Approx(0.3989422804).epsilon(1e-10));
    // Simetría: phi(x) = phi(-x)
    REQUIRE(math::norm_pdf(1.5) == Catch::Approx(math::norm_pdf(-1.5)));
}

TEST_CASE("norm_cdf known values", "[math]") {
    REQUIRE(math::norm_cdf(0.0) == Catch::Approx(0.5).epsilon(1e-12));
    // Cuantil del 97.5%, el famoso 1.96 de los intervalos de confianza
    REQUIRE(math::norm_cdf(1.959963984540054) == Catch::Approx(0.975).epsilon(1e-12));
    // Simetría: N(x) + N(-x) = 1
    REQUIRE(math::norm_cdf(0.7) + math::norm_cdf(-0.7) == Catch::Approx(1.0).epsilon(1e-14));
}

TEST_CASE("norm_cdf keeps precision in the left tail", "[math]") {
    // N(-10) ~ 7.62e-24. Con 0.5 * (1 + erf(x/sqrt2)) saldría exactamente 0
    // por cancelación catastrófica; con erfc se conserva la precisión relativa.
    // Nota: Approx.epsilon() es una tolerancia RELATIVA, por eso funciona con valores tan pequeños.
    REQUIRE(math::norm_cdf(-10.0) == Catch::Approx(7.619853024160527e-24).epsilon(1e-10));
}
