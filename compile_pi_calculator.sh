
#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Ensure the required libraries are installed (example for Debian-based systems)
echo "Checking for required packages..."
sudo apt-get update
sudo apt-get install -y build-essential cmake libgmp-dev libgmpxx4ldbl nvidia-cuda-toolkit

# Create a build directory
echo "Setting up build directory..."
mkdir -p build
cd build

# Run CMake to configure the build system
echo "Configuring the project with CMake..."
cmake ..

# Build the project using Make
echo "Compiling the project..."
make

# Notify the user of the output
echo "Compilation successful. You can find the executable in the build directory."
