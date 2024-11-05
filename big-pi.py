import mpmath
import math
from joblib import Parallel, delayed
import os

# Set mp.dps (decimal places) for mpmath to handle high-precision arithmetic
mpmath.mp.dps = 1000  # Adjust as needed for precision

# Function to calculate individual term in Chudnovsky series
def chudnovsky_term(k):
    M = mpmath.mpf(math.factorial(6 * k)) / (
        math.factorial(3 * k) * (math.factorial(k) ** 3)
    )
    L = 13591409 + 545140134 * k
    X = (-262537412640768000) ** k
    term = M * L / X
    return term

# Calculate Pi using Chudnovsky series in chunks
def calculate_pi(digits, max_memory_gb):
    terms_needed = int(digits / math.log10(533))
    chunk_size = min(int(max_memory_gb * (10**9) / 16), 5000)  # Max 5000 terms in a chunk for safety

    print(f"Using chunk size: {chunk_size} terms (with max chunk {max_memory_gb} GB, total memory {max_memory_gb} GB)")
    
    # Ensure chunk.txt is cleared before starting
    open("chunk.txt", "w").close()

    # Calculate chunks and write each to chunk.txt
    for i in range(0, terms_needed, chunk_size):
        print(f"Calculating terms {i} to {min(i + chunk_size, terms_needed)}...")
        chunk_results = Parallel(n_jobs=-1, backend="multiprocessing", verbose=5)(
            delayed(chudnovsky_term)(k) for k in range(i, min(i + chunk_size, terms_needed))
        )
        
        # Write each chunk result to chunk.txt
        with open("chunk.txt", "a") as chunk_file:
            for result in chunk_results:
                chunk_file.write(str(result) + "\n")
                
    # Summing all chunk results from the file
    print("Summing all chunks to get final result...")
    total_sum = mpmath.mpf(0)
    try:
        with open("chunk.txt", "r") as chunk_file:
            for line in chunk_file:
                total_sum += mpmath.mpf(line.strip())
    except Exception as e:
        print(f"Error reading chunk.txt or summing chunks: {e}")
        return

    # Final calculation of Pi
    try:
        C = 426880 * mpmath.sqrt(10005)
        pi_value = C / total_sum

        # Verify the length of calculated Pi before writing
        if len(str(pi_value)) - 2 < digits:  # Subtract 2 for "3."
            print("Warning: Calculated Pi length is shorter than expected. Calculation might need adjustments.")
            return
    except Exception as e:
        print(f"Error calculating final Pi value: {e}")
        return

    # Save the final result to pi.txt
    try:
        with open("pi.txt", "w") as file:
            file.write(str(pi_value))
        print("Calculation complete. Result stored in pi.txt.")
    except Exception as e:
        print(f"Error writing to pi.txt: {e}")

# Prompt user for the number of digits and run the calculation
if __name__ == "__main__":
    digits = int(input("Enter the number of digits of Pi to calculate: "))
    calculate_pi(digits, max_memory_gb=48)  # Adjust max memory as needed
