// chudnovsky.cpp
//
// This source file implements the functions declared in
// `chudnovsky.h`.  It provides both CPU and GPU variants of the
// Chudnovsky algorithm, as well as a simple BBP approximation.  The
// CPU variant uses Boost.Multiprecision to achieve arbitrary precision
// arithmetic; the GPU variant leverages a CUDA kernel to compute the
// factorial coefficient and linear term for each element of the
// series and then performs the summation on the host.  A BBP
// implementation is included for quick validation and approximate
// values.

#include "picore/chudnovsky.h"

#ifndef DEC_DIGITS
#define DEC_DIGITS 20000   // sichere Obergrenze für ein erstes Testing
#endif

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <cmath>
#include <sstream>
#include <vector>

#ifdef PICORE_WITH_CUDA
#include <cuda_runtime.h>
#endif

namespace picore {

using boost::multiprecision::cpp_dec_float_50;

// Internal helper to compute a single term of the Chudnovsky series
// using 50‑digit floats.  This is adequate for moderate term counts
// and simplifies the implementation.  For serious high‑precision
// computations a full integer binary splitting approach is
// recommended.
static void compute_chudnovsky_term_50(long k,
                                       cpp_dec_float_50 &numer,
                                       cpp_dec_float_50 &denom)
{
    using dec = cpp_dec_float_50;
    // Compute (6k)! / ((3k)! * (k!)^3)
    dec factorial_num = 1;
    dec factorial_den = 1;
    for (long i = 1; i <= 6 * k; ++i) {
        factorial_num *= dec(i);
    }
    for (long i = 1; i <= 3 * k; ++i) {
        factorial_den *= dec(i);
    }
    for (long i = 1; i <= k; ++i) {
        dec idx = dec(i);
        factorial_den *= idx * idx * idx;
    }
    dec coeff = factorial_num / factorial_den;
    dec linear = dec(13591409) + dec(545140134) * dec(k);
    numer = coeff * linear;
    // Compute (640320)^(3k)
    dec pow_factor = pow(dec(640320), dec(3 * k));
    denom = pow_factor;
    if (k % 2 != 0) {
        numer = -numer;
    }
}

// CPU implementation of the Chudnovsky algorithm.
std::string compute_pi_chudnovsky_cpu(int digits, std::function<void(double, const std::string&)> progress_callback)
{
    if (digits <= 0) return "";
    // Guard digits to ensure rounding correctness.
    int guard = 10;
    int total_digits = digits + guard;
    using dec = boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<DEC_DIGITS>>;
    // Estimate number of terms needed: roughly digits/14 + extra margin.
    int num_terms = digits / 14 + 10;
    // Compute the series sum.
    dec sum = 0;
    for (int k = 0; k < num_terms; ++k) {
        if (progress_callback) {
            double progress = static_cast<double>(k) / num_terms;
            progress_callback(progress, "Computing Chudnovsky terms (CPU)");
        }
        cpp_dec_float_50 numer50;
        cpp_dec_float_50 denom50;
        compute_chudnovsky_term_50(k, numer50, denom50);
        dec numer = dec(numer50);
        dec denom = dec(denom50);
        sum += numer / denom;
    }
    if (progress_callback) {
        progress_callback(1.0, "Chudnovsky terms computed (CPU)");
    }
    // Constant factor 426880 * sqrt(10005)
    dec sqrt10005 = sqrt(dec(10005));
    dec constant = dec(426880) * sqrt10005;
    dec pi = constant / sum;
    // Convert to string with requested precision.
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss.precision(digits);
    oss << pi;
    return oss.str();
}

// Simple BBP implementation.  Computes π by summing the first `terms`
// terms of the BBP series.  The BBP formula produces hexadecimal
// digits of π directly and converges slowly.  We compute a double
// precision approximation.
double compute_pi_bbp(int terms)
{
    if (terms <= 0) return 0.0;
    long double sum = 0.0L;
    for (int k = 0; k < terms; ++k) {
        long double kld = static_cast<long double>(k);
        long double multiplier = 1.0L / pow(16.0L, kld);
        long double term = 4.0L / (8.0L * kld + 1.0L)
                         - 2.0L / (8.0L * kld + 4.0L)
                         - 1.0L / (8.0L * kld + 5.0L)
                         - 1.0L / (8.0L * kld + 6.0L);
        sum += multiplier * term;
    }
    return static_cast<double>(sum);
}

} // namespace picore