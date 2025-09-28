
# pi_record_estimator.py
# Quick back‑of‑the‑envelope requirements for Chudnovsky (binary‑splitting) at D decimal digits.
# This is NOT a performance guarantee—just sizing guidance.
#
# Assumptions:
# - ~14.1816474627 decimal digits gained per Chudnovsky term.
# - Store final digits in base 10^9 (4 bytes per 9 digits ≈ 0.444... bytes/digit).
# - Working scratch (out‑of‑core FFT/NTT, merges, checkpoints): 5–10× of digit storage (you choose factor).
#
# Usage:
#   python pi_record_estimator.py 30000000000000  --scratch-mult 6
#
import argparse, math

DIGITS_PER_TERM = 14.181647462725476
BYTES_PER_DIGIT_BASE1E9 = 4/9  # bytes per decimal digit to store digits in base 1e9

def human_tb(bytes_):
    return bytes_ / (1024**4)

def estimate(D, scratch_mult=6.0):
    terms = math.ceil(D / DIGITS_PER_TERM)
    store_bytes = D * BYTES_PER_DIGIT_BASE1E9
    scratch_bytes = store_bytes * scratch_mult
    return terms, store_bytes, scratch_bytes

if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('digits', type=int, help='Target decimal digits D')
    ap.add_argument('--scratch-mult', type=float, default=6.0, help='Scratch multiple of digit storage (5..10 typical)')
    args = ap.parse_args()
    terms, store_b, scratch_b = estimate(args.digits, args.scratch_mult)
    print(f"Target digits: {args.digits:,}")
    print(f"Chudnovsky terms needed (≈ D/14.1816): {terms:,}")
    print(f"Digit storage (base 1e9 ~0.444 bytes/digit): {store_b:,.0f} bytes ≈ {human_tb(store_b):.2f} TB")
    print(f"Scratch space (~{args.scratch_mult}× storage): {scratch_b:,.0f} bytes ≈ {human_tb(scratch_b):.2f} TB")
    print("\nNotes:")
    print(" - Real runs need checkpointing, ECC, and verification (BBP digit checks, duplicate runs, or alt series).")
    print(" - Time scales ~ D log D with fast multiplication; I/O bandwidth of the scratch array is often the bottleneck.")
    print(" - Multi-node is possible via RNS/CRT sharding across primes; otherwise use a single fat box with many NVMes.")
