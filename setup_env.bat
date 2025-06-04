@echo off
REM --- Salvo il PATH originale per poterlo mostrare o ripristinare se serve ---
set "OLD_PATH=%PATH%"

REM --- Aggiungo le cartelle necessarie ---
set "PATH=C:\Program Files\Eclipse Adoptium\jdk-21.0.5.11-hotspot\bin;C:\Program Files\Git\cmd;C:\Program Files\CMake\bin;%OLD_PATH%"

REM --- Controllo se ld.exe è nel PATH ---
where ld.exe >nul 2>&1
if errorlevel 1 (
    echo ERRORE: ld.exe non trovato nella variabile PATH. Controlla l'installazione di MinGW o del linker.
) else (
    echo ld.exe trovato correttamente.
)

echo PATH configurato:
echo %PATH%

pause
