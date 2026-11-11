# C++ Derivatives Pricing Library

![CI](https://github.com/miguelrascon/pricing-lib/actions/workflows/ci.yml/badge.svg)

A modern C++17 quantitative finance library built to implement standard pricing models, from closed-form analytic solutions to Monte Carlo engines and PDE solvers.

This is a learning project developed alongside a Master's programme in quantitative finance. See [ROADMAP.md](ROADMAP.md) for the planned features.

## Features

| Model | Products | Outputs |
|---|---|---|
| Black-Scholes (closed form) | European call / put | Price, delta |

- Input validation (`std::invalid_argument` for out-of-domain or non-finite parameters).
- Degenerate cases handled: at expiry (`T = 0`) and zero volatility (`σ = 0`).

## Project structure

```
include/pricing/   Public headers (math.h, pricing.h)
src/               Library implementation + example app (main.cpp)
tests/             Unit tests (Catch2)
```

## Build and test

Requirements: CMake ≥ 3.14 and a C++17 compiler (GCC, Clang or MSVC). Catch2 is downloaded automatically.

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Optional: `-DPRICING_WARNINGS_AS_ERRORS=ON` treats compiler warnings as errors (used in CI).

## Usage

```cpp
#include "pricing/pricing.h"

// spot, strike, rate, vol, time
pricing::BSParams p{100.0, 100.0, 0.05, 0.2, 1.0};
double call  = pricing::bs_call(p);     // 10.4506
double delta = pricing::delta_call(p);  // 0.6368
```

## References

- J. Hull, *Options, Futures, and Other Derivatives*.

## Author

Miguel Rascón Ramiro (miguelrasconramiro@gmail.com)
