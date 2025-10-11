#include <cuda_runtime.h>

// A simple no‑op kernel for demonstration purposes.
__global__ void ntt_kernel() {
    // do nothing
}

// Optional entry point to launch the kernel.
extern "C" void launch_ntt_kernel() {
    // Launch with one block of one thread for demonstration.
    ntt_kernel<<<1, 1>>>();
    cudaDeviceSynchronize();
}
