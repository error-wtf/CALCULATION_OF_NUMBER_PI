import decimal

# Setze die Präzision auf eine höhere Anzahl von Dezimalstellen
decimal.getcontext().prec = 50

# Funktion zur Berechnung der Wurzel mit decimal
def berechne_wurzel(zahl, wurzelexponent):
    zahl_decimal = decimal.Decimal(zahl)
    exponent_decimal = decimal.Decimal(1) / decimal.Decimal(wurzelexponent)
    return zahl_decimal ** exponent_decimal

# Hauptprogramm
if __name__ == "__main__":
    try:
        # Eingabe der Zahl
        zahl = input("Gib die Zahl ein, von der du die Wurzel berechnen möchtest: ")
        
        # Eingabe des Wurzelexponenten
        wurzelexponent = input("Gib den Wurzelexponent ein (z.B. 2 für Quadratwurzel, 3 für dritte Wurzel): ")

        # Berechnung der Wurzel
        ergebnis = berechne_wurzel(zahl, wurzelexponent)

        # Ausgabe des Ergebnisses ohne wissenschaftliche Notation
        print(f"Die {wurzelexponent}-te Wurzel von {zahl} ist {ergebnis}")

    except decimal.InvalidOperation:
        print("Bitte gib eine gültige Zahl ein.")
