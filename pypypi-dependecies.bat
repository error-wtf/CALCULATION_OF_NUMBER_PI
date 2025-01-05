@echo off
REM PyPy Installer and Script Runner
REM Set PyPy installation directory
set PYPY_DIR=C:\pypy3

REM Check if PyPy is installed
if not exist "%PYPY_DIR%" (
    echo PyPy is not installed. Downloading now...
    curl -o pypy3.zip https://downloads.python.org/pypy/pypy3.9-v7.3.12-win64.zip
    echo Extracting PyPy...
    powershell -Command "Expand-Archive -Path pypy3.zip -DestinationPath C:\"
    rename C:\pypy3.9-* pypy3
    del pypy3.zip
    echo PyPy installed at %PYPY_DIR%.
) else (
    echo PyPy is already installed.
)

REM Add PyPy to the PATH temporarily for this session
set PATH=%PYPY_DIR%;%PATH%

REM Verify PyPy installation
echo Verifying PyPy installation...
pypy3 --version || (echo PyPy installation failed. Exiting... && exit /b)

REM Run the Pi Calculator script
echo Running the Pi Calculator script...
pypy3 big_pi_pro_optimized.py

REM End of script
pause
