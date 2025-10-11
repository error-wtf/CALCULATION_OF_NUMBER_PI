// pi_cli.cpp
//
// Command line interface for the PiCore project. This program
// exposes several algorithms for computing π.
// It can be run with command-line arguments or in an interactive
// menu mode if no arguments are provided.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <cmath>
#include <limits>
#include <iomanip>

#include "picore/chudnovsky.h"
#include "picore/resources.h"

// Forward declaration from the old spigot implementation, kept for algorithm variety
static std::string compute_pi_spigot(int digits);

// --- Progress Bar ---
void print_progress_bar(double progress, const std::string& message) {
    const int bar_width = 70;
    std::cout << "[";
    int pos = bar_width * progress;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << static_cast<int>(progress * 100.0) << "% " << message << "\r";
    std::cout.flush();
}

// --- Resource Calculation and Interactive Menu ---


void print_resources(std::size_t free_ram, std::uint64_t free_disk, std::size_t total_swap, std::size_t free_vram) {
    auto to_gb = [](std::uint64_t bytes) {
        return static_cast<double>(bytes) / (1024ULL * 1024ULL * 1024ULL);
    };
    auto to_mb = [](std::uint64_t bytes) {
        return static_cast<double>(bytes) / (1024ULL * 1024ULL);
    };

    std::cout << "--------------------------------\n";
    std::cout << "pi-error-wtf - A Pi Computation Suite\n";
    std::cout << "Copyright ANTI-CAPITALIST SOFTWARE LICENSE (v 1.4) 2024 © Lino Casu\n";
    std::cout << "--------------------------------\n";
    std::cout << "System Resource Overview\n";
    std::cout << "--------------------------------\n";

    std::cout << "Free RAM: " << std::fixed << std::setprecision(2) << to_gb(free_ram) << " GB\n";
    std::cout << "Free Disk Space: " << std::fixed << std::setprecision(2) << to_gb(free_disk) << " GB\n";
    std::cout << "Total Swap Space: " << std::fixed << std::setprecision(2) << to_gb(total_swap) << " GB\n";
    if (free_vram > 0) {
        std::cout << "Free GPU VRAM: " << std::fixed << std::setprecision(2) << to_mb(free_vram) << " MB\n";
    } else {
        std::cout << "Free GPU VRAM: Not available (nvidia-smi not found or failed)\n";
    }
    std::cout << "--------------------------------\n";
}

// --- Main Application Logic ---

static void usage() {
    std::cout << "pi_cli - A Pi Computation Suite\n";
    std::cout << "Copyright ANTI-CAPITALIST SOFTWARE LICENSE (v 1.4) 2024 © Lino Casu\n\n";
    std::cout << "Usage: pi_cli [options]\n";
    std::cout << "Run without arguments to enter interactive mode.\n\n";
    std::cout << "Options:\n";
    std::cout << "  --digits <N>          Number of decimal digits to compute.\n";
    std::cout << "  --output <file>       Path to output file.\n";
    std::cout << "  --algorithm <name>    Algorithm: spigot, bbp, chudnovsky-cpu, chudnovsky-gpu.\n";
    std::cout << "  --bbp-terms <T>       Number of BBP terms to sum.\n";
    std::cout << "  --verbose             Enable verbose output.\n";
    std::cout << "\nExample:\n  pi_cli --digits 1000 --algorithm chudnovsky-gpu --output out.txt\n";
}

int main(int argc, char *argv[]) {
    // Default values
    int digits = 0;
    std::string outFile = "pi.txt";
    std::string algorithm = "chudnovsky-gpu";
    int bbpTerms = 10;
    bool verbose = false;

    // --- Resource Calculation ---
    std::size_t free_ram = picore::get_free_ram_bytes();
    std::uint64_t free_disk = picore::get_free_disk_bytes(".");
    std::size_t total_swap = picore::get_total_swap_bytes();
    std::size_t free_vram = picore::get_free_vram_bytes();

    // Heuristics for max digits
    std::size_t usable_ram = (free_ram > 0) ? (free_ram * 7 / 10) : 0; // 70% of free RAM
    std::uint64_t disk_after_swap = (free_disk > total_swap) ? (free_disk - total_swap) : 0;
    std::size_t usable_disk = (disk_after_swap > 0) ? (disk_after_swap * 9 / 10) : 0; // 90% of free disk after swap
    
    // Using the project's established heuristic of ~10 bytes RAM per digit
    std::size_t max_ram_digits = usable_ram / 10;
    // 1 byte per digit for output file
    std::size_t max_disk_digits = usable_disk;
    std::size_t max_allowed_digits = std::min(max_ram_digits, max_disk_digits);
    
    if (max_allowed_digits == 0) {
        max_allowed_digits = 100; // Fallback to a small number if detection fails
    }


    // --- Mode Selection ---
    if (argc == 1) {
        // --- Interactive Mode ---
        print_resources(free_ram, free_disk, total_swap, free_vram);
        
        std::cout << "Maximum recommended digits based on resources: " << max_allowed_digits << "\n";
        if (free_vram < 100 * 1024 * 1024 && algorithm == "chudnovsky-gpu") {
             std::cout << "Warning: Low VRAM (<100MB). GPU algorithm might fail.\n";
        }
        std::cout << "Note: For very large calculations, RAM (70% free) and Disk (90% free minus swap) are the primary limits.\n\n";

        // Prompt for output file
        std::cout << "Enter the output file path (default: pi.txt): ";
        std::string input_path;
        std::getline(std::cin, input_path);
        if (!input_path.empty()) {
            outFile = input_path;
        }

        if (std::filesystem::is_directory(outFile)) {
            std::cerr << "Error: Output path is a directory. Please provide a file name." << std::endl;
            return 1;
        }

        // Prompt for digits
        std::cout << "Enter the number of digits to compute (2-" << max_allowed_digits << "): ";
        std::cin >> digits;

        if (!std::cin.good() || digits < 2) {
            std::cerr << "Invalid input. Aborting.\n";
            return 1;
        }
        if (static_cast<std::size_t>(digits) > max_allowed_digits) {
            std::cout << "Requested digits exceed the maximum. Limiting to " << max_allowed_digits << ".\n";
            digits = static_cast<int>(max_allowed_digits);
        }
        
        std::cout << "\nUsing algorithm: " << algorithm << "\n";
        std::cout << "Computing " << digits << " digits...\n";
        std::cout << "Output will be saved to: " << outFile << "\n\n";

    } else {
        // --- Argument Mode ---
        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], "--digits") == 0 && i + 1 < argc) {
                digits = std::atoi(argv[++i]);
            } else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
                outFile = argv[++i];
                if (std::filesystem::is_directory(outFile)) {
                    std::cerr << "Error: Output path is a directory. Please provide a file name." << std::endl;
                    return 1;
                }
            } else if (std::strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
                algorithm = argv[++i];
            } else if (std::strcmp(argv[i], "--bbp-terms") == 0 && i + 1 < argc) {
                bbpTerms = std::atoi(argv[++i]);
            } else if (std::strcmp(argv[i], "--verbose") == 0) {
                verbose = true;
            } else {
                usage();
                return 1;
            }
        }

        if (digits == 0) {
            std::cerr << "Error: --digits is a required argument in non-interactive mode.\n";
            usage();
            return 1;
        }
        if (verbose) {
            std::cout << "Parsed arguments:\n";
            std::cout << "  Digits: " << digits << "\n";
            std::cout << "  Output File: " << outFile << "\n";
            std::cout << "  Algorithm: " << algorithm << "\n";
            std::cout << "  BBP Terms: " << bbpTerms << "\n";
            std::cout << "--------------------------------\n";
        }
        if (digits < 2) {
            std::cerr << "Error: Digits must be at least 2.\n";
            return 1;
        }

        if (static_cast<std::size_t>(digits) > max_allowed_digits) {
            if (verbose) {
                print_resources(free_ram, free_disk, total_swap, free_vram);
            }
            std::cerr << "\nWarning: Requested digits (" << digits 
                      << ") exceed the calculated safe limit of " << max_allowed_digits << ".\n"
                      << "Continuing anyway, but the program may fail due to resource exhaustion.\n\n";
        }
    }

    // --- Computation ---
    std::string pi_string;
    long double pi_ld = 0.0L;
    try {
        if (algorithm == "spigot") {
            pi_string = compute_pi_spigot(digits);
        } else if (algorithm == "bbp") {
            if (bbpTerms < 1) {
                std::cerr << "BBP terms must be positive.\n";
                return 1;
            }
            pi_ld = picore::compute_pi_bbp(bbpTerms);
        } else if (algorithm == "chudnovsky-cpu" || algorithm == "chudnovsky") {
            pi_string = picore::compute_pi_chudnovsky_cpu(digits, [&](double progress, const std::string& message) {
                if (verbose) {
                    print_progress_bar(progress, message);
                }
            });
        } else if (algorithm == "chudnovsky-gpu") {
            pi_string = picore::compute_pi_chudnovsky_gpu(digits, [&](double progress, const std::string& message) {
                if (verbose) {
                    print_progress_bar(progress, message);
                }
            });
        } else {
            std::cerr << "Unknown algorithm: " << algorithm << "\n";
            usage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred during computation: " << e.what() << '\n';
        return 1;
    }

    // --- Output ---
    std::ofstream ofs(outFile);
    if (!ofs) {
        std::cerr << "Failed to open output file for writing: " << outFile << "\n";
        return 1;
    }
    if (algorithm == "bbp") {
        ofs.setf(std::ios::fixed);
        ofs.precision(20);
        ofs << pi_ld << "\n";
        std::cout << "Wrote π approximation using " << bbpTerms
                  << " BBP terms to " << outFile << ".\n";
        if (verbose) {
            std::cout << "\n"; // Clear the progress bar line
        }
    } else {
        ofs << pi_string << "\n";
        std::cout << "Wrote π with " << digits << " digits to " << outFile
                  << " using " << algorithm << " algorithm.\n";
        if (verbose) {
            std::cout << "\n"; // Clear the progress bar line
        }
    }
    ofs.close();
    return 0;
}


// --- Spigot implementation (from original file) ---
static std::string compute_pi_spigot(int digits) {
    if (digits <= 0) return "";
    int len = digits * 10 / 3 + 2;
    std::vector<int> a(len, 20);
    std::string result = "3";
    result.reserve(digits + 2);

    for (int i = 0; i < digits; ++i) {
        int carry = 0;
        for (int j = len - 1; j > 0; --j) {
            int rem = (j * 2) + 1;
            int val = a[j] + carry;
            int quot = val / rem;
            a[j] = val % rem;
            carry = quot * j;
        }
        int last_val = a[0] % 10;
        a[0] = last_val;
        result += std::to_string(last_val);
    }
    result.insert(1, ".");
    return result;
}
