import math
import os
from mpmath import mp, mpf, factorial

class MPMathPiCalculator:
    def __init__(self, digits, chunk_size=None, verbose=True):
        self.digits = digits
        self.chunk_size = chunk_size if chunk_size else max(1000, digits // 1000)
        self.output_file = "pi_partial_results.txt"
        self.verbose = verbose
        self.log_file = "pi_calculation_log.txt"

        # Set precision for mpmath calculations
        mp.dps = digits + 10  # Add extra precision to avoid rounding issues

        # Constants for the Chudnovsky algorithm
        self.A = mpf(13591409)
        self.B = mpf(545140134)
        self.C = mpf(640320)
        self.C3_24 = self.C**3 / 24

    def chudnovsky_term(self, k):
        """
        Compute the k-th term of the Chudnovsky series using mpmath.
        """
        M = factorial(6 * k) / (factorial(3 * k) * factorial(k) ** 3)
        L = self.A + self.B * k
        X = (-1) ** k * self.C**(3 * k)
        term = M * L / X
        return term

    def compute_chunk(self, start, end):
        """
        Compute a chunk of terms [start, end).
        """
        total = mpf(0)
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
        total_sum = mpf(0)
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
        pi_sum = mpf(0)
        with open(self.output_file, "r") as f:
            for line in f:
                pi_sum += mpf(line.strip())

        # Compute Pi
        C = mpf(426880) * mp.sqrt(10005)
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

if __name__ == "__main__":
    digits = int(input("Enter the number of digits of Pi to compute: "))
    chunk_size = input("Enter the chunk size (leave blank for auto): ")
    chunk_size = int(chunk_size) if chunk_size.strip() else None
    verbose = input("Enable verbose mode? (yes/no): ").strip().lower() == "yes"

    calculator = MPMathPiCalculator(digits, chunk_size, verbose)

    # Compute Pi
    pi = calculator.compute_pi()

    # Save the result
    calculator.save_to_file(pi)