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

REM Install SOCKS dependencies for pip
echo Installing SOCKS dependencies...
pypy3 -m pip install requests[socks] pysocks
if %errorlevel% neq 0 (
    echo Failed to install SOCKS dependencies. Exiting...
    pause
    exit /b
)

REM Check if psutil is installed
echo Checking for required Python dependencies...
pypy3 -m pip show psutil >nul 2>&1
if %errorlevel% neq 0 (
    echo Installing psutil...
    pypy3 -m pip install psutil
) else (
    echo psutil is already installed.
)

REM Run the Pi Calculator script
echo Running Pi Calculator...
pypy3 pypypi.py
pause
