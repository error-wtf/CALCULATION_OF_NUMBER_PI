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
color a
echo Calculate the number [PI] with 6 GB of RAM up to 32 billion digits?
color 4
echo (64 GB RAM required in total - may crash system - use at your own risk)
color a
echo Calculate the number Pi with a [python] script (short version, 1,000,000 digits)
echo PS: I made a [workaround] to calculate Pi with more digits, but the process status could not be made so fancy in python.
echo Or would you like to see [Fractals]?
echo Or would you like to learn about [Fibonacci]?
echo Or would you like to calculate the Square [Root]?
echo Or would you like to calculate the [Square]?
echo Open my [HOMEPAGE]?
echo [exit]

set /p variable=

if /i %variable% == PI goto :INGE
if /i %variable% == Pi goto :INGE
if /i %variable% == pi goto :INGE
if /i %variable% == someone goto :M
if /i %variable% == Someone goto :M
if /i %variable% == SOMEONE goto :M
if /i %variable% == HOMEPAGE goto :CASU
if /i %variable% == Homepage goto :CASU
if /i %variable% == homepage goto :CASU
if /i %variable% == python goto :python
if /i %variable% == Python goto :python
if /i %variable% == PYTHON goto :python
if /i %variable% == EXIT goto :Q
if /i %variable% == Exit goto :Q
if /i %variable% == exit goto :Q
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

:python 
@echo off
call pi-install-python.bat
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

:Q
@echo off
exit
