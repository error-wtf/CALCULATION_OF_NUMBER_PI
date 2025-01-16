@echo off

REM Set title
setlocal enabledelayedexpansion
set script_title=TPI Configuration Helper
set digits_required=
set ram_required=

REM Set title
TITLE !script_title!

REM Check if Python and psutil are installed
ECHO Checking Python installation...
python --version >nul 2>&1
IF ERRORLEVEL 1 (
    ECHO Python is not installed. Please install Python before proceeding.
    PAUSE
    EXIT /B
)

ECHO Checking if psutil is installed...
python -c "import psutil" >nul 2>&1
IF ERRORLEVEL 1 (
    ECHO Installing the Python module "psutil"...
    pip install psutil
)

REM Retrieve system information
python systemstats.py >nul 2>&1
FOR /F "tokens=2 delims==" %%A IN ('FIND "Total_RAM" stats.txt') DO SET total_ram=%%A
FOR /F "tokens=2 delims==" %%A IN ('FIND "Available_Disk_Space" stats.txt') DO SET total_disk_space=%%A
FOR /F "tokens=2 delims==" %%A IN ('FIND "Max_Pi_Digits" stats.txt') DO SET max_digits=%%A
FOR /F "tokens=2 delims==" %%A IN ('FIND "Required_RAM" stats.txt') DO SET required_ram=%%A

REM Translate RAM into suggestions for command
SET /A max_ram_gb=!total_ram! / 2
IF !max_ram_gb! LSS 2 SET max_ram_gb=2
ECHO Based on your available RAM (!total_ram! GiB), you can use up to:
ECHO - 2Gi for smaller calculations (~10 billion digits, requires ~16 GiB total RAM)
ECHO - !max_ram_gb!Gi for larger calculations (~!max_ram_gb!*10 billion digits)

REM Get user inputs
:input
cls
ECHO ===== TPI Configuration Helper =====

ECHO Available RAM: !total_ram! GiB
ECHO Available disk space: !total_disk_space! GiB
ECHO Maximum Pi digits that can be stored: !max_digits!
ECHO Estimated RAM required for maximum digits: !required_ram! GiB

ECHO INSTRUCTIONS:
ECHO - Enter the number of Pi digits you want to calculate (e.g., 10 for 10 billion digits).
ECHO - Enter the amount of RAM to allocate for the calculation (e.g., 8 for 8GB RAM).
ECHO - Maximum RAM you can allocate: !total_ram! GB.

SET /P digits_required=Please enter the number of digits to calculate: 
SET /P ram_required=Please enter the amount of RAM to use: 

REM Normalize inputs for comparison
SET digits_required_gb=!digits_required!
SET ram_required_gb=!ram_required!

REM Check if there is enough RAM
IF !ram_required_gb! GTR !total_ram! (
    ECHO Error: Not enough RAM available for this configuration.
    ECHO You have !total_ram! GB available, but !ram_required! GB was requested.
    ECHO Please enter a RAM value less than or equal to !total_ram! GB.
    PAUSE
    EXIT /B
)

REM Check if the entered digits exceed the maximum digits that can be stored
IF !digits_required_gb! GTR !max_digits! (
    ECHO Error: The number of digits exceeds the storage capacity.
    ECHO You can store up to !max_digits! digits based on your available disk space.
    ECHO Please enter a number of digits less than or equal to !max_digits!.
    PAUSE
    EXIT /B
)

ECHO Creating configuration...
ECHO Running tpi command...
tpi -T 8 -m !ram_required!Gi -d !digits_required!G -o pi.txt !digits_required!G

PAUSE
