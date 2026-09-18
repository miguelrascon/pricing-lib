#pragma once 

namespace math{
    double norm_pdf(double x);
    double norm_cdf(double x);
}

namespace pricing{
    struct BSParams{
        double spot; // S
        double strike; //K
        double rate; //r (risk free rate)
        double vol; //sigma (implied volatility)
        double time; // T (time to maturity in years)
    };

    double d1(const BSParams& params);
    double d2(const BSParams& params);
    double bs_call(const BSParams& params);
    double bs_put(const BSParams& params);
}