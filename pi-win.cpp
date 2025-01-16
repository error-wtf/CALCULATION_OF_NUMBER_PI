#include <iostream>
#include <fstream>
#include <gmp.h>
#include <gmpxx.h>
#include <mpfr.h>
#include <thread>
#include <stdexcept>
#include <cmath>
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

// Memory Check
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

size_t getUsedRAM() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX memInfo;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memInfo, sizeof(memInfo));
    return memInfo.WorkingSetSize; // Returns memory in bytes
#else
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss * 1024; // Returns memory in bytes
#endif
}

size_t getAvailableDiskSpace(const string& path = ".") {
    fs::space_info si = fs::space(path);
    return si.available;
}

int getAvailableCores() {
    return thread::hardware_concurrency();
}

string getExecutableDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
#else
    char buffer[PATH_MAX];
    readlink("/proc/self/exe", buffer, PATH_MAX);
#endif
    return fs::path(buffer).parent_path().string();
}

struct PQT {
    mpz_class P, Q;
    mpf_class T;
};

// Function to save a chunk to file
void saveChunkToFile(const PQT& chunk, const string& filename) {
    ofstream outfile(filename, ios::app);
    if (outfile.is_open()) {
        outfile << chunk.P << "\n" << chunk.Q << "\n" << chunk.T << "\n";
        outfile.close();
    } else {
        cerr << "Error: Unable to open file " << filename << " for writing.\n";
    }
}

// Function to clear memory used by GMP objects
void clearPQT(PQT& pqt) {
    mpz_clear(pqt.P.get_mpz_t());
    mpz_clear(pqt.Q.get_mpz_t());
    mpf_clear(pqt.T.get_mpf_t());
}

class Chudnovsky {
    mpz_class A, B, C, C3_24;
    mpf_class D, E;
    int DIGITS, N;
    size_t MAX_CHUNK_SIZE;
    int NUM_THREADS;

    void normalizePQT(PQT& pqt);

public:
    Chudnovsky(int digits, size_t chunk_size, int num_threads);
    PQT computePQT(size_t n1, size_t n2);
    void computePi(size_t available_disk, const string& output_directory);
};

Chudnovsky::Chudnovsky(int digits, size_t chunk_size, int num_threads) {
    if (digits <= 0) {
        throw invalid_argument("Number of digits must be positive.");
    }
    DIGITS = digits;
    A = 13591409;
    B = 545140134;
    C = 640320;
    C3_24 = (C * C * C) / 24;
    D = 426880;
    E = sqrt(mpf_class(10005));
    N = static_cast<int>(ceil(DIGITS / log10(640320.0 * 640320.0 * 640320.0 / 24)));
    MAX_CHUNK_SIZE = chunk_size;
    NUM_THREADS = num_threads;
    mpf_set_default_prec(DIGITS * 4); // Set precision for all mpf_class variables
}

void Chudnovsky::normalizePQT(PQT& pqt) {
    mpf_class scale_factor;
    scale_factor = sqrt(mpf_class(pqt.P) * mpf_class(pqt.Q));
    pqt.P /= scale_factor.get_ui();
    pqt.Q /= scale_factor.get_ui();
    pqt.T /= scale_factor;
}

PQT Chudnovsky::computePQT(size_t n1, size_t n2) {
    if (n1 + 1 == n2) {
        PQT res;
        mpz_class n2_mpz = mpz_class(static_cast<long unsigned int>(n2));
        res.P = (2 * n2_mpz - 1) * (6 * n2_mpz - 1) * (6 * n2_mpz - 5);
        res.Q = C3_24 * n2_mpz * n2_mpz * n2_mpz;
        res.T = mpf_class(res.P) * mpf_class(A + B * n2_mpz) / mpf_class(res.Q);

        if (res.Q == 0) {
            cerr << "Error: Division by zero detected in computePQT. Adjusting values." << endl;
            res.Q = 1; // Avoid division by zero
        }

        if (n2 % 2 != 0) res.T = -res.T;
        normalizePQT(res);
        return res;
    } else {
        size_t mid = (n1 + n2) / 2;
        PQT left, right;
        thread left_thread([&]() { left = computePQT(n1, mid); });
        thread right_thread([&]() { right = computePQT(mid, n2); });
        left_thread.join();
        right_thread.join();
        PQT res;
        res.P = left.P * right.P;
        res.Q = left.Q * right.Q;
        res.T = left.T + right.T;
        
        if (res.Q == 0) {
            cerr << "Error: Division by zero detected during merge. Adjusting values." << endl;
            res.Q = 1; // Avoid division by zero
        }

        normalizePQT(res);
        clearPQT(left);
        clearPQT(right);
        return res;
    }
}

void Chudnovsky::computePi(size_t available_disk, const string& output_directory) {
    cout << "Calculating Pi with " << DIGITS << " digits." << endl;

    if (DIGITS <= 0) {
        cerr << "Error: Number of digits must be greater than 0." << endl;
        return;
    }

    if (available_disk < 1024 * 1024 * 100) { // Minimum 100 MB of disk space
        cerr << "Error: Insufficient disk space for computation." << endl;
        return;
    }

    mpz_class P_total = 1, Q_total = 1;
    mpf_class T_total = 0;

    // Removed save directory creation as requested
    string output_file = getExecutableDirectory() + "\\pi.txt";

    for (size_t i = 0; i < N; i += MAX_CHUNK_SIZE) {
        size_t end = min(i + MAX_CHUNK_SIZE, static_cast<size_t>(N));
        cout << "Processing terms " << i << " to " << end << "..." << endl;

        PQT chunk = computePQT(i, end);
        saveChunkToFile(chunk, "chunk_results.txt");

        P_total *= chunk.P;
        Q_total *= chunk.Q;
        T_total += chunk.T;
        clearPQT(chunk);

        if (Q_total == 0) {
            cerr << "Error: Q_total became zero during computation. Adjusting values." << endl;
            Q_total = 1; // Avoid division by zero
        }

        cout << "Progress: " << ((end * 100) / N) << "% completed." << endl;
        cout << "Used RAM: " << getUsedRAM() / (1024 * 1024) << " MB\n";
    }

    mpf_class pi;
    if ((mpf_class(A) * mpf_class(Q_total) + T_total) == 0) {
        cerr << "Error: Division by zero in final Pi computation. Adjusting denominator." << endl;
        pi = 0;
    } else {
        pi = D * E * mpf_class(Q_total) / (mpf_class(A) * mpf_class(Q_total) + T_total);
    }

    cout << "Final values: P_total=" << P_total << ", Q_total=" << Q_total << ", T_total=" << T_total << endl;
    cout << "Calculated Pi: " << pi << endl;

    ofstream pi_file(output_file, ios::trunc);
    if (!pi_file.is_open()) {
        cerr << "Error: Could not open the file " << output_file << " for writing." << endl;
        return;
    }

    pi_file.precision(DIGITS);
    pi_file << fixed << pi << endl;
    if (pi_file.fail()) {
        cerr << "Error: Failed to write Pi to the file." << endl;
        return;
    }
    pi_file.close();
    cout << "Pi successfully saved to '" << output_file << "'." << endl;
}

int main() {
    cout << "This program uses MPFR and GMP libraries for high-precision arithmetic." << endl;
    cout << "Copyright (C) 2024 Lino Casu. All rights reserved." << endl;
    try {
        size_t available_ram = getAvailableRAM();
        size_t available_disk = getAvailableDiskSpace();

        cout << "Available RAM: " << available_ram / (1024 * 1024) << " MB" << endl;
        cout << "Available Disk Space: " << available_disk / (1024 * 1024) << " MB" << endl;

        size_t usable_ram = (available_ram * 80) / 100;
        cout << "Maximum usable RAM (80%): " << usable_ram / (1024 * 1024) << " MB" << endl;

        if (usable_ram < 1024 * 1024 * 500) { // Minimum 500 MB RAM
            cerr << "Error: Insufficient RAM for computation." << endl;
            return -1;
        }

        size_t digits = (usable_ram / 10) * 1000; // Estimate digits based on usable RAM

        cout << "The maximum digits that can be computed with available resources is approximately: " << digits << " digits." << endl;

        string user_input;
        cout << "Do you want to proceed with this calculation? (yes/no): ";
        cin >> user_input;

        if (user_input != "yes") {
            cout << "Enter the number of digits you want to compute (up to " << digits << "): ";
            cin >> digits;

            if (digits <= 0 || digits > (usable_ram / 10) * 1000) {
                cerr << "Error: Invalid number of digits entered. Exiting." << endl;
                return -1;
            }

            cout << "Proceeding with " << digits << " digits." << endl;
        }

        size_t chunk_size = usable_ram / 10; // 10% of usable RAM

        string exe_directory = getExecutableDirectory();
        cout << "Executable directory: " << exe_directory << endl;

        Chudnovsky calculator(digits, chunk_size, getAvailableCores());
        calculator.computePi(available_disk, exe_directory);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }

    return 0;
}
