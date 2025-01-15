#include <iostream>
#include <fstream>
#include <gmpxx.h>
#include <cmath>
#include <thread>
#include <mutex>
#include <cstring> // For mpf_get_str

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

// Copyright (c) 2025 Lino Casu and Akira (AI)

// Mutex for thread-safe logging
std::mutex log_mutex;

// Header function for program details
void displayHeader() {
    cout << "----------------------------------------" << endl;
    cout << "Optimized Pi Calculation with GMP" << endl;
    cout << "Copyright (c) 2025 Lino Casu and Akira (AI)" << endl;
    cout << "----------------------------------------" << endl << endl;
}

// Function to display program settings
void displaySettings(size_t ram_mb, size_t file_size_gb, int digits, int cores) {
    cout << "Settings:" << endl;
    cout << "  Available RAM: " << ram_mb << " MB" << endl;
    cout << "  Maximum file size: " << file_size_gb << " GB" << endl;
    cout << "  Digits to compute: " << digits << endl;
    cout << "  CPU cores used: " << cores << endl;
    cout << endl;
}

// Progress display with logging
void displayProgress(int completed_terms, int total_terms) {
    double progress = (static_cast<double>(completed_terms) / total_terms) * 100.0;
    {
        std::lock_guard<std::mutex> lock(log_mutex);
        cout << "Progress: " << progress << "% completed ("
             << completed_terms << " of " << total_terms << " terms)" << endl;

        ofstream log_file("progress.log", ios_base::app);
        if (log_file.is_open()) {
            log_file << "Progress: " << progress << "% completed ("
                     << completed_terms << " of " << total_terms << " terms)" << endl;
        }
    }
}

// Chudnovsky class for Pi calculation
class Chudnovsky {
    mpz_class A, B, C, C3_24, D, E;
    int DIGITS, TERMS;
    size_t MAX_CHUNK_SIZE;

public:
    Chudnovsky(int digits, size_t chunk_size);
    mpf_class computePi();

    static size_t getAvailableRAM();
    static size_t calculateChunkSize(size_t available_ram);
};

Chudnovsky::Chudnovsky(int digits, size_t chunk_size) {
    DIGITS = digits;
    TERMS = digits / 14; // Approximation based on convergence rate
    A = 13591409;
    B = 545140134;
    C = 640320;
    C3_24 = C * C * C / 24;
    D = 426880;
    E = 10005;
    MAX_CHUNK_SIZE = chunk_size;
}

mpf_class Chudnovsky::computePi() {
    mpf_set_default_prec(DIGITS * log2(10));

    mpf_class sum = 0.0;
    mpf_class term, P, Q, T;

    for (int k = 0; k < TERMS; ++k) {
        // Calculate P
        mpz_class Pz = 1;
        for (int i = 1; i <= 6 * k; ++i) Pz *= i;
        mpf_set_z(P.get_mpf_t(), Pz.get_mpz_t());

        // Calculate Q
        mpz_class Qz = 1;
        for (int i = 1; i <= 3 * k; ++i) Qz *= i;
        Qz = Qz * Qz * Qz; // Q^3
        mpf_set_z(Q.get_mpf_t(), Qz.get_mpz_t());

        // Calculate T
        T = A + B * k;
        T *= P;

        // Add term to sum
        term = T / Q;
        if (k % 2 == 1) term = -term; // Alternate signs
        sum += term;

        displayProgress(k + 1, TERMS);
    }

    // Final calculation of Pi
    mpf_class sqrtC = sqrt((mpf_class)E);
    mpf_class pi = D * sqrtC / sum;

    return pi;
}

size_t Chudnovsky::getAvailableRAM() {
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

size_t Chudnovsky::calculateChunkSize(size_t available_ram) {
    size_t usable_ram = static_cast<size_t>(available_ram * 0.8);
    size_t term_memory = 20480;
    return usable_ram / term_memory;
}

int main() {
    try {
        displayHeader();

        size_t available_ram = Chudnovsky::getAvailableRAM();
        size_t ram_mb = available_ram / (1024 * 1024);
        size_t chunk_size = Chudnovsky::calculateChunkSize(available_ram);

        cout << "Enter the number of digits to calculate: ";
        int digits;
        cin >> digits;

        Chudnovsky calculator(digits, chunk_size);
        mpf_class pi = calculator.computePi();

        // Save result to file
        char* pi_str = nullptr;
        mp_exp_t exp;
        size_t precision = digits + 2; // Additional digits for precision
        pi_str = mpf_get_str(nullptr, &exp, 10, precision, pi.get_mpf_t());

        if (pi_str) {
            ofstream pi_file("pi.txt");
            if (!pi_file) {
                cerr << "Error opening file to save Pi." << endl;
                return -1;
            }
            pi_file << pi_str[0] << "." << (pi_str + 1) << "e" << exp << endl;
            free(pi_str); // Free memory allocated by GMP
            cout << "Pi calculation complete. Result saved to 'pi.txt'." << endl;
        } else {
            cerr << "Error converting Pi to string." << endl;
            return -1;
        }

    } catch (exception &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }

    return 0;
}

