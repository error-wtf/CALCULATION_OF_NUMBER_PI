#!/bin/bash


# Update package lists and install dependencies
echo "Updating system and installing dependencies..."
sudo apt-get update
sudo apt-get install -y python3-pip python3-venv dos2unix python3-gmpy2

# Erstellt einen Symlink, sodass `python` als `python3` aufgerufen werden kann
sudo ln -s /usr/bin/python3 /usr/bin/python


# Create a Python virtual environment in the current directory
#echo "Creating a Python virtual environment..."
#python3 -m venv venv

# Activate the virtual environment
#echo "Activating the Python virtual environment..."
#source venv/bin/activate

# Install necessary Python libraries inside the virtual environment
echo "Installing Python libraries gmpy2 and psutil..."
pip3 install gmpy2 psutil --root-user-action=ignore

# Confirm installations
#echo "gmpy2 and psutil have been installed successfully!"

# Start Script
#echo "Starting Python script..."
#python "$PWD/pi.py"

# Deactivate the virtual environment after script execution
#echo "Deactivating the Python virtual environment..."
#deactivate

