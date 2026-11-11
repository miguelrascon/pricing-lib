#pragma once

#include "pricing/math.h"

namespace pricing {

    // Parámetros del modelo de Black-Scholes para una opción europea.
    struct BSParams {
        double spot;    // S: precio actual del subyacente (> 0)
        double strike;  // K: precio de ejercicio (> 0)
        double rate;    // r: tipo libre de riesgo, compuesto en continuo (puede ser negativo)
        double vol;     // sigma: volatilidad anualizada (>= 0)
        double time;    // T: tiempo hasta vencimiento en años (>= 0)
    };

    // Comprueba que los parámetros son válidos.
    // Lanza std::invalid_argument si algún valor no es finito o está fuera de dominio.
    void validate(const BSParams& params);

    // d1 y d2 de Black-Scholes.
    // Requieren vol > 0 y time > 0; en otro caso lanzan std::domain_error,
    // porque el término vol * sqrt(T) del denominador sería cero.
    double d1(const BSParams& params);
    double d2(const BSParams& params);

    // Precios de opciones europeas. Admiten los casos límite T = 0 y vol = 0,
    // en los que devuelven el valor intrínseco descontado max(S - K e^{-rT}, 0).
    double bs_call(const BSParams& params);
    double bs_put(const BSParams& params);

    // Delta: sensibilidad del precio de la opción al spot (dV/dS).
    double delta_call(const BSParams& params);
    double delta_put(const BSParams& params);

}  // namespace pricing
