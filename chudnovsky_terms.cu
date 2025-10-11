// chudnovsky_terms.cu
//
// CUDA kernel implementation for computing terms of the Chudnovsky
// series in double precision.  Each thread computes the term
// corresponding to its index and writes the result to a device array.

#include <cmath>

// Note: the kernel must have C linkage so that it can be called from
// C++ code that is unaware of CUDA name mangling.
extern "C" __global__ void compute_chudnovsky_terms_kernel(double *terms, int n_terms)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= n_terms) return;
    // Compute factorial ratios using the gamma function.  This is
    // inherently unstable for large k, so this kernel is intended
    // only for small k (≈ up to 15) before overflow occurs.  See
    // chudnovsky.cpp for details.
    double kf = static_cast<double>(idx);
    // (6k)! / (3k)! / (k!)^3
    double coeff = tgamma(6.0 * kf + 1.0);
    coeff /= tgamma(3.0 * kf + 1.0);
    double kfact = tgamma(kf + 1.0);
    coeff /= (kfact * kfact * kfact);
    // Linear term (13591409 + 545140134 * k)
    double linear = 13591409.0 + 545140134.0 * kf;
    double term = coeff * linear;
    // (640320)^(3k)
    double powfactor = pow(640320.0, 3.0 * kf);
    term /= powfactor;
    // Alternating sign
    if (idx & 1) term = -term;
    terms[idx] = term;
}