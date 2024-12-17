#include <iostream>
#include <fstream>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/filesystem.hpp>
#include <thread>
#include <stdexcept>
#include <cmath>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
using namespace boost::multiprecision;
using cpp_int = boost::multiprecision::cpp_int;
using cpp_dec_float_100 = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<100>>;

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

size_t getAvailableDiskSpace(const string& path = ".") {
    boost::filesystem::space_info si = boost::filesystem::space(path);
    return si.available;
}

int getAvailableCores() {
    return thread::hardware_concurrency();
}

struct PQT {
    cpp_int P, Q;
    cpp_dec_float_100 T;
};

class Chudnovsky {
    cpp_int A, B, C, C3_24, D;
    cpp_dec_float_100 E;
    int DIGITS, N;
    size_t MAX_CHUNK_SIZE;
    int NUM_THREADS;

public:
    Chudnovsky(int digits, size_t chunk_size, int num_threads);
    PQT computePQT(size_t n1, size_t n2);
    void computePi(size_t available_disk);
};

Chudnovsky::Chudnovsky(int digits, size_t chunk_size, int num_threads) {
    DIGITS = digits;
    A = 13591409;
    B = 545140134;
    C = 640320;
    C3_24 = (C * C * C) / 24;
    D = 426880;
    E = sqrt(cpp_dec_float_100(10005));
    N = static_cast<int>(ceil(DIGITS / log10(640320.0 * 640320.0 * 640320.0 / 24)));
    MAX_CHUNK_SIZE = chunk_size;
    NUM_THREADS = num_threads;
}

PQT Chudnovsky::computePQT(size_t n1, size_t n2) {
    if (n1 + 1 == n2) {
        PQT res;
        res.P = (2 * cpp_int(n2) - 1) * (6 * cpp_int(n2) - 1) * (6 * cpp_int(n2) - 5);
        res.Q = C3_24 * cpp_int(n2) * cpp_int(n2) * cpp_int(n2);
        res.T = cpp_dec_float_100(res.P) * cpp_dec_float_100(A + B * n2) / cpp_dec_float_100(res.Q);
        if (n2 % 2 != 0) res.T = -res.T;
        return res;
    }
    else {
        size_t mid = (n1 + n2) / 2;
        PQT left = computePQT(n1, mid);
        PQT right = computePQT(mid, n2);
        PQT res;
        res.P = left.P * right.P;
        res.Q = left.Q * right.Q;
        res.T = left.T + right.T;
        return res;
    }
}

void Chudnovsky::computePi(size_t available_disk) {
    cout << "Calculating Pi with " << DIGITS << " digits." << endl;

    cpp_int P_total = 1, Q_total = 1;
    cpp_dec_float_100 T_total = 0;

    for (size_t i = 0; i < N; i += MAX_CHUNK_SIZE) {
        size_t end = min(i + MAX_CHUNK_SIZE, static_cast<size_t>(N));
        cout << "Processing terms " << i << " to " << end << "..." << endl;

        PQT chunk = computePQT(i, end);
        P_total *= chunk.P;
        Q_total *= chunk.Q;
        T_total += chunk.T;
        cout << "Progress: " << ((end * 100) / N) << "% completed." << endl;
    }

    cpp_dec_float_100 pi = cpp_dec_float_100(D) * E * cpp_dec_float_100(Q_total) / (cpp_dec_float_100(A) * cpp_dec_float_100(Q_total) + T_total);

    size_t file_size_estimate = max(DIGITS / 2, size_t(1) * 1024);
    if (file_size_estimate > available_disk) {
        cerr << "Not enough disk space for the output file." << endl;
        return;
    }

    ofstream pi_file("pi.txt");
    if (!pi_file) {
        cerr << "Error opening file to save Pi." << endl;
        return;
    }
    pi_file.precision(DIGITS);
    pi_file << fixed << pi << endl;
    cout << "Pi successfully saved to 'pi.txt'." << endl;
}

int main() {
    try {
        size_t available_ram = getAvailableRAM();
        size_t available_disk = getAvailableDiskSpace();

        cout << "Available RAM: " << available_ram / (1024 * 1024) << " MB" << endl;
        cout << "Available Disk Space: " << available_disk / (1024 * 1024) << " MB" << endl;

        size_t usable_ram = (available_ram * 80) / 100;
        cout << "Maximum usable RAM (80%): " << usable_ram / (1024 * 1024) << " MB" << endl;

        size_t digits = (usable_ram / 10) * 1000; // Estimate digits based on usable RAM
        size_t file_size_estimate = max(digits / 2, size_t(1) * 1024);

        while (true) {
            cout << "Maximum possible digits: " << digits << endl;
            cout << "Estimated file size: " << file_size_estimate / (1024 * 1024) << " MB" << endl;
            cout << "Is this number of digits acceptable? (y/n): ";
            char response;
            cin >> response;
            if (response == 'y' || response == 'Y') {
                break;
            }
            else {
                cout << "Please enter a smaller number of digits: ";
                cin >> digits;
                file_size_estimate = max(digits / 2, size_t(1) * 1024);
            }
        }

        size_t chunk_size = usable_ram / 10; // 10% of usable RAM
        Chudnovsky calculator(digits, chunk_size, getAvailableCores());
        calculator.computePi(available_disk);
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }

    return 0;
}
