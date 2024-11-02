from joblib import Parallel, delayed
from decimal import Decimal, getcontext
import math

# Setze den Präzisionslevel für Decimal
def initialize_decimal_precision(digits):
    getcontext().prec = digits + 2

# Berechnung des Faktorials mit Decimal
def decimal_factorial(n):
    result = Decimal(1)
    for i in range(2, n + 1):
        result *= i
    return result

# Chudnovsky Algorithmus zur Berechnung der einzelnen Terme
def chudnovsky_term(k):
    C = 426880 * Decimal(math.sqrt(10005))
    M = decimal_factorial(6 * k) / (decimal_factorial(3 * k) * (decimal_factorial(k) ** 3))
    L = (545140134 * k) + 13591409
    X = (-262537412640768000) ** k
    return Decimal(M * L) / X

# Chudnovsky Algorithmus zur Berechnung von Pi mit joblib und verbose
def chudnovsky_algorithm(digits, max_memory_gb):
    initialize_decimal_precision(digits)
    
    # Definiere die Anzahl der Terme basierend auf der Präzision
    terms = digits // 14 + 1
    print("Starting parallel computation with joblib...")

    # Berechnung mit joblib und verbose für eine regelmäßige Statusmeldung
    results = Parallel(n_jobs=-1, backend="loky", verbose=5)(
        delayed(chudnovsky_term)(k) for k in range(terms)
    )
    
    # Summiere alle Terme
    pi_approximation = sum(results)
    
    # Berechne Pi durch Multiplikation mit der Konstante C
    C = 426880 * Decimal(math.sqrt(10005))
    pi_value = C / pi_approximation
    return pi_value

# Berechnung und Ausgabe von Pi
def calculate_pi(digits, max_memory_gb):
    print(f"Calculating Pi to {digits} digits with up to {max_memory_gb} GB of memory...")
    pi_value = chudnovsky_algorithm(digits, max_memory_gb)
    print(f"\nCalculated Pi: {str(pi_value)[:digits + 2]}")
    return pi_value

# Hauptskript
if __name__ == "__main__":
    digits = int(input("Enter the number of digits of Pi to calculate: "))
    max_memory_gb = 32  # Beispielwert für maximalen Speicher
    calculate_pi(digits, max_memory_gb)
