import logging
from mpmath import mp
import psutil
import os
import math

# Logging Configuration
tools = logging.getLogger()
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(message)s')

# System Resource Queries
def get_system_memory_mb():
    mem = psutil.virtual_memory()
    return mem.available / (1024 * 1024)

def get_free_disk_mb(path=None):
    if path is None:
        path = os.path.dirname(os.path.abspath(__file__))
    usage = psutil.disk_usage(path)
    return usage.free / (1024 * 1024)

# Limits Calculation based on bytes (90% disk, then 70% RAM)
def calculate_limits():
    mem_mb = get_system_memory_mb()
    disk_mb = get_free_disk_mb()

    mem_bytes = mem_mb * 1024 * 1024
    disk_bytes = disk_mb * 1024 * 1024

    max_ram_bytes = int(mem_bytes * 0.7)
    max_disk_bytes = int(disk_bytes * 0.9)

    max_digits = min(max_ram_bytes, max_disk_bytes)

    logging.info(f"Available RAM: {mem_mb:.2f} MB ({mem_bytes:.0f} bytes)")
    logging.info(f"Free disk: {disk_mb:.2f} MB ({disk_bytes:.0f} bytes)")
    logging.info(f"Max digits by RAM (70%): {max_ram_bytes}")
    logging.info(f"Max digits by disk (90%): {max_disk_bytes}")
    logging.info(f"=> Using up to {max_digits} digits of Pi based on byte limits.")

    return max_digits

# √-Space Block-Based Chudnovsky Implementation
def pebble_factorial(n, stored, block_size):
    """Compute n! by pebbling tree; store only at block boundaries."""
    if n in stored:
        return stored[n]
    if n <= 1:
        stored[n] = mp.mpf(1)
        return stored[n]
    mid = n // 2
    f1 = pebble_factorial(mid, stored, block_size)
    f2 = pebble_factorial(n - mid, stored, block_size)
    prod = mp.nprod(lambda i: mid + i, [1, n-mid])
    result = f1 * f2 * prod
    if (n % block_size) == 0:
        stored[n] = result
    return result


def compute_block(k_start, k_end, block_size):
    """Compute partial Chudnovsky sum for k in [k_start, k_end)."""
    S = mp.mpf(0)
    stored = {}
    for k in range(k_start, k_end):
        fact6k = pebble_factorial(6*k, stored, block_size)
        factk = pebble_factorial(k, stored, block_size)
        fact3k = pebble_factorial(3*k, stored, block_size)
        binom = fact6k / (factk**3 * fact3k)
        term = binom * (13591409 + 545140134*k) / mp.power(-262537412640768000, k)
        S += term
    return S

if __name__ == "__main__":
    max_digits = calculate_limits()
    use_full = input(f"Sollen {max_digits} Stellen verwendet werden? [j/N]: ").strip().lower().startswith('j')
    if use_full:
        digits = max_digits
    else:
        while True:
            try:
                choice = int(input(f"Stellenzahl wählen (2–{max_digits}): "))
                if 2 <= choice <= max_digits:
                    digits = choice
                    break
            except ValueError:
                pass
            print(f"Bitte gültige Zahl zwischen 2 und {max_digits} eingeben.")

    mp.dps = digits + 10
    K = digits
    # Default smaller blocks: ~sqrt(K)/10, min 1
    default_block = max(1, int(math.sqrt(K) / 10))
    print(f"Vorgeschlagene Blockgröße: {default_block}")
    use_default = input("Standard-Blockgröße verwenden? [j/N]: ").strip().lower().startswith('j')
    if use_default:
        block_size = default_block
    else:
        while True:
            try:
                bs = int(input(f"Blockgröße wählen (1–{K}): "))
                if 1 <= bs <= K:
                    block_size = bs
                    break
            except ValueError:
                pass
            print(f"Bitte gültige Blockgröße zwischen 1 und {K} eingeben.")

    num_blocks = math.ceil(K / block_size)
    logging.info(f"Berechne Pi mit {digits} Stellen in {num_blocks} Blöcken à {block_size}...")

    total_S = mp.mpf(0)
    for b in range(num_blocks):
        start = b * block_size
        end = min(K, (b + 1) * block_size)
        logging.info(f"Block {b+1}/{num_blocks}: k∈[{start},{end})")
        total_S += compute_block(start, end, block_size)

    pi_val = (mp.mpf(426880) * mp.sqrt(10005)) / total_S
    pi_str = mp.nstr(pi_val, digits + 1)

    with open('pi.txt', 'w') as f:
        f.write(pi_str)
    size_mb = os.path.getsize('pi.txt') / (1024 * 1024)
    print(f"Pi mit {digits} Stellen in pi.txt geschrieben ({size_mb:.2f} MB)")
    logging.info("Programm abgeschlossen.")
