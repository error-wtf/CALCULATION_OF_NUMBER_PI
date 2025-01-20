@echo off
:begin
cls
color c
echo THANK YOU !!!
echo HERE IS A SCRIPT TO CALCULATE PI USING THE FORMULA OF CHUDNOVSKY
pause
:B
cls
color e
date /T
time /T
color F0
echo ==============================================================================================================================
echo                                         Calculate the number Pi - MULTIPLE OPTIONS
echo ==============================================================================================================================
color a
echo [workaround] 	Calculate Pi with Python and chunking {can cause calculation loop on too long digits}
echo [python]   	Calculate Pi with Python (up to 1,000,000 digits)
echo [hybrid]   	Calculate Pi using my own hybrid Python algorithm (better memory management but never tested on long digits)
echo [c++]      	Calculate Pi using the C++ program and choose how many billion digits you want to calculate
color F0
echo ==============================================================================================================================
echo [exit]     	Exit the program
echo ==============================================================================================================================
color a
echo 				Calculate the number [PI] with 6 GB of RAM up to 32 billion digits?
color F0
echo ==============================================================================================================================
echo 				(64 GB RAM required in total - may crash system - use at your own risk)
echo ==============================================================================================================================
color a
echo Calculate the number Pi with a [python] script (short version, 1,000,000 digits)
echo PS: I made a [workaround] to calculate Pi with more digits, but 64 GB RAM NEEDED !!! and it is not so fancy and in Python.
echo Or would you like to see [Fractals]?
echo Or would you like to learn about [Fibonacci]?
echo Or would you like to calculate the Square [Root]?
echo Or would you like to calculate the [Square]?
echo ==============================================================================================================================
echo Open my [HOMEPAGE]?
echo [exit]
echo ==============================================================================================================================
set /p variable=

if /i %variable% == PI goto :INGE
if /i %variable% == Pi goto :INGE
if /i %variable% == pi goto :INGE
if /i %variable% == python goto :PYTHON
if /i %variable% == Python goto :PYTHON
if /i %variable% == PYTHON goto :PYTHON
if /i %variable% == hybrid goto :HYBRID
if /i %variable% == Hybrid goto :HYBRID
if /i %variable% == HYBRID goto :HYBRID
if /i %variable% == c++ goto :CPP
if /i %variable% == C++ goto :CPP
if /i %variable% == exit goto :END
if /i %variable% == Exit goto :END
if /i %variable% == EXIT goto :END
if /i %variable% == FRACTALS goto :FRACTAL
if /i %variable% == Fractals goto :FRACTAL
if /i %variable% == fractals goto :FRACTAL
if /i %variable% == FIBONACCI goto :FIBONACI
if /i %variable% == Fibonacci goto :FIBONACI
if /i %variable% == fibonacci goto :FIBONACI
if /i %variable% == ROOT goto :WURZEL
if /i %variable% == Root goto :WURZEL
if /i %variable% == root goto :WURZEL
if /i %variable% == SQUARE goto :QUADRAT
if /i %variable% == Square goto :QUADRAT
if /i %variable% == square goto :QUADRAT
if /i %variable% == WORKAROUND goto :WORKAROUND
if /i %variable% == Workaround goto :WORKAROUND
if /i %variable% == workaround goto :WORKAROUND

goto :begin

:PYTHON
ECHO Starting Python calculation...
CALL pi-install-python.bat
PAUSE
GOTO begin

:HYBRID
ECHO Starting Pi hybrid calculation...
CALL install_hybrid_and_cplusplus.bat
CALL python pi-hybrid.py
PAUSE
GOTO begin

:CPP
ECHO Starting C++ Pi calculation...
CALL install_hybrid_and_cplusplus.bat
CALL "calculation of number pi - c++.bat"
PAUSE
GOTO begin

:END
ECHO Exiting...
ENDLOCAL
EXIT /B

:CASU
@echo off
start https://lino.lol
goto :begin

:INGE
tpi -T 8 -m 6Gi -d 32G -o pi.txt 32G
echo Now writing to file pi.txt...
pause
set zahlen="pi.txt"
type %zahlen%
echo END OF DATA
pause
goto :begin

:M
@echo off
call y-cruncher.exe
pause
goto :begin

:FRACTAL
@echo off
call fractal-install.bat
pause
goto :begin

:FIBONACI
@echo off
call fibonaci.bat
pause
goto :begin

:WURZEL
@echo off
call python wurzel.py
pause
goto :begin

:QUADRAT
@echo off
call python quadrat.py
pause
goto :begin

:WORKAROUND
@echo off
call big-pi-install.bat
call python big-pi.py
pause
goto :begin
