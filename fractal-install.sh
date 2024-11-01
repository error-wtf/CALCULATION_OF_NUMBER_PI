#!/bin/bash
# Install Python dependencies
echo "Installing dependencies..."
apt install python3-gmpy2
pip install numpy matplotlib --root-user-action=ignore

# Run the fractal script
echo "Running the fractal script..."
python3 fractal.py
