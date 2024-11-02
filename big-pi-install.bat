@echo off
echo Installing dependencies for Pi calculation script...

REM Check if Python is installed
python --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Python is not installed. Downloading Python installer...
    REM Download Python installer
    powershell -Command "Invoke-WebRequest -Uri https://www.python.org/ftp/python/3.10.9/python-3.10.9-amd64.exe -OutFile python-installer.exe"
    echo Installing Python...
    start /wait python-installer.exe /quiet InstallAllUsers=1 PrependPath=1
    del python-installer.exe
    echo Python installed successfully.
) else (
    echo Python is already installed.
)

REM Ensure pip is up-to-date
echo Updating pip...
python -m ensurepip --upgrade
python -m pip install --upgrade pip

REM Install required Python modules
echo Installing required Python modules...
python -m pip install joblib mpmath

REM Confirm installations
echo All dependencies have been installed.

echo Dependencies setup complete. Press any key to continue.
pause
