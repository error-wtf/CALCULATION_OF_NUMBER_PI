import logging
from mpmath import mp
import psutil
import os

# Logging Configuration
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(message)s')

# Set the default precision
mp.dps = 1000  # Decimal places

# System Information
def get_system_memory():
    """
    Get available system memory in bytes.
    """
    memory = psutil.virtual_memory()
    return memory.available

def get_system_cores():
    """
    Get the number of available CPU cores.
    """
    return os.cpu_count()

# Calculate Maximum Digits, Chunks, and Optimal BBP Terms
def calculate_limits():
    """
    Calculate safe limits for digits, chunks, and BBP terms based on system resources.
    """
    available_memory = get_system_memory()
    cores = get_system_cores()

    # Estimations (1MB = ~1 million digits)
    max_digits_memory = available_memory // (1024 * 1024)
    chunk_size = max(1000, max_digits_memory // 10)  # Divide memory into 10 chunks

    # BBP term estimation (based on CPU cores and practical limits)
    optimal_bbp_terms = min(100, cores * 10)  # Example: 10 terms per core

    logging.info(f"System Memory: {available_memory / (1024**2):.2f} MB")
    logging.info(f"CPU Cores: {cores}")
    logging.info(f"Estimated Max Digits: {max_digits_memory}")
    logging.info(f"Suggested Chunk Size: {chunk_size}")
    logging.info(f"Optimal BBP Terms: {optimal_bbp_terms}")

    return max_digits_memory, chunk_size, optimal_bbp_terms

# BBP Formula
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
    logging.info(f"Calculating BBP with {n} terms...")
    pi = mp.mpf(0)
    for k in range(n):
        term = compute_bbp_term(k) / (16 ** k)
        pi += term
    logging.info("BBP calculation completed.")
    return pi

# Chudnovsky Formula
def compute_chudnovsky_chunked(total_digits, chunk_size):
    """
    Compute Pi using the Chudnovsky formula with chunking.
    """
    logging.info(f"Calculating Chudnovsky for {total_digits} digits in chunks of {chunk_size}...")
    C = 426880 * mp.sqrt(10005)
    M = mp.mpf(1)
    X = mp.mpf(1)
    L = mp.mpf(13591409)
    K = mp.mpf(6)
    S = L

    try:
        for start in range(0, total_digits, chunk_size):
            end = min(total_digits, start + chunk_size)
            logging.info(f"Processing chunk: start={start}, end={end}")
            for k in range(start, end):
                if k == 0:
                    continue  # Skip division by zero
                M = M * ((K**3) - 16*K) / (mp.mpf(k)**3)
                X *= -262537412640768000
                L += 545140134
                S += (M * L) / X
                K += 12
    except Exception as e:
        logging.error(f"Error during Chudnovsky computation: {e}")
        raise

    result = C / S
    logging.info("Chudnovsky calculation completed.")
    return result

# Normalization
def normalize_bbp_to_chudnovsky(bbp_pi, bbp_terms):
    """
    Normalize BBP result to match Chudnovsky's precision.
    """
    logging.info("Normalizing BBP result...")
    normalization_factor = mp.power(16, -bbp_terms)
    return bbp_pi * normalization_factor

# Hybrid Implementation
def hybrid_pi(bbp_terms, total_digits, chunk_size):
    """
    Combine BBP and Chudnovsky results for a hybrid \(\pi\) calculation.
    """
    logging.info("Starting hybrid \(\pi\) calculation...")
    bbp_result = compute_bbp(bbp_terms)
    chudnovsky_result = compute_chudnovsky_chunked(total_digits, chunk_size)

    normalized_bbp = normalize_bbp_to_chudnovsky(bbp_result, bbp_terms)
    hybrid_result = normalized_bbp + chudnovsky_result

    logging.info("Hybrid \(\pi\) calculation completed.")
    return hybrid_result

# Main Execution
if __name__ == "__main__":
    try:
        # Calculate system limits
        max_digits, suggested_chunk_size, optimal_bbp_terms = calculate_limits()

        # Display suggestions
        print(f"Maximum digits: {max_digits}")
        print(f"Suggested chunk size: {suggested_chunk_size}")
        print(f"Optimal BBP terms: {optimal_bbp_terms}")

        # User Input
        bbp_terms = int(input(f"Enter the number of BBP terms (suggested: {optimal_bbp_terms}): "))
        total_digits = int(input(f"Enter the total number of digits for Chudnovsky (max: {max_digits}): "))
        chunk_size = int(input(f"Enter the chunk size for Chudnovsky (suggested: {suggested_chunk_size}): "))

        # Perform Calculation
        result = hybrid_pi(bbp_terms, total_digits, chunk_size)

        # Output Result
        print("\nCalculated \(\pi\):")
        print(result)
    except Exception as e:
        logging.error(f"An error occurred: {e}")
    finally:
        logging.info("Program execution completed.")
