#!/bin/bash
echo "Installing dependencies for hybrid calculation (Python)..."
pip3 install mpmath
echo "Dependencies installed!"
echo "Starting hybrid Python algorithm for Pi calculation..."
python3 pi-hybrid.py