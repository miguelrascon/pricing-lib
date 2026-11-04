#include "pricing/pricing.h"
#include <cmath>


namespace math {
    const double PI = 3.14159265358979323846;

    double norm_pdf(double x){
        return std::exp(-0.5 * x * x) / std::sqrt(2 * PI);
    }

    double norm_cdf(double x){
        return 0.5 * std::erfc(-x / std::sqrt(2));
    }
}

namespace pricing{
    // D1 and D2 emerge from solving the stochastic differential equatino of the stock's path using Ito's lemma. 
    // N2 is the riks-neutral probability of the option expiring in the money, and N(d1) is the expected Delta of the call - option (the probability-weighted sensitivity of the options price to the underlying stock)
    double d1(const BSParams& params){
        return (std::log(params.spot / params.strike) + (params.rate + 0.5 * params.vol * params.vol) * params.time) / (params.vol * std::sqrt(params.time));
    }

    double d2(const BSParams& params){
        return d1(params) - params.vol * std::sqrt(params.time);
    }

    double bs_call(const BSParams& params){
        return params.spot * math::norm_cdf(d1(params)) - params.strike * std::exp(-params.rate * params.time) * math::norm_cdf(d2(params));
    }

    double bs_put(const BSParams& params){
        return params.strike * std::exp(-params.rate * params.time) * math::norm_cdf(-d2(params)) - params.spot * math::norm_cdf(-d1(params));
    };

    double delta_call(const BSParams& params){
        return math::norm_cdf(d1(params));
    }

    double delta_put(const BSParams& params){
        return math::norm_cdf(d1(params)) - 1;
    }
}