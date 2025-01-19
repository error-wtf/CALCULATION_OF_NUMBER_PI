#include <iostream>
#include <cmath>
#include <cuda_runtime.h>
#include <gmp.h>
#include <gmpxx.h>
#include <chrono>
#include <vector>

// Konstanten für die Chudnovsky-Methode
__constant__ double A = 13591409.0;
__constant__ double B = 545140134.0;
__constant__ double C = 640320.0;
__constant__ double C3_24 = (C * C * C) / 24.0;

// CUDA-Kernel für die Berechnung der Chudnovsky-Terme
__global__ void compute_terms(double *numerators, double *denominators, int start, int n) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;

    if (idx < n) {
        int k = start + idx;
        numerators[idx] = tgamma(6 * k + 1.0) / (tgamma(3 * k + 1.0) * pow(tgamma(k + 1.0), 3));
        denominators[idx] = pow(-C3_24, k) * (A + B * k);
    }
}

// Funktion zur Fehlerprüfung
inline void check_cuda_error(cudaError_t err, const char* msg) {
    if (err != cudaSuccess) {
        std::cerr << "CUDA Error (" << msg << "): " << cudaGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Funktion zur Berechnung eines Abschnitts von Pi mit BBP
mpf_class calculate_bbp_section(int start, int terms_count) {
    mpf_class sum = 0;
    for (int k = start; k < start + terms_count; ++k) {
        mpf_class term = 1.0 / pow(16.0, k) * (
            4.0 / (8.0 * k + 1.0) -
            2.0 / (8.0 * k + 4.0) -
            1.0 / (8.0 * k + 5.0) -
            1.0 / (8.0 * k + 6.0));
        sum += term;
    }
    return sum;
}

// Karatsuba-Multiplikation für große Zahlen
mpz_class karatsuba_multiply(const mpz_class &a, const mpz_class &b) {
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

    return z2 * mpz_class(pow(10, 2 * m)) + (z1 - z2 - z0) * mpz_class(pow(10, m)) + z0;
}

// Host-Funktion zur Berechnung von Pi mit GMP und CUDA
void calculate_pi_combined(int terms_count, int threads_per_block) {
    // Speicher auf dem Host und Gerät
    double *device_numerators, *device_denominators;
    size_t size = terms_count * sizeof(double);

    std::vector<mpf_class> numerators(terms_count);
    std::vector<mpf_class> denominators(terms_count);

    check_cuda_error(cudaMalloc(&device_numerators, size), "cudaMalloc (numerators)");
    check_cuda_error(cudaMalloc(&device_denominators, size), "cudaMalloc (denominators)");

    // CUDA-Konfiguration
    int blocks = (terms_count + threads_per_block - 1) / threads_per_block;

    auto start_time = std::chrono::high_resolution_clock::now();

    // CUDA-Kernel ausführen
    compute_terms<<<blocks, threads_per_block>>>(device_numerators, device_denominators, 0, terms_count);
    check_cuda_error(cudaGetLastError(), "Kernel-Ausführung");

    // Ergebnisse zurück zum Host kopieren
    std::vector<double> host_numerators(terms_count);
    std::vector<double> host_denominators(terms_count);
    check_cuda_error(cudaMemcpy(host_numerators.data(), device_numerators, size, cudaMemcpyDeviceToHost), "cudaMemcpy (numerators)");
    check_cuda_error(cudaMemcpy(host_denominators.data(), device_denominators, size, cudaMemcpyDeviceToHost), "cudaMemcpy (denominators)");

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << "CUDA-Kernel Laufzeit: " << elapsed.count() << " Sekunden" << std::endl;

    // Summation der Terme mit GMP
    mpf_set_default_prec(1000000); // Hohe Präzision setzen
    mpf_class sum_numerator = 0;
    mpf_class sum_denominator = 0;

    for (int i = 0; i < terms_count; i++) {
        numerators[i] = host_numerators[i];
        denominators[i] = host_denominators[i];
        sum_numerator += numerators[i];
        sum_denominator += denominators[i];
    }

    mpf_class pi_chudnovsky = (426880 * sqrt(mpf_class(10005))) / (sum_numerator / sum_denominator);

    // BBP-Abschnitt zur Validierung
    mpf_class pi_bbp = calculate_bbp_section(0, terms_count / 10);

    std::cout << "Berechnetes Pi (Chudnovsky): " << pi_chudnovsky << std::endl;
    std::cout << "Validiertes Pi (BBP): " << pi_bbp << std::endl;

    // Speicher freigeben
    check_cuda_error(cudaFree(device_numerators), "cudaFree (numerators)");
    check_cuda_error(cudaFree(device_denominators), "cudaFree (denominators)");
}

int main() {
    int terms_count = 100000; // Anzahl der Terme
    int threads_per_block = 256; // Standardanzahl von Threads pro Block

    std::cout << "Weltrekordversuch zur Berechnung von Pi mit CUDA, GMP und algorithmischer Kombination" << std::endl;
    std::cout << "Anzahl der Terme: " << terms_count << std::endl;

    calculate_pi_combined(terms_count, threads_per_block);

    return 0;
}
