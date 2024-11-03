from joblib import Parallel, delayed
import mpmath
import os

# Calculation function for a term in the Chudnovsky series (moved outside)
def chudnovsky_term(k):
    M = mpmath.fac(6 * k) / (mpmath.fac(3 * k) * mpmath.fac(k) ** 3)
    L = 13591409 + 545140134 * k
    X = (-262537412640768000) ** k
    S = M * L / X
    return S

def calculate_pi(digits, max_memory_gb=48):
    print(f"Calculating Pi to {digits} digits with a {max_memory_gb} GB memory limit...")

    # Set mpmath precision
    mpmath.mp.dps = digits

    # Define a chunk size based on memory limit and term size estimation
    term_memory_mb = 8  # Approximate memory per term in MB; adjust based on testing
    terms_per_chunk = max_memory_gb * 1024 // term_memory_mb  # Calculate terms per chunk based on memory
    chunk_size = min(terms_per_chunk, 5000)  # Limit chunk size to avoid crashes; adjust as needed

    print(f"Using chunk size: {chunk_size} terms (with max chunk {chunk_size * term_memory_mb // 1024} GB, total memory {max_memory_gb} GB)")

    # File for storing intermediate chunk results
    with open("chunk.txt", "w") as chunk_file:
        for i in range(0, digits, chunk_size):
            print(f"Calculating terms {i} to {min(i + chunk_size, digits)}...")
            # Calculate a chunk of terms and save to disk
            chunk_results = Parallel(n_jobs=-1, backend="multiprocessing", verbose=5)(
                delayed(chudnovsky_term)(k) for k in range(i, min(i + chunk_size, digits))
            )
            chunk_sum = sum(chunk_results)
            chunk_file.write(f"{chunk_sum}\n")  # Write the chunk sum to file

    # Summing all chunk results from the file
    print("Summing all chunks to get final result...")
    total_sum = mpmath.mpf(0)
    with open("chunk.txt", "r") as chunk_file:
        for line in chunk_file:
            total_sum += mpmath.mpf(line.strip())

    # Final calculation of Pi
    C = 426880 * mpmath.sqrt(10005)
    pi_value = C / total_sum

    # Save the final result to pi.txt
    with open("pi.txt", "w") as file:
        file.write(str(pi_value))

    print("Calculation complete. Result stored in pi.txt.")

# Run the calculation
if __name__ == "__main__":
    digits = int(input("Enter the number of digits of Pi to calculate: "))
    calculate_pi(digits, max_memory_gb=48)
