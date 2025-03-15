import os

def search_in_pi(file_path: str, search_number: str, max_digits: int = 1000000):
    """
    Searches for a number in the first max_digits decimal places of Pi.
    :param file_path: Path to the file containing Pi's decimal places
    :param search_number: The number to search for as a string
    :param max_digits: Maximum number of decimal places to search
    :return: The position of the number or -1 if not found
    """
    if not os.path.exists(file_path):
        print("Error: The file does not exist.")
        return -1
    
    with open(file_path, "r") as file:
        file.seek(2)  # Skip "3."
        pi_digits = file.read(max_digits)  # Read the first max_digits characters
    
    position = pi_digits.find(search_number)
    
    if position != -1:
        print(f"The number {search_number} was found at position {position+1} in the decimal places.")
    else:
        print(f"The number {search_number} was not found in the first {max_digits} decimal places.")
    
    return position

# Example call with user input:
file_path = "pi.txt"  # File containing Pi's decimal places
search_number = input("Enter the number to search for: ")  # Get the number from the user
search_in_pi(file_path, search_number)