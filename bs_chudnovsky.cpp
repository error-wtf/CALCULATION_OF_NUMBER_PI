#include "picore/bs_chudnovsky.h"

namespace picore {

PQT bs_chudnovsky(int a, int b) {
    // Placeholder implementation that returns P=1, Q=1, T=0.
    PQT out;
    out.P = BigIntRNS::fromInt(1);
    out.Q = BigIntRNS::fromInt(1);
    out.T = BigIntRNS::fromInt(0);
    (void)a;
    (void)b;
    return out;
}

} // namespace picore
