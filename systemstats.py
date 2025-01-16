import psutil
import shutil

# Function to calculate the maximum digits of Pi that can be saved in a text file
def calculate_max_pi_digits(available_space_gb):
    # 1 GB of space allows storing approximately 5 billion digits of Pi
    digits_per_gb = 5_000_000_000
    return available_space_gb * digits_per_gb

# Function to calculate the required RAM based on digits
def calculate_required_ram(digits):
    # Approximation: 2GB of primary RAM is needed for every 10 billion digits
    digits_in_billions = digits / 10_000_000_000
    return max(2, int(digits_in_billions * 2))  # Minimum 2GB RAM

def main():
    # Get available RAM in GiB
    total_ram = psutil.virtual_memory().total // (1024 ** 3)

    # Get available disk space in GiB
    total_disk_space, _, free_disk_space = shutil.disk_usage(".")
    free_disk_space_gb = free_disk_space // (1024 ** 3)

    # Calculate maximum Pi digits that can be stored
    max_digits = calculate_max_pi_digits(free_disk_space_gb)

    # Calculate required RAM for max digits
    required_ram_for_max_digits = calculate_required_ram(max_digits)

    # Output system stats to file in a simpler format
    with open("stats.txt", "w") as f:
        f.write(f"Total_RAM={total_ram} GiB\n")
        f.write(f"Available_Disk_Space={free_disk_space_gb} GiB\n")
        f.write(f"Max_Pi_Digits={max_digits}\n")
        f.write(f"Required_RAM={required_ram_for_max_digits} GiB\n")

if __name__ == "__main__":
    main()
