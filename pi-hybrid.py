import math
from mpmath import mp, mpmathify

mp.dps = 1000  # High precision for calculations

def compute_bbp_term(k):
    """
    Compute the k-th term of the BBP series.
    """
    return (4.0 / (8.0 * k + 1.0)) - (2.0 / (8.0 * k + 4.0)) - \
           (1.0 / (8.0 * k + 5.0)) - (1.0 / (8.0 * k + 6.0))

def compute_bbp(n):
    """
    Compute Pi using the BBP formula up to n terms.
    """
    pi = mp.mpf(0)
    for k in range(n):
        term = compute_bbp_term(k) / (16 ** k)
        pi += term
    return pi

def compute_chudnovsky(digits):
    """
    Compute Pi using the Chudnovsky algorithm to the specified number of digits.
    """
    C = 426880 * mp.sqrt(10005)
    M = 1
    X = 1
    L = 13591409
    K = 6
    S = L

    for k in range(1, digits):
        M = (M * (K ** 3 - 16 * K) // (k ** 3))
        X *= -262537412640768000
        L += 545140134
        S += (M * L) / X
        K += 12

    pi = C / S
    return pi

def normalize_bbp_to_chudnovsky(bbp_pi, bbp_terms):
    """
    Normalize BBP result to match Chudnovsky's precision.
    """
    normalization_factor = mp.power(16, -bbp_terms)
    return bbp_pi * normalization_factor

def hybrid_bbp_chudnovsky(bbp_terms, chudnovsky_digits):
    """
    Integrate BBP and Chudnovsky algorithms.
    """
    print("[INFO] Calculating BBP...")
    bbp_pi = compute_bbp(bbp_terms)
    print(f"[INFO] BBP Result: {bbp_pi}")

    print("[INFO] Calculating Chudnovsky...")
    chudnovsky_pi = compute_chudnovsky(chudnovsky_digits)
    print(f"[INFO] Chudnovsky Result: {chudnovsky_pi}")

    print("[INFO] Normalizing BBP to Chudnovsky...")
    bbp_adjusted = normalize_bbp_to_chudnovsky(bbp_pi, bbp_terms)
    print(f"[INFO] Adjusted BBP Result: {bbp_adjusted}")

    # Combine results with weighted contributions
    hybrid_pi = chudnovsky_pi + bbp_adjusted
    return hybrid_pi

# Parameters for testing
bbp_terms = 10  # Increase for higher precision
chudnovsky_digits = 50  # Set desired precision

# Execute hybrid calculation
hybrid_result = hybrid_bbp_chudnovsky(bbp_terms, chudnovsky_digits)

# Output results
print("[INFO] Hybrid Pi Result:", hybrid_result)
