@echo off
REM Install dependencies and run Pi calculator

REM Check if PyPy is installed
set PYPY_DIR=C:\pypy3
if not exist "%PYPY_DIR%" (
    echo PyPy is not installed. Please install PyPy in C:\pypy3.
    pause
    exit /b
)

REM Add PyPy to PATH for this session
set PATH=%PYPY_DIR%;%PATH%

REM Check if psutil is installed
echo Checking for required Python dependencies...
pypy3 -m pip install --upgrade pip
pypy3 -m pip show psutil >nul 2>&1
if %errorlevel% neq 0 (
    echo Installing psutil...
    pypy3 -m pip install psutil
) else (
    echo psutil is already installed.
)

REM Run the Pi Calculator script
echo Running Pi Calculator...
pypy3 big_pi_pro_optimized.py
pause
