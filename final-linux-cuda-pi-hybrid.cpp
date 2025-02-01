#include <unistd.h>
#include <iostream>
#include <cmath>
#include <cuda_runtime.h>
#include <gmp.h>
#include <gmpxx.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdio>
#include <cstdlib>
#include <fstream>
namespace fs = std::filesystem;

// ------------------------------
// Funktion: Ermittlung des verfügbaren Hauptspeichers (RAM)
// ------------------------------
size_t getAvailableRAM() {
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return static_cast<size_t>(pages) * static_cast<size_t>(page_size);
}

// ------------------------------
// Konstanten für die Chudnovsky-Methode (GPU)
// ------------------------------
// Dynamische Initialisierung ist für __constant__ nicht erlaubt –
// daher werden hier die Werte direkt gesetzt.
__constant__ double A = 13591409.0;
__constant__ double B = 545140134.0;
__constant__ double C = 640320.0;
// 640320^3 = 262537412640768000, geteilt durch 24 ergibt:
__constant__ double C3_24 = 10939058860032000.0;

// ------------------------------
// CUDA-Kernel zur Berechnung der Chudnovsky-Terme (GPU-Version)
// Berechnet den k-ten Term gemäß der Formel:
//   T_k = ((6k)! * (A + B*k)) / ((3k)! * (k!)^3 * (-C3_24)^k)
__global__ void compute_terms(double *numerators, double *denominators, int start, int n) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    if (idx < n) {
        int k = start + idx;
        numerators[idx] = tgamma(6 * k + 1.0) * (A + B * k);
        denominators[idx] = (tgamma(3 * k + 1.0) * pow(tgamma(k + 1.0), 3)) * pow(-C3_24, k);
    }
}

// ------------------------------
// Hilfsfunktion zur Fehlerprüfung bei CUDA-Aufrufen
// ------------------------------
inline void check_cuda_error(cudaError_t err, const char* msg) {
    if (err != cudaSuccess) {
        std::cerr << "CUDA Error (" << msg << "): " 
                  << cudaGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }
}

// ------------------------------
// Funktion: Berechnung eines Abschnitts von π mittels der BBP-Formel (CPU-Version mit GMP)
// ------------------------------
mpf_class calculate_bbp_section(int start, int terms_count) {
    mpf_set_default_prec(1000000);
    mpf_class sum = 0;
    for (int k = start; k < start + terms_count; ++k) {
        mpf_class term = (4.0 / (8.0 * k + 1.0)
                        - 2.0 / (8.0 * k + 4.0)
                        - 1.0 / (8.0 * k + 5.0)
                        - 1.0 / (8.0 * k + 6.0));
        term /= pow(16.0, k);
        sum += term;
    }
    return sum;
}

// ------------------------------
// Funktion: Normalisierung des BBP-Ergebnisses (multipliziert mit 16^(-bbp_terms))
// ------------------------------
mpf_class normalize_bbp(const mpf_class &bbp, int bbp_terms) {
    mpf_set_default_prec(1000000);
    mpf_class factor;
    mpf_pow_ui(factor.get_mpf_t(), mpf_class(16).get_mpf_t(), bbp_terms);
    factor = 1 / factor;
    return bbp * factor;
}

// ------------------------------
// GPU-basierte Berechnung der Chudnovsky-Methode (double-Precision)
// ------------------------------
mpf_class compute_chudnovsky_gpu(int terms_count, int threads_per_block) {
    size_t size = terms_count * sizeof(double);
    std::vector<double> host_numerators(terms_count);
    std::vector<double> host_denominators(terms_count);
    
    double *device_numerators = nullptr, *device_denominators = nullptr;
    check_cuda_error(cudaMalloc(&device_numerators, size), "cudaMalloc (numerators)");
    check_cuda_error(cudaMalloc(&device_denominators, size), "cudaMalloc (denominators)");
    
    int blocks = (terms_count + threads_per_block - 1) / threads_per_block;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    compute_terms<<<blocks, threads_per_block>>>(device_numerators, device_denominators, 0, terms_count);
    check_cuda_error(cudaGetLastError(), "Kernel-Ausführung");
    check_cuda_error(cudaDeviceSynchronize(), "cudaDeviceSynchronize");
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << "CUDA-Kernel Laufzeit: " << elapsed.count() << " Sekunden" << std::endl;
    
    std::vector<double> host_num(terms_count);
    std::vector<double> host_den(terms_count);
    check_cuda_error(cudaMemcpy(host_num.data(), device_numerators, size, cudaMemcpyDeviceToHost), "cudaMemcpy (numerators)");
    check_cuda_error(cudaMemcpy(host_den.data(), device_denominators, size, cudaMemcpyDeviceToHost), "cudaMemcpy (denominators)");
    
    check_cuda_error(cudaFree(device_numerators), "cudaFree (numerators)");
    check_cuda_error(cudaFree(device_denominators), "cudaFree (denominators)");
    
    mpf_set_default_prec(1000000);
    mpf_class S = 0;
    for (int i = 0; i < terms_count; i++) {
        mpf_class term = mpf_class(host_num[i]) / mpf_class(host_den[i]);
        S += term;
    }
    mpf_class pi_chudnovsky = (426880 * sqrt(mpf_class(10005))) / S;
    return pi_chudnovsky;
}

// ------------------------------
// CPU-basierte multipräzise Berechnung der Chudnovsky-Methode mit GMP (Chunking-Version)
// Diese Funktion teilt die Iterationen in Chunks auf, schreibt den aktuellen Summenwert
// in die Datei "chunk.txt" und liefert schließlich π = C / S zurück.
mpf_class compute_chudnovsky_cpu(int iterations, int chunk_size) {
    // Lösche ggf. vorhandene "chunk.txt"
    std::remove("chunk.txt");
    
    mpf_set_default_prec(1000000);
    mpf_class C_val = 426880 * sqrt(mpf_class(10005));
    mpz_class M = 1;
    mpz_class K = 6;
    mpz_class X = 1;
    mpf_class L = 13591409;
    mpf_class S = L;
    
    for (int k = 1; k < iterations; ++k) {
        mpz_class Kcubed;
        mpz_pow_ui(Kcubed.get_mpz_t(), K.get_mpz_t(), 3);
        mpz_class temp = Kcubed - 16 * K;
        M = (M * temp) / (k * k * k);
        X *= mpz_class("-262537412640768000");
        L += 545140134;
        mpf_class term = (mpf_class(M) * L) / mpf_class(X);
        S += term;
        K += 12;
        
        if (k % chunk_size == 0) {
            std::ofstream chunkFile("chunk.txt", std::ios::app);
            if (chunkFile.is_open()) {
                char buffer[256];
                gmp_snprintf(buffer, sizeof(buffer), "%.*Ff", 50, S.get_mpf_t());
                chunkFile << "Chunk " << k << ": S = " << buffer << std::endl;
                chunkFile.close();
            }
        }
    }
    return C_val / S;
}

// ------------------------------
// Hybrid-Funktion: Wählt zwischen GPU-basierter (hybrider) und CPU-basierter multipräziser Berechnung
// basierend auf der gewünschten Anzahl Nachkommastellen.
// Für niedrige Präzision (bis ca. 700 Nachkommastellen) wird die hybride Methode verwendet,
// ansonsten die CPU-basierte multipräzise Berechnung (mit Chunking).
void calculate_pi_hybrid(int desiredDecimalPlaces, int terms_count, int threads_per_block, int bbp_terms, int chunk_size) {
    std::string resultStr;
    if (desiredDecimalPlaces > 700) {
        std::cout << "\nHohe Präzision gewünscht: " << desiredDecimalPlaces << " Nachkommastellen." << std::endl;
        int iterations = desiredDecimalPlaces / 14 + 1;
        if (iterations < 1) iterations = 1;
        mpf_set_default_prec(desiredDecimalPlaces * 4);
        mpf_class pi_cpu = compute_chudnovsky_cpu(iterations, chunk_size);
        int outputPrecision = desiredDecimalPlaces;
        size_t bufferSize = outputPrecision + 10;
        char* buffer = new char[bufferSize];
        gmp_snprintf(buffer, bufferSize, "%.*Ff", outputPrecision, pi_cpu.get_mpf_t());
        resultStr = buffer;
        std::cout << "Berechnetes π (CPU, multipräzise): " << buffer << std::endl;
        delete[] buffer;
    } else {
        std::cout << "\nNiedrige Präzision (bis ca. 700 Nachkommastellen) wird mittels hybrider Methode (GPU + CPU) berechnet." << std::endl;
        mpf_class pi_hybrid = compute_chudnovsky_gpu(terms_count, threads_per_block)
                               + normalize_bbp(calculate_bbp_section(0, bbp_terms), bbp_terms);
        int outputPrecision = desiredDecimalPlaces;
        size_t bufferSize = outputPrecision + 10;
        char* buffer = new char[bufferSize];
        gmp_snprintf(buffer, bufferSize, "%.*Ff", outputPrecision, pi_hybrid.get_mpf_t());
        resultStr = buffer;
        std::cout << "Hybrides π: " << buffer << std::endl;
        delete[] buffer;
    }
    
    // Ergebnis zusätzlich in die Datei "pi.txt" schreiben
    std::ofstream outFile("pi.txt");
    if (outFile.is_open()) {
        outFile << resultStr;
        outFile.close();
        std::cout << "Ergebnis wurde in 'pi.txt' gespeichert." << std::endl;
    } else {
        std::cerr << "Fehler beim Öffnen von pi.txt zum Schreiben!" << std::endl;
    }
}

// ------------------------------
// Hauptfunktion
// Ermittelt anhand des verfügbaren Hauptspeichers (70% des RAM)
// und des freien Festplattenspeichers im aktuellen Verzeichnis
// ein maximales Limit an Nachkommastellen (heuristisch).
// Anschließend wird der Benutzer zur Eingabe aufgefordert.
int main() {
    size_t availableRam = getAvailableRAM();
    size_t mainRamLimit = static_cast<size_t>(availableRam * 0.70);
    size_t mainRamLimitMB = mainRamLimit / (1024 * 1024);
    // Heuristik: 2 MB pro 10 Nachkommastellen, geteilt durch Faktor 5 (um zusätzlichen Bedarf zu berücksichtigen)
    size_t heuristic_digits_ram = ((mainRamLimitMB / 2) * 10) / 5;
    
    fs::path currentPath = fs::current_path();
    auto diskSpace = fs::space(currentPath);
    size_t freeDiskBytes = diskSpace.free;
    size_t freeDiskMB = freeDiskBytes / (1024 * 1024);
    size_t heuristic_digits_disk = ((freeDiskMB / 2) * 10) / 5;
    
    size_t system_max_allowed = std::min(heuristic_digits_ram, heuristic_digits_disk);
    
    // Hier setzen wir das interne Maximum ausschließlich anhand der Systemressourcen.
    int max_allowed_digits = (int)system_max_allowed;
    
    std::cout << "Verfügbarer Hauptspeicher: " << availableRam / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Hauptspeicherlimit (70%): " << mainRamLimitMB << " MB" << std::endl;
    std::cout << "Freier Festplattenspeicher (akt. Verzeichnis): " << freeDiskMB << " MB" << std::endl;
    std::cout << "Maximale Nachkommastellen (internes Limit): " << max_allowed_digits << std::endl;
    
    std::cout << "Bitte geben Sie die gewünschte Anzahl Nachkommastellen für π ein (zwischen 1 und " 
              << max_allowed_digits << "): ";
    int desiredDecimalPlaces = 0;
    std::cin >> desiredDecimalPlaces;
    if (desiredDecimalPlaces < 1) {
        std::cerr << "Ungültige Eingabe. Setze Nachkommastellen auf 1." << std::endl;
        desiredDecimalPlaces = 1;
    } else if (desiredDecimalPlaces > max_allowed_digits) {
        std::cerr << "Eingabe überschreitet das Maximum. Setze auf " << max_allowed_digits << " Nachkommastellen." << std::endl;
        desiredDecimalPlaces = max_allowed_digits;
    }
    
    // Moduswahl: Bei double‑Precision (hybride Methode) verwenden wir maximal 700 Nachkommastellen.
    if (desiredDecimalPlaces > 700) {
        std::cout << "\nEs wird die CPU-basierte multipräzise Berechnung verwendet." << std::endl;
    } else {
        std::cout << "\nEs wird die hybride Methode (GPU + CPU) verwendet." << std::endl;
    }
    
    // Heuristik für die Chunk-Größe (z. B. wenn Hauptspeicherlimit in MB > 10000, dann größere Chunks)
    int chunk_size = 1000;
    if (mainRamLimitMB > 10000)
        chunk_size = 5000;
    
    // Moduswahl: ≤ 700 Nachkommastellen -> hybride Methode; > 700 -> CPU multipräzise
    if (desiredDecimalPlaces <= 700) {
        int terms_count = desiredDecimalPlaces / 14 + 1;
        if (terms_count < 1) terms_count = 1;
        int bbp_terms = 10;
        int threads_per_block = 256;
        
        std::cout << "\nWeltrekordversuch zur hybriden Berechnung von π (GPU + CPU)" << std::endl;
        std::cout << "Chudnovsky-Terme (GPU): " << terms_count << std::endl;
        std::cout << "BBP-Terme (CPU): " << bbp_terms << std::endl;
        
        calculate_pi_hybrid(desiredDecimalPlaces, terms_count, threads_per_block, bbp_terms, chunk_size);
    } else {
        std::cout << "\nHohe Präzision gewünscht: " << desiredDecimalPlaces << " Nachkommastellen." << std::endl;
        int iterations = desiredDecimalPlaces / 14 + 1;
        if (iterations < 1) iterations = 1;
        mpf_set_default_prec(desiredDecimalPlaces * 4);
        mpf_class pi_cpu = compute_chudnovsky_cpu(iterations, chunk_size);
        int outputPrecision = desiredDecimalPlaces;
        size_t bufferSize = outputPrecision + 10;
        char* buffer = new char[bufferSize];
        gmp_snprintf(buffer, bufferSize, "%.*Ff", outputPrecision, pi_cpu.get_mpf_t());
        std::cout << "Berechnetes π (CPU, multipräzise): " << buffer << std::endl;
        
        std::ofstream outFile("pi.txt");
        if (outFile.is_open()) {
            outFile << buffer;
            outFile.close();
            std::cout << "Ergebnis wurde in 'pi.txt' gespeichert." << std::endl;
        } else {
            std::cerr << "Fehler beim Öffnen von pi.txt zum Schreiben!" << std::endl;
        }
        
        delete[] buffer;
    }
    
    return 0;
}
