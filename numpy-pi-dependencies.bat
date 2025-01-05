@echo off
REM Check if Python3 or PyPy is installed
if NOT exist "C:\pypy3" (
    echo PyPy is not installed. Please install PyPy in C:\pypy3.
    pause
    exit /b
)

REM Add PyPy and its Scripts directory to PATH for this session
set PATH=C:\pypy3;C:\pypy3\Scripts;%PATH%

REM Ensure pip is installed
echo Checking if pip is installed for PyPy...
pypy3 -m ensurepip --default-pip
if %errorlevel% neq 0 (
    echo Failed to ensure pip. Exiting...
    pause
    exit /b
)

REM Upgrade pip
echo Upgrading pip...
pypy3 -m pip install --upgrade pip

REM Install dependencies
echo Installing numpy and psutil...
pypy3 -m pip install numpy psutil

REM Verify installation
echo Verifying installation...
pypy3 -m pip show numpy psutil
if %errorlevel% neq 0 (
    echo Failed to install dependencies. Please check for issues.
    pause
    exit /b
)

echo Dependencies installed successfully!
pause
