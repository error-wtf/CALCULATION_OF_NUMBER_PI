#!/bin/bash

# Ensure the script is run with superuser privileges
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root. Use sudo." 
   exit 1
fi

# Check if Python is installed
if ! command -v python3 &> /dev/null
then
    echo "Python3 is not installed. Installing..."
    apt update && apt install -y python3 python3-pip
else
    echo "Python3 is already installed."
fi

# Check if pip is installed
if ! command -v pip3 &> /dev/null
then
    echo "pip is not installed. Installing..."
    python3 -m ensurepip --upgrade
    python3 -m pip install --upgrade pip
else
    echo "pip is already installed."
fi

# Install mpmath
echo "Installing mpmath..."
pip3 install mpmath

# Confirm installation
if python3 -c "import mpmath" &> /dev/null
then
    echo "mpmath successfully installed."
else
    echo "Failed to install mpmath. Please check your pip configuration."
    exit 1
fi

echo "All dependencies installed successfully."
