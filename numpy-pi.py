import math
import os
import psutil
from decimal import Decimal, getcontext

class NumpyPiCalculator:
    def __init__(self, digits, chunk_size=None, verbose=True):
        self.digits = digits
        self.chunk_size = chunk_size if chunk_size else max(1000, digits // 1000)
        self.output_file = "pi_partial_results.txt"
        self.verbose = verbose
        self.log_file = "pi_calculation_log.txt"

        # Set precision for Decimal calculations
        getcontext().prec = digits + 10  # Add extra precision to avoid rounding issues

    def factorial(self, n):
        """
        Compute factorial using Decimal for high precision.
        """
        result = Decimal(1)
        for i in range(2, n + 1):
            result *= i
        return result

    def chudnovsky_term(self, k):
        """
        Compute the k-th term of the Chudnovsky series using high-precision Decimal.
        """
        M = self.factorial(6 * k) / (self.factorial(3 * k) * self.factorial(k) ** 3)
        L = Decimal(13591409) + Decimal(545140134) * k
        X = (-1) ** k * Decimal(640320) ** (3 * k)
        term = M * L / X
        return term

    def compute_chunk(self, start, end):
        """
        Compute a chunk of terms [start, end).
        """
        total = Decimal(0)
        for k in range(start, end):
            total += self.chudnovsky_term(k)
            if self.verbose and (k - start) % 100 == 0:
                print(f"[Progress] Computed term {k} in chunk {start}-{end}")
                # Write progress to log file
                with open(self.log_file, "a") as log:
                    log.write(f"Term {k} in chunk {start}-{end}: {total}\n")
        return total

    def compute_pi(self):
        """
        Compute Pi using the Chudnovsky algorithm.
        """
        print(f"Calculating Pi to {self.digits} digits...")
        terms = int(self.digits / math.log10(53360))  # Approximate number of terms needed
        print(f"Total number of terms: {terms}")
        print(f"Using chunk size: {self.chunk_size}")

        if os.path.exists(self.output_file):
            os.remove(self.output_file)

        # Compute in chunks
        total_sum = Decimal(0)
        for i in range(0, terms, self.chunk_size):
            start = i
            end = min(i + self.chunk_size, terms)
            print(f"[Chunk Progress] Computing chunk {start} to {end}...")
            chunk_result = self.compute_chunk(start, end)

            # Save intermediate results
            with open(self.output_file, "a") as f:
                f.write(f"{chunk_result}\n")

        # Summing intermediate results
        print("Summing intermediate results...")
        pi_sum = Decimal(0)
        with open(self.output_file, "r") as f:
            for line in f:
                pi_sum += Decimal(line.strip())

        # Compute Pi
        C = Decimal(426880) * Decimal(10005).sqrt()
        pi = C / pi_sum
        print("Calculation complete.")
        return pi

    def save_to_file(self, pi):
        """
        Save the computed value of Pi to a file in chunks.
        """
        print("Saving result to 'pi.txt'...")
        chunk_size = 10**6  # Save 1 million digits per chunk
        with open("pi.txt", "w") as f:
            pi_str = str(pi)
            for i in range(0, len(pi_str), chunk_size):
                f.write(pi_str[i:i + chunk_size])
        print("Save complete.")

def get_max_digits_from_ram():
    """
    Calculate the maximum number of digits of Pi based on available RAM.
    """
    ram = psutil.virtual_memory().total  # Total system RAM in bytes
    # Account for both computation and saving (4x overhead)
    max_digits = int((ram * 0.9) / (8 * 4))
    return max_digits

if __name__ == "__main__":
    max_digits = get_max_digits_from_ram()
    print(f"Your system can support up to {max_digits} digits of Pi based on available RAM.")

    while True:
        try:
            digits = int(input(f"Enter the number of digits of Pi to compute (1-{max_digits}): "))
            if 1 <= digits <= max_digits:
                break
            else:
                print(f"Please enter a value between 1 and {max_digits}.")
        except ValueError:
            print("Invalid input. Please enter an integer value.")

    chunk_size = input("Enter the chunk size (leave blank for auto): ")
    chunk_size = int(chunk_size) if chunk_size.strip() else None
    verbose = input("Enable verbose mode? (yes/no): ").strip().lower() == "yes"

    calculator = NumpyPiCalculator(digits, chunk_size, verbose)

    # Compute Pi
    pi = calculator.compute_pi()

    # Save the result
    calculator.save_to_file(pi)