// Minimal BigIntRNS class for demonstration purposes.
// This implementation wraps a string representation and uses
// 64-bit integers internally for arithmetic. It is NOT intended
// for high‑precision work but provides a placeholder API for
// future extension.

#pragma once

#include <string>
#include <cstdint>

namespace picore {

class BigIntRNS {
public:
    BigIntRNS() : value_(0) {}
    explicit BigIntRNS(std::int64_t v) : value_(v) {}

    static BigIntRNS fromInt(std::int64_t v) { return BigIntRNS(v); }
    static BigIntRNS fromString(const std::string& s);

    std::string toString() const;
    std::int64_t toInt() const { return value_; }

    BigIntRNS operator+(const BigIntRNS& other) const {
        return BigIntRNS(value_ + other.value_);
    }
    BigIntRNS operator-(const BigIntRNS& other) const {
        return BigIntRNS(value_ - other.value_);
    }
    BigIntRNS operator*(const BigIntRNS& other) const {
        return BigIntRNS(value_ * other.value_);
    }

private:
    std::int64_t value_;
};

} // namespace picore
