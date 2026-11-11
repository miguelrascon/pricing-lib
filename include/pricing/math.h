#pragma once

// Utilidades matemáticas compartidas por todos los modelos de la librería
// (analíticos, Monte Carlo, EDP...). Se mantienen separadas de pricing.h
// para que cada módulo incluya solo lo que necesita.

namespace math {

    // Constantes en tiempo de compilación (C++17 no tiene std::numbers, que llega en C++20).
    inline constexpr double PI = 3.14159265358979323846;
    inline constexpr double INV_SQRT_2PI = 0.39894228040143267794;  // 1 / sqrt(2*pi)
    inline constexpr double INV_SQRT_2 = 0.70710678118654752440;    // 1 / sqrt(2)

    // Función de densidad de la normal estándar: phi(x) = exp(-x^2/2) / sqrt(2*pi)
    double norm_pdf(double x);

    // Función de distribución acumulada de la normal estándar: N(x) = P(Z <= x)
    double norm_cdf(double x);

}  // namespace math
