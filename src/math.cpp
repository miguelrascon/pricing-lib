#include "pricing/math.h"

#include <cmath>

namespace math {

    double norm_pdf(double x) {
        return INV_SQRT_2PI * std::exp(-0.5 * x * x);
    }

    double norm_cdf(double x) {
        // Se usa erfc en lugar de 0.5 * (1 + erf(x / sqrt(2))) porque erfc mantiene
        // precisión relativa en la cola izquierda (x muy negativo), donde
        // 1 + erf(...) sufriría cancelación catastrófica y devolvería 0.
        return 0.5 * std::erfc(-x * INV_SQRT_2);
    }

}  // namespace math
