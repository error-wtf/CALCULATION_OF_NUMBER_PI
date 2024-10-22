#!/bin/bash

echo # Update package lists and install pip if not installed
echo "Updating system and installing dependencies..."
sudo apt-get update

echo # Install Python package manager (pip)
sudo apt-get install -y python3-pip

echo # Install gmpy2, psutil, and other necessary libraries
echo "Installing Python libraries gmpy2 and psutil..."
pip3 install gmpy2 psutil

echo # Confirm installations
echo "gmpy2 and psutil have been installed successfully!"

echo # Start Script
Python pi.py 

Pause
