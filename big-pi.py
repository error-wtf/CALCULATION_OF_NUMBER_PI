from mpmath import mp, factorial, sqrt
from joblib import Parallel, delayed
import os

# Function to calculate a chunk of terms in the Chudnovsky series
def chudnovsky_chunk(start, end, chunk_id):
    chunk_sum = mp.mpf(0)
    for k in range(start, end):
        M = factorial(6 * k) / (factorial(3 * k) * (factorial(k) ** 3))
        L = (545140134 * k + 13591409)
        X = (-262537412640768000) ** k
        chunk_sum += mp.mpf(M * L) / X
    
    # Write each chunk to a temporary file
    with open(f"chunk_{chunk_id}.tmp", "w") as f:
        f.write(str(chunk_sum))
    print(f"Chunk {chunk_id} saved to disk.")
    return f"chunk_{chunk_id}.tmp"  # Return file name instead of data

# Calculate chunk size based on 48GB memory limit, with a maximum chunk size of 8GB
def calculate_chunk_size(digits, num_terms=1000, memory_limit_gb=48, max_chunk_gb=8):
    bytes_per_term = 32  # Approximate bytes per term for high precision
    terms_per_gb = (1024 ** 3) / bytes_per_term
    max_terms = int(memory_limit_gb * terms_per_gb)  # Total terms fitting into 48GB
    chunk_terms = int(max_chunk_gb * terms_per_gb)   # Terms for 8GB chunks
    chunk_size = min(chunk_terms, num_terms // os.cpu_count())
    print(f"Calculated chunk size: {chunk_size} terms (with max chunk {max_chunk_gb} GB, total memory {memory_limit_gb} GB)")
    return chunk_size

# Function to sum results from each chunk file
def sum_chunks(chunk_files):
    total_sum = mp.mpf(0)
    for chunk_file in chunk_files:
        with open(chunk_file, "r") as f:
            chunk_value = mp.mpf(f.read())
            total_sum += chunk_value
        os.remove(chunk_file)  # Delete the temporary file after reading
    return total_sum

def chudnovsky_algorithm(digits, num_terms=1000):
    mp.dps = digits + 10  # Set precision for mpmath
    chunk_size = calculate_chunk_size(digits, num_terms)
    print("Starting parallel computation with joblib and calculated chunk size...")
    num_chunks = (num_terms + chunk_size - 1) // chunk_size

    # Compute each chunk and store the resulting filenames
    chunk_files = Parallel(n_jobs=-1, backend="loky", verbose=5)(
        delayed(chudnovsky_chunk)(i * chunk_size, min((i + 1) * chunk_size, num_terms), i)
        for i in range(num_chunks)
    )

    # Sum up all chunk results from temporary files
    series_sum = sum_chunks(chunk_files)
    
    # Final calculation of Pi
    C = 426880 * sqrt(10005)
    pi = C / series_sum
    return pi

def calculate_pi(digits):
    print(f"Calculating Pi to {digits} digits with a 48 GB memory limit.")
    pi_value = chudnovsky_algorithm(digits)
    print("Calculation completed.")
    
    # Write the result to pi.txt
    with open("pi.txt", "w") as file:
        file.write(str(pi_value))
    print("Result stored in pi.txt")
    return str(pi_value)

# Main execution
if __name__ == "__main__":
    digits = int(input("Enter the number of digits of Pi to calculate: "))
    pi_str = calculate_pi(digits)
    print(f"First 100 digits of Pi: {pi_str[:100]}")
