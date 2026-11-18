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

        // Dividend factor: e^{-qT}
        double dividend_factor(const BSParams& p) {
            return std::exp(-p.q * p.time);
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
        
        if (!std::isfinite(p.q))
            throw std::invalid_argument("BSParams: dividend q must be finite");
    }

    // d1 y d2 surgen al resolver la ecuación diferencial estocástica del subyacente
    // (movimiento browniano geométrico) mediante el lema de Itô y valorar bajo la
    // medida riesgo-neutral:
    //   - N(d2) es la probabilidad riesgo-neutral de que el call termine dentro del dinero.
    //   - N(d1) es la delta del call: sensibilidad del precio del call al subyacente (N(d1) * e^{-qT}).
    // TODO(q): the last bullet is no longer exact. With dividends the call delta is
    //   e^{-qT} N(d1), not N(d1). N(d1) alone is now the delta with respect to the
    //   prepaid forward S e^{-qT}. Update it so the comment matches delta_call().
    double d1(const BSParams& p) {
        validate(p);
        if (is_degenerate(p))
            throw std::domain_error("d1 undefined: requires vol > 0 and time > 0");

        const double vol_sqrt_t = p.vol * std::sqrt(p.time);
        return (std::log(p.spot / p.strike) + (p.rate - p.q + 0.5 * p.vol * p.vol) * p.time) / vol_sqrt_t;
    }

    double d2(const BSParams& p) {
        return d1(p) - p.vol * std::sqrt(p.time);
    }

    double bs_call(const BSParams& p) {
        validate(p);
        const double df = discount_factor(p);
        const double divf = dividend_factor(p);

        // Límite determinista: C = max(S e^{-qT} - K e^{-rT}, 0)
        if (is_degenerate(p))
            return std::max(p.spot*divf - p.strike * df, 0.0);

        // Se calcula d1 una sola vez y se reutiliza para d2 (evita recalcular el logaritmo).
        const double d_1 = d1(p);
        const double d_2 = d_1 - p.vol * std::sqrt(p.time);
        return p.spot * divf * math::norm_cdf(d_1) - p.strike * df * math::norm_cdf(d_2);
    }

    double bs_put(const BSParams& p) {
        validate(p);
        const double df = discount_factor(p);
        const double divf = dividend_factor(p);

        // Límite determinista: P = max(K e^{-rT} - S e^{-qT}, 0)
        if (is_degenerate(p))
            return std::max(p.strike * df - p.spot*divf, 0.0);

        const double d_1 = d1(p);
        const double d_2 = d_1 - p.vol * std::sqrt(p.time);
        return p.strike * df * math::norm_cdf(-d_2) - p.spot * divf * math::norm_cdf(-d_1);
    }

    // Call delta: dC/dS = e^{-qT} N(d1).
    // The e^{-qT} factor appears because the price depends on S only through the
    // prepaid forward S e^{-qT}: moving S by 1 moves the prepaid forward by e^{-qT}.
    double delta_call(const BSParams& p) {
        validate(p);

        const double divf = dividend_factor(p);  // e^{-qT}

        // Degenerate case (vol = 0 or T = 0): the call is worth max(S e^{-qT} - K e^{-rT}, 0),
        // so its delta is a step function of S:
        //   - e^{-qT} if S e^{-qT} > K e^{-rT}   (in the money: slope of S e^{-qT})
        //   - 0       if S e^{-qT} < K e^{-rT}   (out of the money: price is flat at 0)
        //   - 0.5 e^{-qT} exactly at the kink. This is the limit of e^{-qT} N(d1) as vol -> 0:
        //     at that point ln(S/K) = (q - r)T, the (r - q) terms cancel and
        //     d1 = vol*sqrt(T)/2 -> 0, so N(d1) -> 0.5.
        if (is_degenerate(p)) {
            const double prepaid_forward = p.spot * divf;                     // S e^{-qT}
            const double discounted_strike = p.strike * discount_factor(p);   // K e^{-rT}
            if (prepaid_forward > discounted_strike) return divf;
            if (prepaid_forward < discounted_strike) return 0.0;
            return 0.5 * divf;
        }

        return divf * math::norm_cdf(d1(p));
    }

    // Put delta, obtained from put-call parity instead of a separate formula.
    // Parity with dividends:           C - P = S e^{-qT} - K e^{-rT}
    // Differentiating w.r.t. S:        delta_call - delta_put = e^{-qT}
    // Therefore:                       delta_put = delta_call - e^{-qT} = -e^{-qT} N(-d1)
    // Note it is e^{-qT}, not 1: with q = 0 we recover the familiar delta_call - 1.
    double delta_put(const BSParams& p) {
        // delta_call is computed in its own statement on purpose. In an expression like
        // `delta_call(p) - dividend_factor(p)` C++ does not specify which operand is
        // evaluated first, so dividend_factor could run before the validation inside
        // delta_call. Splitting it into two statements guarantees the order:
        // validate first, then compute.
        const double call_delta = delta_call(p);
        return call_delta - dividend_factor(p);
    }

}  // namespace pricing
