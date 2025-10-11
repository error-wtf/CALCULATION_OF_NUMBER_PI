// chudnovsky.h
//
// Declarations for computing π using the Chudnovsky series on both
// CPU and GPU.  The CPU implementation uses Boost.Multiprecision to
// perform arbitrary precision arithmetic, while the GPU implementation
// offloads the evaluation of individual series terms to a CUDA kernel
// and accumulates the result on the host.  In addition, a simple
// Bailey–Borwein–Plouffe (BBP) implementation is provided for
// approximate values and validation.  These functions are intended
// for educational and moderate‑precision use.  For truly large
// computations you should implement a full binary splitting
// algorithm and out‑of‑core arithmetic as discussed in the project
// documentation.

#pragma once

#include <string>
#include <cstdint>
#include <functional>

namespace picore {

// Compute π using the Chudnovsky series on the CPU.  This function
// evaluates a truncated Chudnovsky series using Boost's
// multiprecision types and returns π as a fixed‑point decimal string
// with the requested number of digits after the decimal point.  A
// small guard is added internally to ensure the result rounds
// correctly.  For large values of `digits` (tens of thousands and
// beyond) this implementation becomes slow; consider binary
// splitting if you need millions of digits.
//
// Parameters:
//   digits – number of decimal digits of π to compute (≥ 1)
//
// Returns:
//   A string containing π with exactly `digits` digits after the
//   decimal point.
std::string compute_pi_chudnovsky_cpu(int digits, std::function<void(double, const std::string&)> progress_callback = nullptr);

// Compute π using the Chudnovsky series with a GPU assist.  The
// GPU kernel evaluates a limited number of terms of the series in
// double precision and returns an accumulated sum to the host.  This
// technique is only suitable for relatively small precision
// (hundreds of digits) because factorial values overflow double
// quickly.  If CUDA is not available or the requested digit count
// exceeds the safe range, this function falls back to the CPU
// implementation above.  When compiled without CUDA support, the
// function behaves identically to compute_pi_chudnovsky_cpu().
//
// Parameters:
//   digits – number of decimal digits of π to compute (≥ 1)
//
// Returns:
//   A string containing π with exactly `digits` digits after the
//   decimal point.
std::string compute_pi_chudnovsky_gpu(int digits, std::function<void(double, const std::string&)> progress_callback = nullptr);

// Compute π using a simple Bailey–Borwein–Plouffe (BBP) series.  This
// algorithm yields hexadecimal digits of π and converges slowly in
// comparison to Chudnovsky.  It is useful here primarily for
// cross‑checking and validating chunks of the Chudnovsky result.  The
// return value is a double precision approximation of π computed
// using the given number of terms.  Because of limited precision,
// this function should not be used for high‑precision results.
//
// Parameters:
//   terms – number of BBP terms to sum (≥ 1)
//
// Returns:
//   A double precision approximation of π.
double compute_pi_bbp(int terms);

} // namespace picore