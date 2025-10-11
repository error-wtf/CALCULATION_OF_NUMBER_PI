// resources.cpp
//
// Implementation of system resource queries for PiCore.  These
// functions detect free RAM and free disk space using platform
// specific system calls.  See resources.h for details.

#include "picore/resources.h"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <limits>
#include <cstdio>
#include <memory>
#include <array>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#else
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#endif

namespace picore {

std::size_t get_free_ram_bytes() {
#if defined(_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return static_cast<std::size_t>(status.ullAvailPhys);
    }
    return 0;
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return static_cast<std::size_t>(info.freeram) * info.mem_unit;
    }
    return 0;
#endif
}

std::uint64_t get_free_disk_bytes(const std::string &path) {
#if defined(_WIN32)
    ULARGE_INTEGER free_bytes_available;
    ULARGE_INTEGER total_number_of_bytes;
    ULARGE_INTEGER total_number_of_free_bytes;
    if (GetDiskFreeSpaceExA(path.c_str(), &free_bytes_available,
                            &total_number_of_bytes,
                            &total_number_of_free_bytes)) {
        return static_cast<std::uint64_t>(free_bytes_available.QuadPart);
    }
    return 0;
#else
    struct statvfs buf;
    if (statvfs(path.c_str(), &buf) == 0) {
        return static_cast<std::uint64_t>(buf.f_bsize) * buf.f_bavail;
    }
    return 0;
#endif
}

std::size_t estimate_max_digits(std::size_t free_ram,
                                std::uint64_t free_disk) {
    // Use ~10 bytes of RAM per digit for conservative overhead.  Only
    // 70% of the free RAM should be considered usable to avoid
    // exhausting memory.  We multiply by 7 then divide by 10 to
    // approximate 70% (integer math).  For example, with 1 GB of free
    // RAM, usable_ram becomes 0.7 GB.  Then we divide by 10 bytes per
    // digit to estimate the maximum digits storable in memory.
    std::size_t usable_ram = (free_ram * 7) / 10;
    std::size_t max_by_ram = usable_ram / 10;
    // One byte per digit for output file; reserve half the disk as a safety margin.
    std::size_t max_by_disk = static_cast<std::size_t>(free_disk / 2);
    return max_by_ram < max_by_disk ? max_by_ram : max_by_disk;
}

std::size_t estimate_max_digits_by_disk(std::uint64_t free_disk_bytes,
                                        double scratch_mult,
                                        double bytes_per_digit) {
    // If no disk space is available, nothing can be computed.
    if (free_disk_bytes == 0) {
        return 0;
    }
    // Ensure the parameters are positive; if not, revert to defaults.
    if (scratch_mult <= 0.0) {
        scratch_mult = 6.0;
    }
    if (bytes_per_digit <= 0.0) {
        bytes_per_digit = 4.0 / 9.0;
    }
    // Compute the maximum digits: total bytes required per digit = bytes_per_digit * (scratch_mult + 1).
    long double per_digit_total = static_cast<long double>(bytes_per_digit) * (scratch_mult + 1.0L);
    // Avoid division by zero.
    if (per_digit_total <= 0.0L) {
        return 0;
    }
    // Compute the maximum as floating point then cast down.
    long double max_digits_ld = static_cast<long double>(free_disk_bytes) / per_digit_total;
    if (max_digits_ld < 0.0L) {
        return 0;
    }
    // Cap to size_t bounds.
    long double max_size_t = static_cast<long double>(std::numeric_limits<std::size_t>::max());
    if (max_digits_ld > max_size_t) {
        return std::numeric_limits<std::size_t>::max();
    }
    return static_cast<std::size_t>(max_digits_ld);
}

std::size_t get_total_swap_bytes() {
#if defined(_WIN32)
    // Windows doesn't have a direct equivalent to total swap in the same way.
    // The page file size is dynamic. For this purpose, we can estimate it,
    // but for now, returning 0 is a safe default.
    return 0;
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return static_cast<std::size_t>(info.totalswap) * info.mem_unit;
    }
    return 0;
#endif
}

std::size_t get_free_vram_bytes() {
#if defined(_WIN32)
    // nvidia-smi is also available on Windows
    const char* cmd = "nvidia-smi --query-gpu=memory.free --format=csv,noheader,nounits";
#else
    const char* cmd = "nvidia-smi --query-gpu=memory.free --format=csv,noheader,nounits";
#endif
    std::array<char, 128> buffer;
    std::string result;
#if defined(_WIN32)
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
#endif
    if (!pipe) {
        return 0; // popen failed
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (result.empty()) {
        return 0; // nvidia-smi not found or no output
    }

    try {
        // Assuming single GPU, first line is the free memory in MiB
        long long free_mib = std::stoll(result);
        return static_cast<std::size_t>(free_mib) * 1024 * 1024;
    } catch (const std::exception& e) {
        return 0; // Parsing failed
    }
}

} // namespace picore