#!/bin/bash

# Update package lists and install pipx if not installed
echo "Updating system and installing dependencies..."
sudo apt-get update
sudo apt-get install -y python3-pip python3-venv

# Install pipx
echo "Installing pipx..."
python3 -m pip install --user pipx
python3 -m pipx ensurepath

# Install gmpy2, psutil, and other necessary libraries using pipx
echo "Installing Python libraries gmpy2 and psutil..."
pipx install gmpy2
pipx install psutil

# Confirm installations
echo "gmpy2 and psutil have been installed successfully!"

# Start Script
echo "Starting Python script..."
python3 pi.py
