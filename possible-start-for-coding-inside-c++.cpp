#include <iostream>
#include <fstream>
#include <gmp.h>
#include <gmpxx.h>
#include <mpfr.h>
#include <thread>
#include <stdexcept>
#include <cmath>
#include <filesystem>
#include <vector>
#include <mutex>

using namespace std;
namespace fs = std::filesystem;

mutex io_mutex; // Mutex für Thread-Sicherheit

struct PQT {
    mpz_class P, Q;
    mpf_class T;
};

// Chudnovsky-Konstante
const mpz_class A = 13591409;
const mpz_class B = 545140134;
const mpz_class C = 640320;
const mpz_class C3_24 = (C * C * C) / 24;
const mpf_class D = 426880;
const mpf_class E = sqrt(mpf_class(10005));

// Funktion zur Berechnung eines Chunks der Chudnovsky-Serie
PQT computeChunk(int start, int end) {
    PQT chunk{1, 1, 0};
    for (int k = start; k < end; ++k) {
        mpz_class P_k = (6 * k - 5) * (2 * k - 1) * (6 * k - 1);
        mpz_class Q_k = C3_24 * k * k * k;
        mpf_class T_k = mpf_class(P_k) * mpf_class(A + B * k) / mpf_class(Q_k);

        if (k % 2 != 0) T_k = -T_k;

        chunk.P *= P_k;
        chunk.Q *= Q_k;
        chunk.T += T_k;
    }
    return chunk;
}

// Funktion zur Kombination der Ergebnisse mehrerer Chunks
PQT mergeChunks(const PQT& left, const PQT& right) {
    PQT result;
    result.P = left.P * right.P;
    result.Q = left.Q * right.Q;
    result.T = left.T + right.T;
    return result;
}

// Parallelisierte Berechnung der Chudnovsky-Serie
PQT computeChudnovsky(int total_terms, int chunk_size) {
    vector<thread> threads;
    vector<PQT> results((total_terms + chunk_size - 1) / chunk_size);

    for (int i = 0; i < total_terms; i += chunk_size) {
        int start = i;
        int end = min(i + chunk_size, total_terms);
        threads.emplace_back([&, start, end, i]() {
            results[i / chunk_size] = computeChunk(start, end);
        });
    }
    for (auto& t : threads) t.join();

    // Chunks kombinieren
    PQT total = results[0];
    for (size_t i = 1; i < results.size(); ++i) {
        total = mergeChunks(total, results[i]);
    }
    return total;
}

// Hauptfunktion zur Berechnung von \(\pi\)
mpf_class computePi(int digits, int total_terms, int chunk_size) {
    mpf_set_default_prec(digits * 4); // Genauigkeit setzen
    PQT result = computeChudnovsky(total_terms, chunk_size);
    mpf_class pi = D * E * mpf_class(result.Q) / (mpf_class(A) * mpf_class(result.Q) + result.T);
    return pi;
}

// Hauptprogramm
int main() {
    try {
        int digits = 1000;       // Anzahl der gewünschten Stellen
        int terms = 500;         // Anzahl der Chudnovsky-Terme
        int chunk_size = 50;     // Chunk-Größe

        cout << "Berechnung von Pi mit " << digits << " Stellen..." << endl;
        mpf_class pi = computePi(digits, terms, chunk_size);

        // Ergebnis speichern
        ofstream output("pi_result.txt");
        output.precision(digits);
        output << fixed << pi << endl;
        cout << "Pi wurde erfolgreich berechnet und gespeichert." << endl;
    } catch (const exception& e) {
        cerr << "Fehler: " << e.what() << endl;
    }
    return 0;
}
