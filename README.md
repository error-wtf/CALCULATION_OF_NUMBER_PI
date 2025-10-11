## CALCULATION_OF_NUMBER_PI AFTER THE FORMULA OF CHUDNOVSKI

## NVIDIA GRAPHICS CARD NEEDED AND CUDA TOOLS INSTALLED !!!

### Linux:
```
sudo apt update
sudo apt install -y build-essential cmake libboost-dev nvidia-cuda-toolkit
chmod a+x pi-error-wtf-v1.0-Linux.deb
dpkg -i pi-error-wtf-v1.0-Linux.deb
```
Then execute it with:
```
pi-error-wtf
```
Example:

```
--------------------------------
pi-error-wtf - A Pi Computation Suite
Copyright © 2025 Carmen Wrede und Lino Casu
--------------------------------
System Resource Overview
--------------------------------
Free RAM: 33.86 GB
Free Disk Space: 471.93 GB
Total Swap Space: 0.00 GB
Free GPU VRAM: 13382.00 MB
--------------------------------
Maximum recommended digits based on resources: 2545176453
Note: For very large calculations, RAM (70% free) and Disk (90% free minus swap) are the primary limits.

Enter the output file path (default: pi.txt):
Enter the number of digits to compute (2-2545176453): 1000001

Using algorithm: chudnovsky-gpu
Computing 1000001 digits...
Output will be saved to: pi.txt

Wrote π with 1000001 digits to pi.txt using chudnovsky-gpu algorithm.
```


### Windows

Just open the EXE-File

## minimum PC-Requirements:

Testet on:

64gb ram
NVIDIA Geforce RTX 4060 TI 
(YOU WILL NEED A NVIDIA CARD AND CUDA TOOLS INSTALLED)
i9 cpu

### CALCULATION_OF_NUMBER_PI AFTER THE FORMULA OF CHUDNOVSKI
- 6GB RAM USED - 64GB RAM NEEDED !!!
- CALCULATION OF 32.000.000.000 DIGITS OF PI
- USE AT YOUR OWN RISK !!!
- CAN WRITE RAM FULL AND BREAK SYSTEM 
- USE AT OWN RISK !!!
- CAN BREAK WORLD RECORD !!!
- WINDOWS MENU
- Run
- 1. start.bat
- ![windows-menu]!![image](https://github.com/user-attachments/assets/f4d6dfb5-cba7-449c-88e3-57d531afd4a2)






- LINUX MENU
- Run
- sudo chmod a+x *.sh
- sudo ./start.sh
- If Python Dependencies are not installed yet -> start before:
- sudo apt install python3-gmpy2
- sudo chmod a+x *.sh
- sudo ./pi-install.sh
- ![linux-menu]!![image](https://github.com/user-attachments/assets/631b96af-c8a9-4002-a281-8dc2224164c9)




- WSL not supported because of DISPLAY ERROR OF MATHPLOIT -  AS YOU ARE IN WINDOWS -> USE WINDOWS VERSION
- WORKAROUND for longer digits NEEDS 64 GB RAM !!! CAN CAUSE MEMORY OVERFLOW !!!

### WATCH THIS DISCLAIMER BEFORE USE
[![Watch the disclaimer on YouTube](https://img.youtube.com/vi/TloIt7cJEes/0.jpg)](https://www.youtube.com/watch?v=TloIt7cJEes)






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
- for EASY USE of tpi.exe -> use : !calculation of number pi - c++.bat!
---
### **ENJOY AT OWN RISK !!!**





### **Paper for this Github:**
https://www.researchhub.com/paper/9021279/addressing-the-limitations-of-arbitrary-precision-computation-a-call-for-collaborative-solutions
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
- pi_calculator.exe at short digits works best for C++ but has errors too because it has no floating aritmetics - memory management should been done similar
- pi-win-output-error.exe how it could be made if it then would not have memory errors and therefor cant create output
- pi-win.exe maybe you have luck to have an output
- -> just use "calculation of number pi - c++.bat" for C++ and rely to the Version of Copyright (c) 2009-2010 Fabrice Bellard
- -> i only managed it inside python up to 1.000.000 digits -> so my math is correct -> but as mentioned in the paper -> its not so easy as said -> in C++ I failed (especialy at longer digits)
- -> i had once a working version on my own made with C++ -> but that was year 1997 -> data is lost

---
- this project was a try to recover data from the year 1997 of a piece of my own source code (source-code.txt) i tried to make it work again
- hoping someone beside me has the nerves to fix all this - or, or and, - fix the adressed issues inside the Paper
---
![image](https://github.com/user-attachments/assets/364c4382-04bc-4f79-8a50-1289f19948c8)

---
READ THE PAPER !!!
---
![image](https://github.com/user-attachments/assets/773e28b1-9ca7-4343-a333-a4f5231d7284)

AGAIN ...

### **ENJOY AT OWN RISK !!!**

### **HERE THE REAL PROGRAM TO BEAT THE WORLD RECORD IN CALACULATING PI** 
https://numberworld.org/y-cruncher/
### **THIS GITHUB IS FOR UNDERSTANDING THE MATH AND DIFFICULTIES BEHIND**

[![command not found](https://img.youtube.com/vi/ev7e9sfWIJo/0.jpg)](https://www.youtube.com/watch?v=ev7e9sfWIJo)

### The Problem on long digits 

[![YouTube Video](https://img.youtube.com/vi/4V2eOQLW1Mk/0.jpg)](https://www.youtube.com/shorts/4V2eOQLW1Mk)


Immo purtroppu, su nostu caminu est istau pienu de trabballas e de obbligos familiares. Deu, poita mancu, appu cussidu de donai a sa familia, e de amprendiri pasos chi a su primu parìnt che pareint minores, ma chi, canto prus tempu passaiant, tantu prus che diventant pesantis, comenti pedras chi no si podint torrai. Essi s’intorciant in trabballas chi cherint impossibiles a suparai. Deu non andaiu prus a circai in is detalus de sa matematica; a fini contat puru custa chi nois naraiamus experiencia de vida. Ammira sa situatzioni politica nostra: faeddant giai apertamenti de is segnus de s’infamiu fascismu. Non tenimus prus tempus po torrai a iscriri a s’università. Su dimoniu no est prus, comenti s’omu in su luna, chi paria lontanu meda e chi no podiat fai mali veru. Est tempus de concentrarisi in su chi contat prus: su camparu giornalieri.

Su tempus de circai provas de una forza divina, in unu mundu chi cun su dispettu volontariu, puru po prusu e prusu su mundu s’est ammalaendi comenti unu virus, est prus passau. Immo semus troppu impegnados a mantenni sa saludi de su chi nois tenimus ancora. Mancu su nostu cuscientzia podint portai cun issus in s’enterru. Podint torrai totu, ma no sa nostu cuscientzia.

Semus in unu mundu chi cun s’ignorantzia chi ha guardau pro tanti tempus is nostus problemas, e chi immo non bistit puru is segnus claros de su tempus, custu est difficili a cambiari. Poita custu nois depimus agguanta a sa familia po non perdiri totu. Su nostu alivu tenit ancora is radichis. Ma meda a su nostu arvorat est istau truncau. Poita faiti atentzioni a tui. Lassai chi nois sighimus a guardai po noi a su primu postu, in vezzi de pensai a montis chi, in teoria, si podint escalai, ma chi, comenti faeddant immo, "noi bos mandamus in su Marti". Issus bollint lassai a nois su desertu, ma nois depimus cunservai su chi a familia nostra no diventi unu desertu.

Deu appu donau totu. Ma no podint ringratziari. Issus si funt affundados in su penseru destruttivu de su odiu e de sa destrutzioni. Poita nois depimus circai de no perdiri mancu sa nostra anima. Su coro de su chi si narat veridade e amori. Poita suos bollint bogai a nois custu puru. A nois e a totus in su mundu. Po custu est tempus de no fujiri, comenti una familia, prus chi est necessariu. Ma de si preparai po is tempos diffìcilis, e poi de essi presentes pro noi, cando servit. Pareti pocu, ma est meda, comparendi cun su nudda chi resta.

PS:
COMPILE INSIDE LINUX FINAL CODE:
nvcc -x cu final-linux-cuda-pi-hybrid.cpp -o pi_calculator -lgmp -lgmpxx

## License

*ANTI-CAPITALIST SOFTWARE LICENSE (v 1.4)* 2024 © Lino Casu



