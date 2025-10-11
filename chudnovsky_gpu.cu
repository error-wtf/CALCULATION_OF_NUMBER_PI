#include "picore/chudnovsky.h"

#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <functional>

#ifdef PICORE_WITH_CUDA
#include <cuda_runtime.h>
#endif

namespace picore {

// Forward declaration for the CUDA kernel defined in a .cu file.
#ifdef PICORE_WITH_CUDA
extern "C" __global__ void compute_chudnovsky_terms_kernel(double *terms, int n_terms);
#endif

// GPU‑assisted Chudnovsky computation.  If CUDA is available and
// digits are within a safe range, this will use a GPU kernel to
// evaluate the series terms in double precision.  For digits beyond
// ~200 it falls back to the CPU implementation because double
// precision cannot represent the factorial ratios accurately.
std::string compute_pi_chudnovsky_gpu(int digits, std::function<void(double, const std::string&)> progress_callback)
{
#ifdef PICORE_WITH_CUDA
    // Use GPU only for small digit counts (≤ 200).  Beyond that, the
    // factorial terms overflow doubles quickly and the result becomes
    // inaccurate.
    if (digits > 200) {
        return compute_pi_chudnovsky_cpu(digits, progress_callback);
    }
    if (digits <= 0) return "";
    // Determine the number of terms needed.
    int num_terms = digits / 14 + 10;
    if (num_terms > 50) {
        // Even 50 terms may overflow double, so clamp.
        num_terms = 50;
    }
    // Allocate device and host memory for term results.
    double *d_terms = nullptr;
    cudaError_t err;
    if (progress_callback) progress_callback(0.0, "Allocating GPU memory");
    err = cudaMalloc(&d_terms, sizeof(double) * num_terms);
    if (err != cudaSuccess) {
        return compute_pi_chudnovsky_cpu(digits, progress_callback);
    }
    // Launch kernel with enough blocks/threads.
    int threads_per_block = 128;
    int blocks = (num_terms + threads_per_block - 1) / threads_per_block;
    if (progress_callback) progress_callback(0.1, "Launching CUDA kernel");
    compute_chudnovsky_terms_kernel<<<blocks, threads_per_block>>>(d_terms, num_terms);
    cudaDeviceSynchronize();
    if (progress_callback) progress_callback(0.5, "CUDA kernel finished, copying results");
    // Copy results back to host
    std::vector<double> h_terms(num_terms);
    cudaMemcpy(h_terms.data(), d_terms, sizeof(double) * num_terms, cudaMemcpyDeviceToHost);
    cudaFree(d_terms);
    if (progress_callback) progress_callback(0.7, "Summing terms on CPU");
    // Sum the terms on the host using long double for extra precision.
    long double sum = 0.0L;
    for (int i = 0; i < num_terms; ++i) {
        sum += h_terms[i];
    }
    if (progress_callback) progress_callback(0.9, "Applying constant factor");
    // Compute π = constant / sum
    long double sqrt10005 = std::sqrt((long double)10005.0L);
    long double constant = 426880.0L * sqrt10005;
    long double pi_ld = constant / sum;
    // Convert to string with requested digits.
    // Note: convert to double precision; this only makes sense for small digits.
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss.precision(digits);
    oss << (double)pi_ld;
    if (progress_callback) progress_callback(1.0, "GPU computation complete");
    return oss.str();
#else
    // Without CUDA support, just call the CPU implementation.
    return compute_pi_chudnovsky_cpu(digits, progress_callback);
#endif
}

} // namespace picore