// resources.h
//
// Platform‑agnostic helpers for querying system resources such as
// available RAM and free disk space.  These functions are designed
// to operate without external dependencies and provide a conservative
// estimate of how many digits of π can safely be computed given
// current system constraints.  They are not guaranteed to detect
// every possible resource limitation, but they prevent common
// situations where the computation might exhaust memory or fill
// storage.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace picore {

// Return the approximate number of bytes of free physical RAM
// available to the current process.  On Linux this uses
// sysinfo().  On Windows it calls GlobalMemoryStatusEx().  If
// detection fails, zero is returned.
std::size_t get_free_ram_bytes();

// Return the number of free bytes on the file system that contains
// the given path.  On POSIX this calls statvfs().  On Windows it
// calls GetDiskFreeSpaceEx().  If detection fails, zero is returned.
std::uint64_t get_free_disk_bytes(const std::string &path);

// Estimate the maximum safe number of decimal digits of π that can be
// computed given the available RAM and disk.  This heuristic uses
// roughly 10 bytes of RAM per digit to account for overhead and
// assumes one byte of disk per digit for the final output.  Only
// 70% of the free RAM is considered usable to prevent memory
// exhaustion.  The result is a conservative lower bound on the
// feasible digit count.
std::size_t estimate_max_digits(std::size_t free_ram,
                                std::uint64_t free_disk);

// Estimate the maximum safe number of decimal digits of π that can be
// computed based solely on the free disk space in bytes.  This uses a
// simple model derived from the `pi_record_estimator.py` script: the
// final digits are stored in base 10^9 (4 bytes per 9 digits), and
// scratch space for fast multiplication and checkpointing is assumed
// to be `scratch_mult` times the digit storage.  Thus the total
// storage needed is `(scratch_mult + 1) * (4/9) * digits` bytes.  If
// `free_disk_bytes` is zero or the computed number of digits would
// overflow `std::size_t`, the function returns zero.
std::size_t estimate_max_digits_by_disk(std::uint64_t free_disk_bytes,
                                        double scratch_mult = 6.0,
                                        double bytes_per_digit = 4.0 / 9.0);


// Return the total size of swap space in bytes.
std::size_t get_total_swap_bytes();

// Return the free VRAM in bytes for the first detected NVIDIA GPU.
// Returns 0 if nvidia-smi is not found or fails.
std::size_t get_free_vram_bytes();

} // namespace picore