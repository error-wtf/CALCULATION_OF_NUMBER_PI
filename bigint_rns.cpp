#include "picore/bigint_rns.h"
#include <stdexcept>

namespace picore {

BigIntRNS BigIntRNS::fromString(const std::string& s) {
    // Convert string to 64‑bit integer. This will throw if the value
    // exceeds the range of int64_t.
    std::size_t idx = 0;
    std::int64_t val = 0;
    try {
        val = std::stoll(s, &idx, 10);
    } catch (const std::exception&) {
        throw std::invalid_argument("BigIntRNS::fromString: invalid number");
    }
    return BigIntRNS(val);
}

std::string BigIntRNS::toString() const {
    return std::to_string(value_);
}

} // namespace picore
