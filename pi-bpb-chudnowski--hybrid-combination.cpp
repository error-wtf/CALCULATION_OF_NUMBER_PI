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
#include <string>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

// Mutex for thread-safe operations
mutex io_mutex;

// Logging functions
ofstream log_file;
bool verbose = false;

void logMessage(const string& message, const string& level = "INFO") {
    string log_entry = "[" + level + "] " + message;
    cout << log_entry << endl;
    if (log_file.is_open()) {
        log_file << log_entry << endl;
        log_file.flush();
    } else {
        cerr << "Error: Unable to write to log file." << endl;
        throw runtime_error("Log file is not open.");
    }

    if (level == "ERROR") {
        throw runtime_error(log_entry);
    }
}

void openLogFile() {
    log_file.open("error-log.txt", ios::trunc);
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
    if (verbose) {
        logMessage(message, "DEBUG");
    }
}

// System Queries
size_t getAvailableRAM() {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (!GlobalMemoryStatusEx(&memStatus)) {
        logMessage("Error: Unable to get available RAM.", "ERROR");
    }
    return memStatus.ullAvailPhys;
}

size_t getUsedRAM() {
    PROCESS_MEMORY_COUNTERS_EX memInfo;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memInfo, sizeof(memInfo))) {
        logMessage("Error: Unable to get used RAM.", "ERROR");
    }
    return memInfo.WorkingSetSize;
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
        logMessage("Error: Unable to get executable directory.", "ERROR");
        throw runtime_error("Error: Unable to get executable directory.");
    }
    return fs::path(buffer).parent_path().string();
}

// Check write permissions
bool checkWritePermissions(const string& directory) {
    logMessage("Testing write permissions in directory: " + directory, "DEBUG");
    if (!fs::exists(directory)) {
        logMessage("Error: Directory does not exist: " + directory, "ERROR");
        return false;
    }
    string testFile = directory + "\\test_write.tmp"; // Use "\\" for Windows paths
    ofstream outfile(testFile, ios::trunc);
    if (outfile.is_open()) {
        outfile.close();
        fs::remove(testFile); // Delete the test file
        return true;
    }
    logMessage("Error: Unable to create test file in directory: " + directory, "ERROR");
    return false;
}

// Safe disk space calculation
size_t getSafeDiskSpace(size_t available_disk) {
    const size_t safety_margin = 1024 * 1024 * 100; // 100 MB safety margin
    if (available_disk <= safety_margin) {
        logMessage("Error: Insufficient disk space.", "ERROR");
    }
    return available_disk - safety_margin;
}

// Safe RAM calculation
size_t getSafeRAM(size_t available_ram) {
    const size_t safety_margin = 1024 * 1024 * 500; // 500 MB safety margin
    if (available_ram <= safety_margin) {
        logMessage("Error: Insufficient RAM.", "ERROR");
    }
    return available_ram - safety_margin;
}

// PQT Struct
struct PQT {
    mpz_class P, Q;
    mpf_class T;
};

// Forward declaration of computeChunk
PQT computeChunk(int start, int end, const mpz_class& A, const mpz_class& B, const mpz_class& C3_24);

// BBP Algorithm: Compute n-th hex digits of Pi
string computeBBP(int n) {
    double pi = 0.0;
    for (int k = 0; k <= n; ++k) {
        double term = (4.0 / (8.0 * k + 1.0)) - (2.0 / (8.0 * k + 4.0)) - 
                      (1.0 / (8.0 * k + 5.0)) - (1.0 / (8.0 * k + 6.0));
        term /= pow(16.0, k);
        pi += term;
    }
    unsigned long hex_pi = static_cast<unsigned long>(pi * pow(16, n - 1));
    stringstream ss;
    ss << hex << uppercase << hex_pi;
    return ss.str();
}

// Convert Hexadecimal to GMP Binary
mpz_class hexToBinary(const string& hexValue) {
    mpz_class binaryValue;
    if (mpz_set_str(binaryValue.get_mpz_t(), hexValue.c_str(), 16) != 0) {
        logMessage("Invalid Hexadecimal Value: " + hexValue, "ERROR");
    }
    return binaryValue;
}

// Chudnovsky Algorithm for Pi computation
PQT computeChunk(int start, int end, const mpz_class& A, const mpz_class& B, const mpz_class& C3_24) {
    PQT chunk;
    chunk.P = 1;
    chunk.Q = 1;
    chunk.T = 0;

    for (int k = start; k < end; ++k) {
        mpz_class P_k = (6 * k - 5) * (2 * k - 1) * (6 * k - 1);
        mpz_class Q_k = C3_24 * k * k * k;
        mpf_class T_k = mpf_class(P_k) * mpf_class(A + B * k) / mpf_class(Q_k);

        if (P_k == 0 || Q_k == 0) {
            logMessage("Error: Invalid intermediate values in chunk computation.", "ERROR");
            throw runtime_error("Chunk computation failed.");
        }

        if (k % 2 != 0) T_k = -T_k;

        chunk.P *= P_k;
        chunk.Q *= Q_k;
        chunk.T += T_k;
    }

    // Validate chunk
    if (chunk.P == 0 || chunk.Q == 0 || chunk.T == 0) {
        logMessage("Error: Invalid chunk computed. Values are zero.", "ERROR");
        throw runtime_error("Chunk computation failed.");
    }

    return chunk;
}

void saveChunkToFile(const PQT& chunk, const string& base_filename) {
    lock_guard<mutex> lock(io_mutex);

    logMessage("Attempting to save P chunk to file: " + base_filename + "_p-chunk.txt", "INFO");
    ofstream p_file(base_filename + "_p-chunk.txt", ios::app);
    if (!p_file.is_open()) {
        logMessage("Error: Could not create file " + base_filename + "_p-chunk.txt", "ERROR");
        return;
    }
    p_file << chunk.P << "\n";
    if (p_file.fail()) {
        logMessage("Error: Failed to write P chunk to file " + base_filename + "_p-chunk.txt", "ERROR");
    } else {
        logMessage("Successfully wrote P chunk to " + base_filename + "_p-chunk.txt", "INFO");
    }
    p_file.close();

    logMessage("Attempting to save Q chunk to file: " + base_filename + "_q-chunk.txt", "INFO");
    ofstream q_file(base_filename + "_q-chunk.txt", ios::app);
    if (!q_file.is_open()) {
        logMessage("Error: Could not create file " + base_filename + "_q-chunk.txt", "ERROR");
        return;
    }
    q_file << chunk.Q << "\n";
    if (q_file.fail()) {
        logMessage("Error: Failed to write Q chunk to file " + base_filename + "_q-chunk.txt", "ERROR");
    } else {
        logMessage("Successfully wrote Q chunk to " + base_filename + "_q-chunk.txt", "INFO");
    }
    q_file.close();

    logMessage("Attempting to save T chunk to file: " + base_filename + "_t-chunk.txt", "INFO");
    ofstream t_file(base_filename + "_t-chunk.txt", ios::app);
    if (!t_file.is_open()) {
        logMessage("Error: Could not create file " + base_filename + "_t-chunk.txt", "ERROR");
        return;
    }
    t_file << chunk.T << "\n";
    if (t_file.fail()) {
        logMessage("Error: Failed to write T chunk to file " + base_filename + "_t-chunk.txt", "ERROR");
    } else {
        logMessage("Successfully wrote T chunk to " + base_filename + "_t-chunk.txt", "INFO");
    }
    t_file.close();

    logVerbose("Chunks successfully saved to files and logged.");
}

void printProgressBar(double progress) {
    const int barWidth = 50; // Width of the progress bar
    cout << "[";
    int pos = static_cast<int>(barWidth * progress / 100.0);
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) cout << "=";
        else if (i == pos) cout << ">";
        else cout << " ";
    }
    cout << "] " << static_cast<int>(progress) << "%\r";
    cout.flush();
}

bool monitorMemory(size_t max_memory_bytes) {
    size_t used_memory = getUsedRAM();
    logVerbose("Current memory usage: " + to_string(used_memory) + " bytes.");
    return used_memory < max_memory_bytes;
}

void adjustChunkSize(int& chunk_size, size_t max_memory_bytes) {
    if (!monitorMemory(max_memory_bytes)) {
        chunk_size = max(chunk_size / 2, 10); // Halbiere die Chunkgröße, Minimum 10
        logMessage("Memory limit exceeded. Adjusting chunk size to: " + to_string(chunk_size), "WARNING");
    }
}

void saveIntermediateResult(const string& filename, const mpz_class& result) {
    lock_guard<mutex> lock(io_mutex);
    ofstream outfile(filename, ios::app);
    if (!outfile.is_open()) {
        logMessage("Error: Unable to open intermediate file " + filename, "ERROR");
    }
    outfile << result.get_str() << endl;
    if (outfile.fail()) {
        logMessage("Error: Failed to write intermediate result to " + filename, "ERROR");
    }
    outfile.close();
    logVerbose("Intermediate result saved to " + filename);
}

void processChunksAndSaveWithIntermediate(int total_chunks, int chunk_size, const mpz_class& A, const mpz_class& B, const mpz_class& C3_24, size_t max_memory_bytes) {
    string intermediate_file = "intermediate_results.txt";
    for (int i = 0; i < total_chunks; ++i) {
        adjustChunkSize(chunk_size, max_memory_bytes);
        PQT chunk = computeChunk(i * chunk_size, (i + 1) * chunk_size, A, B, C3_24);
        mpz_class T_as_integer(chunk.T.get_mpf_t()); // Konvertiert mpf_class zu mpz_class
        saveIntermediateResult(intermediate_file, T_as_integer);

        // Calculate progress
        double progress = (static_cast<double>(i + 1) / total_chunks) * 100.0;
        logMessage("Processing chunk " + to_string(i + 1) + " of " + to_string(total_chunks) +
                   " (" + to_string(static_cast<int>(progress)) + "% completed)", "INFO");
        printProgressBar(progress);
    }
    logMessage("All chunks processed and intermediate results saved.", "INFO");
}

size_t calculateMaxPiDigits(size_t available_ram, size_t available_disk_space) {
    size_t digits_by_ram = (available_ram / (2 * 1024 * 1024)) * 10; // 2 MB pro 10 Stellen
    size_t digits_by_disk = (available_disk_space / (1024 * 1024)) * 5; // 1 MB pro 5 Stellen
    return min(digits_by_ram, digits_by_disk);
}

void logSystemStats() {
    size_t available_ram = getAvailableRAM();
    size_t available_disk = getAvailableDiskSpace(".");
    size_t max_digits = calculateMaxPiDigits(available_ram, available_disk);

    logMessage("Available RAM: " + to_string(available_ram / (1024 * 1024)) + " MB", "INFO");
    logMessage("Available Disk Space: " + to_string(available_disk / (1024 * 1024)) + " MB", "INFO");
    logMessage("Estimated Maximum Digits: " + to_string(max_digits), "INFO");
}

// Chudnovsky Algorithm for Pi computation with chunking
mpf_class computeChudnovskyChunked(int total_digits, int chunk_size) {
    mpf_class result = 0;
    const mpz_class A = 13591409;
    const mpz_class B = 545140134;
    const mpz_class C = 640320;
    const mpz_class C3_24 = (C * C * C) / 24;

    for (int start = 0; start < total_digits; start += chunk_size) {
        int end = min(total_digits, start + chunk_size);
        PQT chunk = computeChunk(start, end, A, B, C3_24);
        result += chunk.T; // Summiere die Ergebnisse
    }
    return result;
}

// Integration of BBP and Chudnovsky
mpf_class integrateBBPWithChudnovsky(int bbp_terms, int chudnovsky_digits) {
    logVerbose("Calculating initial hex digits using BBP...");
    string bbp_hex = computeBBP(bbp_terms);
    logVerbose("BBP Result (Hex): " + bbp_hex);

    logVerbose("Converting BBP result to binary...");
    mpz_class bbp_binary = hexToBinary(bbp_hex);
    logVerbose("BBP Result (Binary, GMP): " + bbp_binary.get_str());

    logVerbose("Calculating additional digits using Chudnovsky...");
    mpf_class chudnovsky_pi = computeChudnovskyChunked(chudnovsky_digits, 1000);

    logVerbose("Combining results...");
    mpf_class bbp_adjustment = mpf_class(bbp_binary) / pow(16, bbp_terms);
    return chudnovsky_pi + bbp_adjustment;
}

// Save Pi to File
void savePiToFile(const string& filename, const mpf_class& pi, size_t digits) {
    ofstream outfile(filename, ios::trunc);
    if (!outfile.is_open()) {
        logMessage("Error: Unable to open file " + filename, "ERROR");
    }
    outfile.precision(digits);
    outfile << fixed << pi << endl;
    if (outfile.fail()) {
        logMessage("Error: Failed to write Pi to file " + filename, "ERROR");
    }
    outfile.close();
    logMessage("Pi successfully saved to " + filename, "INFO");
}

// Suggest Safe Digits
size_t suggestSafeDigits(size_t available_ram) {
    return min((available_ram / (1024 * 1024)) / 10 * 1000, size_t(1e6)); // Up to 1 million digits
}

// Main Function
int main(int argc, char* argv[]) {
    openLogFile();
    logMessage("Starting Pi Calculation Program", "INFO");

    try {
        string exe_directory = getExecutableDirectory();
        logMessage("Executable directory: " + exe_directory, "DEBUG");
        if (!checkWritePermissions(exe_directory)) {
            logMessage("Error: No write permissions in the output directory.", "ERROR");
        }
        logMessage("Output directory is writable: " + exe_directory, "INFO");

        size_t available_ram = getSafeRAM(getAvailableRAM());
        size_t available_disk = getSafeDiskSpace(getAvailableDiskSpace(exe_directory));
        int cores = getAvailableCores();

        logVerbose("Available RAM (safe): " + to_string(available_ram / (1024 * 1024)) + " MB");
        logVerbose("Available Disk Space (safe): " + to_string(available_disk / (1024 * 1024)) + " MB");
        logVerbose("Available Cores: " + to_string(cores));

        size_t max_digits_ram = (available_ram / 10) * 1000;
        size_t max_digits_disk = (available_disk / 2);
        size_t max_digits = min(max_digits_ram, max_digits_disk);

        logVerbose("Maximum digits computable: " + to_string(max_digits));

        size_t safe_digits = suggestSafeDigits(available_ram);
        cout << "Maximum digits computable: " << max_digits << endl;
        cout << "Suggested safe digits: " << safe_digits << endl;
        cout << "Enter number of digits to compute (up to " << max_digits << "): ";
        size_t digits;
        cin >> digits;

        if (digits <= 0 || digits > max_digits) {
            logMessage("Invalid number of digits entered.", "ERROR");
        }

        // Limit the number of digits to a practical maximum
        if (digits > 1e8) { // Example: 100 million digits
            logMessage("Error: Requested digit count exceeds practical limits.", "ERROR");
        }

        logMessage("Proceeding with " + to_string(digits) + " digits.", "INFO");

        int bbp_terms = 5;
        mpf_class pi = integrateBBPWithChudnovsky(bbp_terms, digits);

        cout.precision(digits);
        cout << "Calculated Pi: " << fixed << pi << endl;

        // Save the result to a file
        logMessage("Finalizing Pi computation...", "INFO");
        savePiToFile("pi_result.txt", pi, digits);
        logMessage("Final Pi result saved to pi_result.txt", "INFO");

    } catch (const exception& e) {
        logMessage(string("Error: ") + e.what(), "ERROR");
        cerr << "Error: " << e.what() << endl;
    }

    closeLogFile();
    return 0;
}

