#!/bin/bash

echo "Starting installation of dependencies for big-pi..."

# Update package list
echo "Updating package list..."
sudo apt update 

# Install Python3 and Pip3 if not already installed
echo "Installing Python3 and Pip3..."
sudo apt install -y python3 python3-pip 

# Upgrade pip and install Python modules with --root-user-action=ignore
echo "Installing required Python modules..."
python3 -m pip install --upgrade pip --root-user-action=ignore
python3 -m pip install joblib --root-user-action=ignore
python3 -m pip install mpmath --root-user-action=ignore
python3 -m pip install tqdm --root-user-action=ignore

# Confirm installation
echo "All dependencies for big-pi have been installed."

# Optional Wine installation for specific Windows executables if needed
#read -p "Do you also need Wine for running Windows executables (y/n)? " wine_install
#if [[ $wine_install == "y" || $wine_install == "Y" ]]; then
#    echo "Checking and installing Wine..."
#    sudo apt install -y wine
#    echo "Wine installation complete."
#else
#    echo "Skipping Wine installation."
#fi

echo "big-pi-install.sh has completed."
