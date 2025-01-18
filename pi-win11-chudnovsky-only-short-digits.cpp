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
#include <windows.h>
#include <psapi.h>
#include <mutex>

using namespace std;
namespace fs = std::filesystem;

mutex io_mutex; // Mutex for thread-safe I/O operations
ofstream log_file; // Log file for logging errors and steps
bool verbose = false; // Verbose mode flag

void logMessage(const string& message, const string& level = "INFO") {
    string log_entry = "[" + level + "] " + message;
    cout << log_entry << endl;
    if (log_file.is_open()) {
        log_file << log_entry << endl;
        log_file.flush();
    }
}

void openLogFile() {
    log_file.open("error-log.txt", ios::trunc); // Change ios::app to ios::trunc to clear the log file
    if (!log_file.is_open()) {
        cerr << "Error: Unable to open log file." << endl;
        throw runtime_error("Error: Unable to open log file.");
    }
}

void closeLogFile() {
    if (log_file.is_open()) {
        logMessage("Log file closed successfully.", "INFO");
        log_file.close();
    }
}

void logVerbose(const string& message) {
    logMessage(message, "INFO");
}

// Memory Check
size_t getAvailableRAM() {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (!GlobalMemoryStatusEx(&memStatus)) {
        logVerbose("Error: Unable to get available RAM.");
        throw runtime_error("Error: Unable to get available RAM.");
    }
    return memStatus.ullAvailPhys;
}

size_t getUsedRAM() {
    PROCESS_MEMORY_COUNTERS_EX memInfo;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memInfo, sizeof(memInfo))) {
        logVerbose("Error: Unable to get used RAM.");
        throw runtime_error("Error: Unable to get used RAM.");
    }
    return memInfo.WorkingSetSize; // Returns memory in bytes
}

size_t getAvailableDiskSpace(const string& path) {
    fs::space_info si = fs::space(path);
    return si.available;
}

int getAvailableCores() {
    return thread::hardware_concurrency();
}

string getExecutableDirectory() {
    char buffer[MAX_PATH];
    if (GetModuleFileNameA(nullptr, buffer, MAX_PATH) == 0) {
        logVerbose("Error: Unable to get executable directory.");
        throw runtime_error("Error: Unable to get executable directory.");
    }
    return fs::path(buffer).parent_path().string();
}

struct PQT {
    mpz_class P, Q;
    mpf_class T;
};

// Function to save a chunk to file
void saveChunkToFile(const PQT& chunk, const string& base_filename) {
    lock_guard<mutex> lock(io_mutex); // Ensure thread-safe file access

    string p_file_path = base_filename + "_p-chunk.txt";
    string q_file_path = base_filename + "_q-chunk.txt";
    string t_file_path = base_filename + "_t-chunk.txt";

    ofstream p_file(p_file_path, ios::app);
    if (!p_file.is_open()) {
        logVerbose("Error: Could not create file " + p_file_path);
        throw runtime_error("Could not create file " + p_file_path);
    }
    p_file << chunk.P << "\n";
    p_file.close();
    if (!fs::exists(p_file_path)) {
        logVerbose("Error: File " + p_file_path + " does not exist after writing.");
    } else {
        logVerbose("Successfully created file: " + p_file_path);
    }

    ofstream q_file(q_file_path, ios::app);
    if (!q_file.is_open()) {
        logVerbose("Error: Could not create file " + q_file_path);
        throw runtime_error("Could not create file " + q_file_path);
    }
    q_file << chunk.Q << "\n";
    q_file.close();
    if (!fs::exists(q_file_path)) {
        logVerbose("Error: File " + q_file_path + " does not exist after writing.");
    } else {
        logVerbose("Successfully created file: " + q_file_path);
    }

    ofstream t_file(t_file_path, ios::app);
    if (!t_file.is_open()) {
        logVerbose("Error: Could not create file " + t_file_path);
        throw runtime_error("Could not create file " + t_file_path);
    }
    t_file << chunk.T << "\n";
    t_file.close();
    if (!fs::exists(t_file_path)) {
        logVerbose("Error: File " + t_file_path + " does not exist after writing.");
    } else {
        logVerbose("Successfully created file: " + t_file_path);
    }
}

// Function to clear memory used by GMP objects
void clearPQT(PQT& pqt) {
    if (pqt.P.get_mpz_t()) mpz_clear(pqt.P.get_mpz_t());
    if (pqt.Q.get_mpz_t()) mpz_clear(pqt.Q.get_mpz_t());
    if (pqt.T.get_mpf_t()) mpf_clear(pqt.T.get_mpf_t());
}

class Chudnovsky {
    mpz_class A, B, C, C3_24;
    mpf_class D, E;
    int DIGITS, N;
    size_t MAX_CHUNK_SIZE;
    int NUM_THREADS;

    void normalizePQT(PQT& pqt);

public:
    Chudnovsky(int digits, size_t chunk_size, int num_threads, size_t usable_ram);
    PQT computePQT(size_t n1, size_t n2);
    void computePi(size_t available_disk, const string& output_directory);
};

Chudnovsky::Chudnovsky(int digits, size_t chunk_size, int num_threads, size_t usable_ram) {
    if (digits <= 0) {
        logVerbose("Error: Number of digits must be positive.");
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
    MAX_CHUNK_SIZE = min((size_t)(usable_ram / 50), (size_t)200000); // Adjusted chunk size
    NUM_THREADS = num_threads;
    mpf_set_default_prec(DIGITS * 4); // Set precision for all mpf_class variables
    logVerbose("Chudnovsky initialized with " + to_string(digits) + " digits, chunk size " + to_string(MAX_CHUNK_SIZE) + ", and " + to_string(num_threads) + " threads.");
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
            logVerbose("Error: Division by zero detected in computePQT. Adjusting values.");
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
            logVerbose("Error: Division by zero detected during merge. Adjusting values.");
            res.Q = 1; // Avoid division by zero
        }

        normalizePQT(res);
        clearPQT(left);
        clearPQT(right);
        return res;
    }
}

void Chudnovsky::computePi(size_t available_disk, const string& output_directory) {
    logVerbose("Calculating Pi with " + to_string(DIGITS) + " digits.");

    if (DIGITS <= 0) {
        logVerbose("Error: Number of digits must be greater than 0.");
        throw invalid_argument("Error: Number of digits must be greater than 0.");
    }

    if (available_disk < 1024 * 1024 * 100) { // Minimum 100 MB of disk space
        logVerbose("Error: Insufficient disk space for computation.");
        throw runtime_error("Error: Insufficient disk space for computation.");
    }

    mpz_class P_total = 1, Q_total = 1;
    mpf_class T_total = 0;

    if (!fs::exists(output_directory)) {
        fs::create_directories(output_directory);
        logVerbose("Created output directory: " + output_directory);
    }
    string base_filename = output_directory + "\\chunk";
    logVerbose("Base filename for chunks: " + base_filename);

    for (size_t i = 0; i < N; i += MAX_CHUNK_SIZE) {
        size_t end = min(i + MAX_CHUNK_SIZE, static_cast<size_t>(N));
        logVerbose("Processing terms " + to_string(i) + " to " + to_string(end) + "...");

        // Check available disk space
        if (getAvailableDiskSpace(output_directory) < 1024 * 1024 * 10) { // Minimum 10 MB of disk space
            logVerbose("Error: Insufficient disk space during computation.");
            throw runtime_error("Error: Insufficient disk space during computation.");
        }

        // Check used RAM
        if (getUsedRAM() > getAvailableRAM() - MAX_CHUNK_SIZE) {
            logVerbose("Error: Insufficient RAM during computation.");
            throw runtime_error("Error: Insufficient RAM during computation.");
        }

        PQT chunk = computePQT(i, end);

        // Validate chunk before saving
        if (chunk.P == 0 || chunk.Q == 0 || chunk.T == 0) {
            logVerbose("Error: Invalid chunk detected. Skipping file write.");
            continue;
        }

        saveChunkToFile(chunk, base_filename);

        P_total *= chunk.P;
        Q_total *= chunk.Q;
        T_total += chunk.T;
        clearPQT(chunk);

        if (Q_total == 0) {
            logVerbose("Error: Q_total became zero during computation. Adjusting values.");
            Q_total = 1; // Avoid division by zero
        }

        logVerbose("Progress: " + to_string((end * 100) / N) + "% completed.");
        logVerbose("Used RAM: " + to_string(getUsedRAM() / (1024 * 1024)) + " MB");
    }

    mpf_class pi;
    if ((mpf_class(A) * mpf_class(Q_total) + T_total) == 0) {
        logVerbose("Error: Division by zero in final Pi computation. Adjusting denominator.");
        pi = 0;
    } else {
        pi = D * E * mpf_class(Q_total) / (mpf_class(A) * mpf_class(Q_total) + T_total);
    }

    string P_total_str = P_total.get_str();
    string Q_total_str = Q_total.get_str();
    char T_total_str[1024], pi_str[1024];
    gmp_sprintf(T_total_str, "%.Ff", T_total.get_mpf_t());
    gmp_sprintf(pi_str, "%.Ff", pi.get_mpf_t());

    logVerbose("Final values: P_total=" + P_total_str + ", Q_total=" + Q_total_str + ", T_total=" + string(T_total_str));
    logVerbose("Calculated Pi: " + string(pi_str));

    string output_file = output_directory + "\\pi.txt";
    ofstream pi_file(output_file, ios::trunc);
    if (!pi_file.is_open()) {
        logVerbose("Error: Could not open the file " + output_file + " for writing.");
        throw runtime_error("Error: Could not open the file " + output_file + " for writing.");
    }

    pi_file.precision(DIGITS);
    pi_file << fixed << pi << endl;
    if (pi_file.fail()) {
        logVerbose("Error: Failed to write Pi to the file.");
        throw runtime_error("Error: Failed to write Pi to the file.");
    }
    pi_file.close();
    logVerbose("Pi successfully saved to '" + output_file + "'.");

    // Verify that the files exist
    if (fs::exists(output_file)) {
        logVerbose("Verified that the file " + output_file + " exists.");
    } else {
        logVerbose("Error: The file " + output_file + " does not exist.");
    }

    logVerbose("Calculation of Pi completed successfully.");
}

int main(int argc, char* argv[]) {
    openLogFile();
    logVerbose("This program uses MPFR and GMP libraries for high-precision arithmetic.");
    logVerbose("Copyright (C) 2024 Lino Casu. All rights reserved.");

    // Check for verbose flag
    if (argc > 1 && string(argv[1]) == "--verbose") {
        verbose = true;
    }

    try {
        string exe_directory = getExecutableDirectory();
        string drive_path = fs::path(exe_directory).root_path().string();

        if (!fs::exists(exe_directory)) {
            logVerbose("Error: Executable directory does not exist: " + exe_directory);
            throw runtime_error("Executable directory does not exist.");
        }

        size_t available_ram = getAvailableRAM();
        size_t available_disk = getAvailableDiskSpace(drive_path);

        logVerbose("Available RAM: " + to_string(available_ram / (1024 * 1024)) + " MB");
        logVerbose("Available Disk Space: " + to_string(available_disk / (1024 * 1024)) + " MB");

        size_t usable_ram = (available_ram * 70) / 100; // Use 70% of available RAM for safety
        logVerbose("Maximum usable RAM (70%): " + to_string(usable_ram / (1024 * 1024)) + " MB");

        if (usable_ram < 1024 * 1024 * 500) { // Minimum 500 MB RAM
            logVerbose("Error: Insufficient RAM for computation.");
            throw runtime_error("Error: Insufficient RAM for computation.");
        }

        // Estimate digits based on usable RAM
        size_t max_digits_ram = (usable_ram / 10) * 1000;

        // Estimate digits based on available disk space
        // Assume each digit takes 1 byte and add some overhead for formatting
        size_t max_digits_disk = (available_disk / 2); // Use half of the available disk space for safety

        size_t digits = min(max_digits_ram, max_digits_disk); // Use the minimum of the two estimates

        logVerbose("The maximum digits that can be computed with available resources is approximately: " + to_string(digits) + " digits.");

        string user_input;
        cout << "Do you want to proceed with this calculation? (yes/no): ";
        cin >> user_input;

        if (user_input != "yes") {
            cout << "Enter the number of digits you want to compute (up to " << digits << "): ";
            cin >> digits;

            if (digits <= 0 || digits > min(max_digits_ram, max_digits_disk)) {
                logVerbose("Error: Invalid number of digits entered. Exiting.");
                throw invalid_argument("Error: Invalid number of digits entered. Exiting.");
            }

            logVerbose("Proceeding with " + to_string(digits) + " digits.");
        }

        size_t chunk_size = min(usable_ram / 50, (size_t)200000); // Adjusted chunk size
        logVerbose("Adjusted chunk size: " + to_string(chunk_size));

        logVerbose("Executable directory: " + exe_directory);

        Chudnovsky calculator(digits, chunk_size, getAvailableCores(), usable_ram);
        calculator.computePi(available_disk, exe_directory);

    } catch (const exception& e) {
        logVerbose("Error: " + string(e.what()));
        cerr << "Error: " << e.what() << endl;
        closeLogFile();
        return -1;
    }

    closeLogFile();
    return 0;
}
