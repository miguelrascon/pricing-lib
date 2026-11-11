#include "pricing/pricing.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace pricing {

    // Namespace anónimo: estas funciones auxiliares solo son visibles dentro de
    // este fichero (enlace interno). No forman parte de la API pública.
    namespace {

        // Factor de descuento continuo: e^{-rT}
        double discount_factor(const BSParams& p) {
            return std::exp(-p.rate * p.time);
        }

        // Caso degenerado: sin incertidumbre (vol = 0) o sin tiempo (T = 0).
        // El subyacente es determinista y la fórmula de Black-Scholes no aplica
        // porque d1 y d2 dividen entre vol * sqrt(T) = 0.
        bool is_degenerate(const BSParams& p) {
            return p.vol == 0.0 || p.time == 0.0;
        }

    }  // namespace

    void validate(const BSParams& p) {
        // Se escribe !(x > 0) en lugar de (x <= 0) a propósito: cualquier
        // comparación con NaN devuelve false, así que (NaN <= 0) no detectaría
        // un NaN, mientras que !(NaN > 0) sí lo rechaza.
        if (!(p.spot > 0.0) || !std::isfinite(p.spot))
            throw std::invalid_argument("BSParams: spot must be finite and > 0");
        if (!(p.strike > 0.0) || !std::isfinite(p.strike))
            throw std::invalid_argument("BSParams: strike must be finite and > 0");
        if (!(p.vol >= 0.0) || !std::isfinite(p.vol))
            throw std::invalid_argument("BSParams: vol must be finite and >= 0");
        if (!(p.time >= 0.0) || !std::isfinite(p.time))
            throw std::invalid_argument("BSParams: time must be finite and >= 0");
        // El tipo de interés puede ser negativo (p. ej. EUR 2015-2022), solo exigimos que sea finito.
        if (!std::isfinite(p.rate))
            throw std::invalid_argument("BSParams: rate must be finite");
    }

    // d1 y d2 surgen al resolver la ecuación diferencial estocástica del subyacente
    // (movimiento browniano geométrico) mediante el lema de Itô y valorar bajo la
    // medida riesgo-neutral:
    //   - N(d2) es la probabilidad riesgo-neutral de que el call termine dentro del dinero.
    //   - N(d1) es la delta del call: sensibilidad del precio del call al subyacente.
    double d1(const BSParams& p) {
        validate(p);
        if (is_degenerate(p))
            throw std::domain_error("d1 undefined: requires vol > 0 and time > 0");

        const double vol_sqrt_t = p.vol * std::sqrt(p.time);
        return (std::log(p.spot / p.strike) + (p.rate + 0.5 * p.vol * p.vol) * p.time) / vol_sqrt_t;
    }

    double d2(const BSParams& p) {
        return d1(p) - p.vol * std::sqrt(p.time);
    }

    double bs_call(const BSParams& p) {
        validate(p);
        const double df = discount_factor(p);

        // Límite determinista: C = max(S - K e^{-rT}, 0)
        if (is_degenerate(p))
            return std::max(p.spot - p.strike * df, 0.0);

        // Se calcula d1 una sola vez y se reutiliza para d2 (evita recalcular el logaritmo).
        const double d_1 = d1(p);
        const double d_2 = d_1 - p.vol * std::sqrt(p.time);
        return p.spot * math::norm_cdf(d_1) - p.strike * df * math::norm_cdf(d_2);
    }

    double bs_put(const BSParams& p) {
        validate(p);
        const double df = discount_factor(p);

        // Límite determinista: P = max(K e^{-rT} - S, 0)
        if (is_degenerate(p))
            return std::max(p.strike * df - p.spot, 0.0);

        const double d_1 = d1(p);
        const double d_2 = d_1 - p.vol * std::sqrt(p.time);
        return p.strike * df * math::norm_cdf(-d_2) - p.spot * math::norm_cdf(-d_1);
    }

    double delta_call(const BSParams& p) {
        validate(p);

        // En el límite determinista la delta es un escalón: 1 si el call acaba
        // dentro del dinero, 0 si no. Justo en S = K e^{-rT} se toma 0.5, que es
        // el límite de N(d1) cuando vol -> 0 (ahí d1 = vol*sqrt(T)/2 -> 0).
        if (is_degenerate(p)) {
            const double forward_strike = p.strike * discount_factor(p);
            if (p.spot > forward_strike) return 1.0;
            if (p.spot < forward_strike) return 0.0;
            return 0.5;
        }
        return math::norm_cdf(d1(p));
    }

    double delta_put(const BSParams& p) {
        // Derivando la paridad put-call (C - P = S - K e^{-rT}) respecto a S:
        // delta_call - delta_put = 1  =>  delta_put = delta_call - 1
        return delta_call(p) - 1.0;
    }

}  // namespace pricing
