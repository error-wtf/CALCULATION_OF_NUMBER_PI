#include <iostream>
#include <fstream>
#include <gmpxx.h>
#include <cmath>
#include <thread>
#include <vector>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

// Copyright (c) 2025 Lino Casu and Akira (AI)

// Mutex for thread-safe logging
std::mutex log_mutex;

// Header-Funktion für Programmdetails
void displayHeader() {
    cout << "----------------------------------------" << endl;
    cout << "Optimierte Pi-Berechnung mit GMP" << endl;
    cout << "Copyright (c) 2025 Lino Casu and Akira (AI)" << endl;
    cout << "----------------------------------------" << endl << endl;
}

// Funktion zum Anzeigen der Programmeinstellungen
void displaySettings(size_t ram_mb, size_t file_size_gb, int digits, int cores, size_t file_size_estimate) {
    cout << "Einstellungen:" << endl;
    cout << "  Max RAM: " << ram_mb << " MB" << endl;
    cout << "  Max Filesize: " << file_size_gb << " GB" << endl;
    cout << "  Max Digits: " << digits << endl;
    cout << "  Used CPU-Cores: " << cores << endl;
    cout << "  Approximate Filesize: " << file_size_estimate / (1024 * 1024) << " MB" << endl;
    cout << endl;
}

// Fortschrittsanzeige mit Logging
void displayProgress(int completed_terms, int total_terms) {
    double progress = (static_cast<double>(completed_terms) / total_terms) * 100.0;
    {
        std::lock_guard<std::mutex> lock(log_mutex);
        cout << "Fortschritt: " << progress << "% abgeschlossen ("
             << completed_terms << " von " << total_terms << " Termen)" << endl;

        ofstream log_file("progress.log", ios_base::app);
        if (log_file.is_open()) {
            log_file << "Fortschritt: " << progress << "% abgeschlossen ("
                     << completed_terms << " von " << total_terms << " Termen)" << endl;
        }
    }
}

// Klasse zur Berechnung mit Chudnovsky-Algorithmus
class Chudnovsky {
    mpz_class A, B, C, C3_24, D, E;
    int DIGITS, N;
    size_t MAX_CHUNK_SIZE;
    int NUM_THREADS;

public:
    Chudnovsky(int digits, size_t chunk_size, int num_threads);
    mpf_class computePi(size_t max_file_size);

    static size_t getAvailableRAM();
    static size_t calculateChunkSize(size_t available_ram);

private:
    void computeChunk(int start, int end, mpz_class &P_chunk, mpz_class &Q_chunk, mpz_class &T_chunk);
};

Chudnovsky::Chudnovsky(int digits, size_t chunk_size, int num_threads) {
    DIGITS = digits;
    A = 13591409;
    B = 545140134;
    C = 640320;
    C3_24 = C * C * C / 24;
    D = 426880;
    E = 10005;
    N = DIGITS / 14.1816474627254776555;
    MAX_CHUNK_SIZE = chunk_size;
    NUM_THREADS = num_threads;
}

void Chudnovsky::computeChunk(int start, int end, mpz_class &P_chunk, mpz_class &Q_chunk, mpz_class &T_chunk) {
    P_chunk = 1;
    Q_chunk = 1;
    T_chunk = 0;

    for (int k = start; k < end; ++k) {
        P_chunk *= (k + 1);
        Q_chunk *= (k + 2);
        T_chunk += P_chunk * (A + B * k);
    }
}

mpf_class Chudnovsky::computePi(size_t max_file_size) {
    mpz_class P_total = 1, Q_total = 1, T_total = 0;

    vector<thread> threads;
    vector<mpz_class> P_chunks(NUM_THREADS), Q_chunks(NUM_THREADS), T_chunks(NUM_THREADS);

    for (int i = 0; i < N; i += MAX_CHUNK_SIZE * NUM_THREADS) {
        for (int t = 0; t < NUM_THREADS; ++t) {
            int start = i + t * MAX_CHUNK_SIZE;
            int end = min(start + static_cast<int>(MAX_CHUNK_SIZE), N);

            threads.emplace_back(&Chudnovsky::computeChunk, this, start, end, ref(P_chunks[t]), ref(Q_chunks[t]), ref(T_chunks[t]));
        }

        for (auto &th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }

        threads.clear();

        for (int t = 0; t < NUM_THREADS; ++t) {
            P_total *= P_chunks[t];
            Q_total *= Q_chunks[t];
            T_total += T_chunks[t];
        }

        displayProgress(i, N);
    }

    mpf_set_default_prec(DIGITS * log2(10));
    mpf_class sqrtC((mpf_class)E);
    sqrtC = sqrt(sqrtC);
    mpf_class pi = D * sqrtC * Q_total;
    pi /= (A * Q_total + T_total);

    ofstream pi_file("pi.txt");
    if (!pi_file) {
        cerr << "Fehler beim Öffnen der Datei für Pi." << endl;
        return pi;
    }
    pi_file.precision(DIGITS);
    pi_file << pi << endl;

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
        size_t max_digits = static_cast<size_t>((available_ram * 0.7) / 8); // 70% des RAMs nutzen

        cout << "Maximal possible Digits regarding RAM: " << max_digits << endl;
        cout << "Enter Digits you want to be calculated (1 - " << max_digits << "): ";

        int digits;
        cin >> digits;

        if (digits < 1 || digits > max_digits) {
            cerr << "Ungültige Anzahl an Stellen. Bitte erneut ausführen." << endl;
            return -1;
        }

        size_t estimated_file_size = digits * 8; // Schätzung: 8 Byte pro Stelle
        cout << "The approximated Filesize: " << estimated_file_size / (1024 * 1024) << " MB." << endl;
        cout << "Do you want to continue? (y/n): ";

        char confirmation;
        cin >> confirmation;

        if (confirmation != 'y' && confirmation != 'Y') {
            cout << "Interrupted." << endl;
            return 0;
        }

        size_t chunk_size = Chudnovsky::calculateChunkSize(available_ram);
        int cores = thread::hardware_concurrency();
        displaySettings(ram_mb, estimated_file_size / (1024 * 1024 * 1024), digits, cores, estimated_file_size);

        Chudnovsky calculator(digits, chunk_size, cores);
        mpf_class pi = calculator.computePi(estimated_file_size);

        cout << "The calculation of Number Pi is finished and stored inside 'pi.txt' gespeichert." << endl;

    } catch (exception &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }

    return 0;
}