@echo off
REM Check if PyPy is installed
set PYPY_DIR=C:\pypy3
if not exist "%PYPY_DIR%" (
    echo PyPy is not installed. Please install PyPy in C:\pypy3.
    pause
    exit /b
)

REM Add PyPy and its Scripts directory to PATH for this session
set PATH=%PYPY_DIR%;%PYPY_DIR%\Scripts;%PATH%

REM Ensure pip is installed for PyPy
echo Checking if pip is installed for PyPy...
pypy3 -m ensurepip --default-pip
if %errorlevel% neq 0 (
    echo Failed to install pip for PyPy. Exiting...
    pause
    exit /b
)

REM Upgrade pip to the latest version
echo Upgrading pip for PyPy...
pypy3 -m pip install --upgrade pip

REM Install mpmath
echo Installing mpmath...
pypy3 -m pip install mpmath
if %errorlevel% neq 0 (
    echo Failed to install mpmath. Exiting...
    pause
    exit /b
)

REM Run the Pi Calculator script
echo Running Pi Calculator...
pypy3 mpmath-pi.py
pause
