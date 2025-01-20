#!/bin/bash

# Make all .sh files executable
chmod a+x *.sh

# Function to check and install Wine if not installed
check_and_install_wine() {
    if ! command -v wine &> /dev/null; then
        echo "Wine is not installed. Installing Wine..."
        sudo apt update
        sudo apt install -y wine
        if [ $? -ne 0 ]; then
            echo "Error installing Wine. Please check your package manager or consult documentation for your distribution."
            exit 1
        fi
    else
        echo "Wine is already installed."
    fi
}

begin() {
    clear
    echo -e "\033[1;31mTHANK YOU !!!\033[0m"
    echo "HERE IS A SCRIPT TO CALCULATE PI USING THE FORMULA OF CHUDNOVSKY"
    read -p "Press [Enter] to continue..."
    B
}

B() {
    clear
    date
    echo -e "\033[1;37m========================================================================================================================\033[0m"
    echo -e "\033[1;37mCalculate the number Pi - MULTIPLE OPTIONS\033[0m"
    echo -e "\033[1;37m========================================================================================================================\033[0m"
    echo -e "\033[1;32m[python]   Calculate Pi with Python (up to 1,000,000 digits)\033[0m"
    echo -e "\033[1;32m[hybrid]   Calculate Pi using the hybrid Python algorithm\033[0m"
    echo -e "\033[1;32m[workaround] Calculate Pi with Python and chunking {can cause calculation loop on too long digits}\033[0m"
    echo -e "\033[1;32m[exit]     Exit the program\033[0m"
    echo -e "\033[1;32mPS: To calculate Pi longer than 1,000,000 digits, a [workaround] is provided (64 GB RAM required).\033[0m"
    echo -e "\033[1;32mOr would you like to see [Fractals]?\033[0m"
    echo -e "\033[1;32mOr would you like to learn about [Fibonacci]?\033[0m"
    echo -e "\033[1;32mOr would you like to calculate the Square [Root]?\033[0m"
    echo -e "\033[1;32mOr would you like to calculate the [Square]?\033[0m"
    echo -e "\033[1;32mOpen my [HOMEPAGE]?\033[0m"
    read -p "Choose an option: " variable

    case $variable in
        someone|Someone|SOMEONE) M ;;
        HOMEPAGE|Homepage|homepage) CASU ;;
        install|Install|INSTALL) install_dependencies ;;
        python|Python|PYTHON) python_script ;;
        hybrid|Hybrid|HYBRID) hybrid_script ;;
        EXIT|Exit|exit) Q ;;
        FRACTALS|Fractals|fractals) FRACTAL ;;
        FIBONACCI|Fibonacci|fibonacci) FIBONACI ;;
        ROOT|Root|root) WURZEL ;;
        SQUARE|Square|square) QUADRAT ;;
        workaround) workaround ;;
        *) begin ;;
    esac
}

CASU() {
    xdg-open https://lino.lol
    begin
}

install_dependencies() {
    echo "Checking and installing dependencies if necessary..."
    ./big-pi-install.sh
    read -p "Dependencies installed. Press [Enter] to continue..."
    begin
}

python_script() {
    echo "Running installation script for Python dependencies..."
    ./pi-install.sh

    echo "Starting Python script to calculate PI..."
    ./pi-start.sh

    read -p "Press [Enter] to continue..."
    begin
}

hybrid_script() {
    echo "Running installation script for hybrid dependencies..."
    ./install_hybrid_and_cplusplus.sh

    echo "Starting hybrid Python algorithm for Pi calculation..."
    python3 pi-hybrid.py

    read -p "Press [Enter] to continue..."
    begin
}

workaround() {
    echo "Running workaround script for high precision Pi calculation..."
    ./big-pi-install.sh
    python3 big-pi.py
    read -p "Press [Enter] to continue..."
    begin
}

FRACTAL() {
    echo "Running fractal installation script..."
    ./fractal-install.sh
    read -p "Press [Enter] to continue..."
    begin
}

FIBONACI() {
    echo "Running Fibonacci script..."
    ./fibonaci.sh
    read -p "Press [Enter] to continue..."
    begin
}

WURZEL() {
    echo "Calculating the square root..."
    python3 wurzel.py
    read -p "Press [Enter] to continue..."
    begin
}

QUADRAT() {
    echo "Calculating the square..."
    python3 quadrat.py
    read -p "Press [Enter] to continue..."
    begin
}

Q() {
    echo "Exiting..."
    exit 0
}

# Start the script
begin
