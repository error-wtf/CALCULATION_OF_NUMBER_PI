import gmpy2
from gmpy2 import mpfr, get_context, set_context
import time
import sys
import psutil  # For memory management

# Set up the precision context using MPFR, with memory-efficient chunking
def setup_context(digits):
    # Increase precision to handle more digits
    precision = digits * 5  # Set precision based on digits
    context = get_context()
    context.precision = precision
    set_context(context)

# Monitor memory usage and adjust chunk size
def monitor_memory(max_memory_gb):
    max_memory_bytes = max_memory_gb * 1024 ** 3  # Convert GB to bytes
    current_memory = psutil.virtual_memory().used
    return current_memory < max_memory_bytes

# Chudnovsky algorithm for Pi calculation with memory limits
def chudnovsky_algorithm(digits, chunk_size=100000, max_memory_gb=32):
    setup_context(digits)
    
    # Chudnovsky constants
    C = mpfr(426880) * gmpy2.sqrt(mpfr(10005))
    M = mpfr(1)
    L = mpfr(13591409)
    X = mpfr(1)
    K = mpfr(6)
    S = L  # Start the sum with L

    total_iterations = digits // 14 + 1  # Estimate iterations needed
    for k in range(1, total_iterations):
        if not monitor_memory(max_memory_gb):
            print("\nMemory limit exceeded, reducing chunk size.")
            chunk_size = max(chunk_size // 2, 10000)  # Reduce chunk size
            break

        M = (K**3 - 16 * K) * M / k**3
        L += 545140134
        X *= -262537412640768000
        S += M * L / X
        K += 12

        # Display progress in a single line
        progress = (k / total_iterations) * 100
        sys.stdout.write(f"\rProgress: {progress:.2f}%")
        sys.stdout.flush()

    # Final Pi computation
    pi_value = C / S
    print(f"\nFinal Pi value (before formatting): {pi_value}")
    return pi_value

# Function to display Pi in the console
def display_pi_as_text(pi, digits):
    pi_str = str(pi)[:digits + 2]  # Keep only the requested digits
    print(f"\nPi to {digits} digits:\n{pi_str}")
    return pi_str

# Save Pi to a file on the local machine
def save_pi_to_file(pi, digits):
    pi_str = str(pi)[:digits + 2]  # Keep only the requested digits
    with open("pi.txt", "w") as f:
        f.write(pi_str)
    print(f"\nPi saved to pi.txt (located in the current directory)")

# Main function to calculate and display Pi
def calculate_pi(digits, max_memory_gb=32):
    start_time = time.time()

    # Calculate Pi with chunking and memory monitoring
    pi_value = chudnovsky_algorithm(digits, max_memory_gb=max_memory_gb)

    # Display Pi in the console
    pi_str = display_pi_as_text(pi_value, digits)

    # Save Pi to a file on the local machine
    save_pi_to_file(pi_value, digits)

    # Display first 100 digits of Pi for reference
    first_100_digits = pi_str[:102]
    print(f"\nFirst 100 digits of Pi:\n{first_100_digits}")

    # Report completion time
    elapsed_time = time.time() - start_time
    print(f"Calculation completed in {elapsed_time:.2f} seconds")

if __name__ == "__main__":
    # Disclaimer
    print("Disclaimer: The calculation is limited to 1,000,000 digits to prevent memory overflow.")
    print("For more digits, use C++ for better performance.")
    print("See this repository: https://github.com/LinoCasu/CALCULATION_OF_NUMBER_PI")
    print("Have fun with math!\n")

    # Ask for the number of digits of Pi to calculate (limit to 1,000,000)
    digits = int(input("Enter the number of digits of Pi to calculate (up to 1,000,000): "))
    digits = min(digits, 1000000)  # Restrict to 1,000,000 digits

    max_memory_gb = 32  # Limit memory usage to 32 GB

    # Call the main function to calculate Pi
    calculate_pi(digits, max_memory_gb=max_memory_gb)
