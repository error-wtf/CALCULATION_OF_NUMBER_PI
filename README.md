"# CALCULATION_OF_NUMBER_PI AFTER THE FORMULA OF CHUDNOVSKI
- 6GB RAM USED - 64GB RAM NEEDED !!!
- CALCULATION OF 32.000.000.000 DIGITS OF PI
- USE AT YOUR OWN RISK !!!
- CAN WRITE RAM FULL AND BREAK SYSTEM 
- USE AT OWN RISK !!!
- CAN BREAK WORLD RECORD !!!
- WINDOWS MENU
- Run
- 1. start.bat
- ![windows-menu](https://github.com/user-attachments/assets/9331c833-28b2-4baa-965b-ebcf2604bf5a)

- LINUX MENU
- Run
- sudo chmod a+x *.sh
- sudo ./start.sh
- If Python Dependencies are not installed yet -> start before:
- sudo apt install python3-gmpy2
- sudo chmod a+x *.sh
- sudo ./pi-install.sh
- ![linux-menu](https://github.com/user-attachments/assets/64c8373c-a8e5-4cbf-9c65-ef3e1d859d1c)

- WSL not supported because of DISPLAY ERROR OF MATHPLOIT -  AS YOU ARE IN WINDOWS -> USE WINDOWS VERSION
- WORKAROUND for longer digits NEEDS 64 GB RAM !!! CAN CAUSE MEMORY OVERFLOW !!!

WATCH THIS DISCLAIMER:
https://www.youtube.com/watch?v=TloIt7cJEes





---
COMMANDS FOR tpi.exe:
- tpi -T 8 -m 6Gi -d 32G -o pi.txt 32G
- MAX TESTED - NEED 64 GB OF RAM !!!
- IF YOU HAVE LESS RAM CHANGE TO:
- tpi -T 8 -m 4Gi -d 10G -o pi.txt 10G
- AND CALCULATE ONLY 10.000.000.000 DIGITS
- 4Gi -> 4GB of primary RAM used -> 32GB RAM NEEDED !!!
- 2Gi -> 2GB of primary RAM used -> 16GB RAM NEEDED !!!
- -d -> DIGITS -> -d 100G -> 100.000.000.000 DIGITS
---
ENJOY AT OWN RISK !!!




Paper for this Github:
https://www.researchhub.com/paper/9021278/addressing-the-limitations-of-arbitrary-precision-computation-a-call-for-collaborative-solutions
This paper highlights the significant challenges faced by researchers attempting to implement
the Chudnovsky algorithm and other arbitrary precision computations in C++ and Python.
Despite advances in hardware, existing libraries and scripts are insufficient for modern
scientific needs, limiting both theoretical mathematics and applied sciences like astrophysics.
This paper critiques the current state of open-source repositories, such as GitHub, which lack
fully functional scripts, and identifies the bottlenecks in Python and C++ libraries. A
collaborative solution is proposed to overcome these limitations and harness the true
potential of modern computing.

---
- *.dll's are the libraries mentioned in the paper for C++
- error.cpp is the source code of error_calculator.exe to show possible errors who can accure
- tpi.exe was first and best approach for C++ but does errors at very very long digits too (search for the euler number e inside the results)
- pi_calculator.exe at short digits works best for C++ but has errors too because it has no floating aritmetics - memory management should been done so
- hoping someone beside me has the nerves to fix all this - or or and - fix the adressed issues inside the Paper
---
READ THE PAPER !!!
---
AGAIN ...
ENJOY AT OWN RISK !!!
