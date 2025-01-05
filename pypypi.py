import math
import os
import psutil
from functools import lru_cache
from decimal import Decimal, getcontext

class PiCalculator:
    def __init__(self, digits, chunk_size=None, verbose=True):
        self.digits = digits
        # Dynamically calculate chunk size if not provided
        if chunk_size is None:
            self.chunk_size = max(1000, digits // 1000)  # Default to 1/1000 of digits or 1000, whichever is greater
        else:
            self.chunk_size = chunk_size
        self.output_file = "pi_partial_results.txt"
        self.verbose = verbose

        # Set precision for Decimal calculations
        getcontext().prec = digits + 10  # Add extra precision to avoid rounding issues

        # Constants for the Chudnovsky algorithm
        self.A = Decimal(13591409)
        self.B = Decimal(545140134)
        self.C = Decimal(640320)
        self.C3_24 = self.C**3 / 24

    @lru_cache(maxsize=None)
    def factorial(self, n):
        """
        Compute the factorial of n with memoization.
        """
        if n == 0 or n == 1:
            return Decimal(1)
        return Decimal(n) * self.factorial(n - 1)

    def chudnovsky_term(self, k):
        """
        Compute the k-th term of the Chudnovsky series.
        """
        M = self.factorial(6 * k) / (self.factorial(3 * k) * self.factorial(k) ** 3)
        L = self.A + self.B * k
        X = (-1) ** k * self.C**(3 * k)
        term = M * L / X
        return term

    def compute_chunk(self, start, end):
        """
        Compute a chunk of terms [start, end).
        """
        total = Decimal(0)
        for k in range(start, end):
            total += self.chudnovsky_term(k)
            if self.verbose:
                print(f"[Progress] Computing term {k} within chunk {start}-{end}: {total}")
        return total

    def compute_pi(self):
        """
        Compute Pi using segmented calculation.
        """
        print(f"Calculating Pi to {self.digits} digits...")
        total_terms = int(self.digits / math.log10(53360))  # Approximate number of terms needed
        print(f"Total number of terms: {total_terms}")
        print(f"Using chunk size: {self.chunk_size}")

        if os.path.exists(self.output_file):
            os.remove(self.output_file)

        # Compute in chunks
        for i in range(0, total_terms, self.chunk_size):
            start = i
            end = min(i + self.chunk_size, total_terms)
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
        Save the computed value of Pi to a file.
        """
        print("Saving result to 'pi.txt'...")
        with open("pi.txt", "w") as f:
            f.write(str(pi))
        print("Save complete.")

def get_max_digits_from_ram():
    """
    Calculate the maximum number of digits of Pi based on available RAM.
    """
    ram = psutil.virtual_memory().total  # Total system RAM in bytes
    max_digits = int((ram * 0.9) / 8)  # Assume 8 bytes per digit and use 90% of RAM
    return max_digits

if __name__ == "__main__":
    # Calculate maximum digits based on RAM
    max_digits = get_max_digits_from_ram()
    print(f"Your system can support up to {max_digits} digits of Pi based on available RAM.")

    # Get user input for number of digits and chunk size
    digits = int(input(f"Enter the number of digits of Pi to compute (1-{max_digits}): "))
    while digits < 1 or digits > max_digits:
        print(f"Please enter a value between 1 and {max_digits}.")
        digits = int(input(f"Enter the number of digits of Pi to compute (1-{max_digits}): "))

    chunk_size = input("Enter the chunk size (leave blank for auto): ")
    chunk_size = int(chunk_size) if chunk_size.strip() else None
    verbose = input("Enable verbose mode? (yes/no): ").strip().lower() == "yes"

    calculator = PiCalculator(digits, chunk_size, verbose)

    # Compute Pi
    pi = calculator.compute_pi()

    # Save the result
    calculator.save_to_file(pi)