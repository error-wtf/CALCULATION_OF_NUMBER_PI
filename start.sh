#!/bin/bash

# Mach alle .sh Dateien ausführbar
chmod a+x *.sh

# Funktion zur Überprüfung und Installation von Wine, falls es nicht vorhanden ist
check_and_install_wine() {
    if ! command -v wine &> /dev/null; then
        echo "Wine is not installed. Installing Wine..."
        sudo apt update
        sudo apt install -y wine
        # Zusätzliche Schritte für die Einrichtung von Wine auf einigen Systemen falls erforderlich
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

    # Menüoptionen in grünem Text
    echo -e "\033[1;32mCalculate the number Pi with a [python] script (short version, 1,000,000 digits)\033[0m"
    echo "Normally under Linux all dependencies are set by default install of python." 
    echo "If not, try [install]ing them."
    
    echo -e "\033[1;32mOr would you like to see [Fractals]?\033[0m"
    echo -e "\033[1;32mOr would you like to learn about [Fibonacci]?\033[0m"
    echo -e "\033[1;32mOr would you like to calculate the Square [Root]?\033[0m"
    echo -e "\033[1;32mOr would you like to calculate the [Square]?\033[0m"
    echo -e "\033[1;32mOpen my [HOMEPAGE]?\033[0m"
    echo "[exit]"
    echo -e "\033[1;32mPS: To calculate Pi longer than 1,000,000 digits - you have to switch to Windows for C++ instead of Python - but then:\033[0m"
    echo -e "\033[1;31m(64 GB RAM required in total - may crash system - use at your own risk)\033[0m"
    read -p "Choose an option: " variable

    case $variable in
        PI|Pi|pi|\[PI\]) INGE ;;  # Accepts PI in various forms, including [PI]
        someone|Someone|SOMEONE) M ;;
        HOMEPAGE|Homepage|homepage) CASU ;;
        install|Install|INSTALL) install_dependencies ;;
        python|Python|PYTHON) python_script ;;
        EXIT|Exit|exit) Q ;;
        FRACTALS|Fractals|fractals) FRACTAL ;;
        FIBONACCI|Fibonacci|fibonacci) FIBONACI ;;
        ROOT|Root|root) WURZEL ;;
        SQUARE|Square|square) QUADRAT ;;
        *) begin ;;
    esac
}

CASU() {
    xdg-open https://lino.lol
    begin
}

INGE() {
    # Check and install Wine if not already installed
    check_and_install_wine

    # Check if tpi.exe exists
    if [ ! -f "tpi.exe" ]; then
        echo "Error: tpi.exe not found in the current directory."
        read -p "Press [Enter] to return to the main menu..."
        begin
        return
    fi

    echo "Calculating PI with 32 GB of memory using Wine and tpi.exe..."
    # Run tpi.exe with Wine and save output to pi.txt
    wine tpi.exe -T 8 -m 6Gi -d 32G -o pi.txt 32G

    # Check if pi.txt was created successfully
    if [ -f "pi.txt" ]; then
        echo "Now writing to file pi.txt..."
        cat pi.txt
        echo "END OF DATA"
    else
        echo "Failed to generate pi.txt. Please check Wine and tpi.exe compatibility."
    fi

    read -p "Press [Enter] to continue..."
    begin
}

M() {
    # Check and install Wine if not already installed
    check_and_install_wine

    # Check if y-cruncher.exe exists
    if [ ! -f "y-cruncher.exe" ]; then
        echo "Error: y-cruncher.exe not found in the current directory."
        read -p "Press [Enter] to return to the main menu..."
        begin
        return
    fi

    echo "Running y-cruncher with Wine..."
    # Run y-cruncher.exe with Wine
    wine y-cruncher.exe

    read -p "Press [Enter] to continue..."
    begin
}

install_dependencies() {
    echo "Checking and installing dependencies if necessary..."
    ./pi-install.sh
    read -p "Dependencies installed. Press [Enter] to continue..."
    begin
}

python_script() {
    echo "Running Python script to calculate PI..."
    ./pi-start.sh
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

