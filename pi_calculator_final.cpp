
#include <iostream>
#include <cmath>
#include <gmp.h>
#include <gmpxx.h>
#include <chrono>
#include <vector>
#include <cuda_runtime.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <stdexcept>
#include <limits>
#include <filesystem>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#endif

using namespace std;
namespace fs = std::filesystem;

// Constants for Chudnovsky algorithm
const double A = 13591409.0;
const double B = 545140134.0;
const double C = 640320.0;
const double C3_24 = (C * C * C) / 24.0;

std::mutex mtx; // Mutex for thread safety

// Function to get available RAM
size_t getAvailableRAM() {
#ifdef _WIN32
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);
    return memStatus.ullAvailPhys;
#else
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
#endif
}

// Function to get used RAM
size_t getUsedRAM() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX memInfo;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memInfo, sizeof(memInfo));
    return memInfo.WorkingSetSize;
#else
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss * 1024;
#endif
}

// Function to get available disk space
size_t getAvailableDiskSpace(const string& path = ".") {
    fs::space_info si = fs::space(path);
    return si.available;
}

// Function to get the number of available CPU cores
int getAvailableCores() {
    return thread::hardware_concurrency();
}

// CUDA kernel for computing Chudnovsky terms
__global__ void compute_chudnovsky_terms(double* numerators, double* denominators, int start, int n) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    if (idx < n) {
        int k = start + idx;
        numerators[idx] = tgamma(6 * k + 1.0) / (tgamma(3 * k + 1.0) * pow(tgamma(k + 1.0), 3));
        denominators[idx] = pow(-C3_24, k) * (A + B * k);
    }
}

// BBP formula calculation
mpf_class calculate_bbp_section(int start, int terms) {
    mpf_class sum = 0;
    for (int k = start; k < start + terms; ++k) {
        mpf_class term = 1.0 / pow(16.0, k) * (
            4.0 / (8 * k + 1) -
            2.0 / (8 * k + 4) -
            1.0 / (8 * k + 5) -
            1.0 / (8 * k + 6)
        );
        sum += term;
    }
    return sum;
}

// Karatsuba multiplication for large numbers
mpz_class karatsuba_multiply(const mpz_class& a, const mpz_class& b) {
    if (a < 10 || b < 10) return a * b;

    size_t n = std::max(a.get_str().size(), b.get_str().size());
    size_t m = n / 2;

    mpz_class high1 = a / mpz_class(pow(10, m));
    mpz_class low1 = a % mpz_class(pow(10, m));
    mpz_class high2 = b / mpz_class(pow(10, m));
    mpz_class low2 = b % mpz_class(pow(10, m));

    mpz_class z0 = karatsuba_multiply(low1, low2);
    mpz_class z1 = karatsuba_multiply(low1 + high1, low2 + high2);
    mpz_class z2 = karatsuba_multiply(high1, high2);

    return z2 * mpz_class(pow(10, 2 * m)) + 
           (z1 - z2 - z0) * mpz_class(pow(10, m)) + 
           z0;
}

// Parallel processing of Chudnovsky terms
void process_chudnovsky_terms(int start, int end, mpf_class& sum_num, mpf_class& sum_den) {
    mpf_class local_sum_num = 0;
    mpf_class local_sum_den = 0;

    for (int k = start; k < end; ++k) {
        mpf_class num = tgamma(6 * k + 1.0) / (tgamma(3 * k + 1.0) * pow(tgamma(k + 1.0), 3));
        mpf_class den = pow(-C3_24, k) * (A + B * k);
        local_sum_num += num;
        local_sum_den += den;
    }

    std::lock_guard<std::mutex> lock(mtx);
    sum_num += local_sum_num;
    sum_den += local_sum_den;
}

// Main calculation function combining CUDA and GMP
void calculate_pi(int terms, int threads_per_block = 256) {
    // Allocate device memory
    double *d_numerators, *d_denominators;
    size_t size = terms * sizeof(double);
    cudaMalloc(&d_numerators, size);
    cudaMalloc(&d_denominators, size);

    // Calculate grid dimensions
    int blocks = (terms + threads_per_block - 1) / threads_per_block;

    // Start timing
    auto start = std::chrono::high_resolution_clock::now();

    // Launch CUDA kernel
    compute_chudnovsky_terms<<<blocks, threads_per_block>>>(
        d_numerators, d_denominators, 0, terms
    );

    // Allocate host memory and copy results back
    std::vector<double> h_numerators(terms);
    std::vector<double> h_denominators(terms);
    cudaMemcpy(h_numerators.data(), d_numerators, size, cudaMemcpyDeviceToHost);
    cudaMemcpy(h_denominators.data(), d_denominators, size, cudaMemcpyDeviceToHost);

    // Set GMP precision
    mpf_set_default_prec(1000000);

    // Sum the terms using GMP
    mpf_class sum_num = 0;
    mpf_class sum_den = 0;
    for (int i = 0; i < terms; i++) {
        sum_num += mpf_class(h_numerators[i]);
        sum_den += mpf_class(h_denominators[i]);
    }

    // Calculate final result
    mpf_class pi_chudnovsky = (426880 * sqrt(mpf_class(10005))) / (sum_num / sum_den);

    // Calculate BBP result for validation
    mpf_class pi_bbp = calculate_bbp_section(0, terms / 10);

    // End timing
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // Output results
    std::cout << "Chudnovsky Pi: " << pi_chudnovsky << std::endl;
    std::cout << "BBP Pi: " << pi_bbp << std::endl;
    std::cout << "Calculation time: " << elapsed.count() << " seconds" << std::endl;

    // Cleanup
    cudaFree(d_numerators);
    cudaFree(d_denominators);
}

int main() {
    int terms = 1000000;  // Adjust based on available GPU memory
    size_t available_ram = getAvailableRAM();
    size_t available_disk = getAvailableDiskSpace();

    std::cout << "Available RAM: " << available_ram / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Available Disk Space: " << available_disk / (1024 * 1024) << " MB" << std::endl;

    std::cout << "Calculating Pi using " << terms << " terms..." << std::endl;
    calculate_pi(terms);
    return 0;
}
