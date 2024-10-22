import decimal

# Setze die Präzision auf eine höhere Anzahl von Dezimalstellen
decimal.getcontext().prec = 50

# Funktion zur Berechnung der Potenz mit decimal
def berechne_potenz(zahl, exponent):
    zahl_decimal = decimal.Decimal(zahl)
    return zahl_decimal ** decimal.Decimal(exponent)

# Hauptprogramm
if __name__ == "__main__":
    try:
        # Eingabe der Zahl
        zahl = input("Gib die Zahl ein, die du potenzieren möchtest: ")
        
        # Eingabe des Exponenten
        exponent = input("Gib den Exponent ein (z.B. 2 für Quadrat, 3 für Kubik): ")

        # Berechnung der Potenz
        ergebnis = berechne_potenz(zahl, exponent)

        # Ausgabe des Ergebnisses ohne wissenschaftliche Notation
        print(f"{zahl} hoch {exponent} ist {ergebnis}")

    except decimal.InvalidOperation:
        print("Bitte gib eine gültige Zahl ein.")
