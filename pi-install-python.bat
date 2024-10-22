@echo off

:: Update system and check if Python is installed
echo Checking if Python is installed...
python --version
if %ERRORLEVEL% neq 0 (
    echo Python is not installed. Please install Python before running this script.
    pause
    exit /b
)

:: Create a virtual environment in the current directory
echo Creating a Python virtual environment...
python -m venv venv

:: Activate the virtual environment
echo Activating the Python virtual environment...
call venv\Scripts\activate

:: Install necessary Python libraries
echo Installing Python libraries gmpy2 and psutil...
pip install gmpy2 psutil

:: Confirm installations
echo gmpy2 and psutil have been installed successfully!

:: Start the Python script
echo Starting Python script...
python pi.py

:: Deactivate the virtual environment
echo Deactivating the Python virtual environment...
deactivate

:: Pause the script to see output before exiting
pause
