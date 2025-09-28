
# spiral_pi_bridge_demo.py
# Usage: python spiral_pi_bridge_demo.py

from decimal import Decimal, getcontext
import math
import numpy as np
import pandas as pd

getcontext().prec = 200

def chudnovsky_pi(ndigits=150):
    getcontext().prec = ndigits + 10
    C = 426880 * Decimal(10005).sqrt()
    M = Decimal(1)
    L = Decimal(13591409)
    X = Decimal(1)
    K = Decimal(6)
    S = L
    for n in range(1, 10):
        M = (M * (K**3 - 16*K)) / (Decimal(n)**3)
        L += 545140134
        X *= -262537412640768000
        S += (M * L) / X
        K += 12
    pi = C / S
    getcontext().prec = ndigits
    return +pi

def padovan_radii(r0, s, k_max):
    return [r0 * (s ** k) for k in range(k_max+1)]

def quantize_area(area, N, noise_strength=0.0):
    a0 = Decimal(1) / Decimal(N*N)
    if noise_strength:
        area = area + Decimal(np.random.normal(scale=float(a0)*noise_strength))
    q = (area / a0).to_integral_value(rounding="ROUND_FLOOR")
    return int(q), a0

def quantize_length(L, N, noise_strength=0.0):
    l0 = Decimal(1) / Decimal(N)
    if noise_strength:
        L = L + Decimal(np.random.normal(scale=float(l0)*noise_strength))
    q = (L / l0).to_integral_value(rounding="ROUND_FLOOR")
    Lm = q * l0
    return Lm

def annulus_pi_disc(N, r_list, s, pi_ref, noise_strength=0.0):
    num = Decimal(0)
    den = Decimal(0)
    s2m1 = s*s - 1
    for k in range(1, len(r_list)):
        rk = r_list[k]
        rkm1 = r_list[k-1]
        area_exact = pi_ref * (rk*rk - rkm1*rkm1)
        Qk, a0 = quantize_area(area_exact, N, noise_strength=noise_strength)
        num += Decimal(Qk) * a0
        den += rkm1*rkm1
    return num / (den * s2m1)

def quarter_arc_pi_disc(N, r_list, pi_ref, noise_strength=0.0):
    Ln_sum = Decimal(0)
    rbar_sum = Decimal(0)
    for n in range(3, len(r_list)):
        seg_r = [r_list[n-3], r_list[n-2], r_list[n-1], r_list[n]]
        L_exact = sum([ (pi_ref/2) * rr for rr in seg_r ])
        L_meas = quantize_length(L_exact, N, noise_strength=noise_strength)
        rbar = sum(seg_r) / 4
        Ln_sum += L_meas
        rbar_sum += rbar
    return Ln_sum / (2 * rbar_sum)

def richardson(piN, pi2N, p):
    two_p = Decimal(2) ** Decimal(p)
    return (two_p * pi2N - piN) / (two_p - 1)

if __name__ == "__main__":
    s = Decimal('1.3247179572447458')
    r0 = Decimal(1)
    num_rings = 40
    res_levels = [50, 100, 200, 400, 800]
    noise_strength = 0.0

    pi_ref = chudnovsky_pi(150)
    r_list = padovan_radii(r0, s, num_rings)

    rows = []
    piA_seq = []
    piL_seq = []
    for N in res_levels:
        piA = annulus_pi_disc(N, r_list, s, pi_ref, noise_strength=noise_strength)
        piL = quarter_arc_pi_disc(N, r_list, pi_ref, noise_strength=noise_strength)
        piA_seq.append(piA); piL_seq.append(piL)
        rows.append({
            "N": N,
            "pi_ref": str(pi_ref),
            "pi_disc_area": str(+piA),
            "pi_disc_area_err": float(abs(piA - pi_ref)),
            "pi_disc_len": str(+piL),
            "pi_disc_len_err": float(abs(piL - pi_ref)),
        })

    for p in [1,2]:
        for i in range(len(res_levels)-1):
            N = res_levels[i]; N2 = res_levels[i+1]
            piA_R = richardson(Decimal(piA_seq[i]), Decimal(piA_seq[i+1]), p)
            piL_R = richardson(Decimal(piL_seq[i]), Decimal(piL_seq[i+1]), p)
            rows.append({
                "N": f"{N}->{N2} (R,p={p})",
                "pi_ref": str(pi_ref),
                "pi_disc_area": str(+piA_R),
                "pi_disc_area_err": float(abs(piA_R - pi_ref)),
                "pi_disc_len": str(+piL_R),
                "pi_disc_len_err": float(abs(piL_R - pi_ref)),
            })

    df = pd.DataFrame(rows)
    df.to_csv("spiral_pi_bridge_results.csv", index=False)
    print(df.to_string(index=False))
