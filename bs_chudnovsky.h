#pragma once

#include "picore/bigint_rns.h"

namespace picore {

// PQT structure holds numerator P, denominator Q and T for binary splitting.
struct PQT {
    BigIntRNS P;
    BigIntRNS Q;
    BigIntRNS T;
};

// Binary splitting implementation for the Chudnovsky series.
// This placeholder returns identity values and does not implement
// the actual algorithm. It can be extended to compute real values.
PQT bs_chudnovsky(int a, int b);

} // namespace picore
