#!/bin/bash

# Ensure the script is run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root. Use sudo."
   exit 1
fi

# Check if Python3 is installed
if ! command -v python3 &> /dev/null; then
    echo "Python3 is not installed. Installing..."
    apt update && apt install -y python3 python3-pip
else
    echo "Python3 is already installed."
fi

# Install or upgrade pip
echo "Upgrading pip..."
python3 -m ensurepip --upgrade
python3 -m pip install --upgrade pip

# Install dependencies
echo "Installing numpy and psutil..."
python3 -m pip install numpy psutil

# Verify installation
echo "Verifying installation..."
if python3 -c "import numpy, psutil" &> /dev/null; then
    echo "numpy and psutil successfully installed."
else
    echo "Failed to install numpy or psutil. Please check for issues."
    exit 1
fi

echo "Dependencies installed successfully."
