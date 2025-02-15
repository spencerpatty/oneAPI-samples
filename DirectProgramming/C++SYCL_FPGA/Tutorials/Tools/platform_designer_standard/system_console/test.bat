@ REM Check if quartus.exe is on the PATH
@ WHERE /q quartus.exe
@ IF ERRORLEVEL 1 (
    ECHO quartus.exe not found. Ensure it is installed and placed in your PATH.
    EXIT /B
) 
 
@ REM  Find Quartus in PATH
@ WHERE quartus.exe > .quartus-log.txt
@ SET /p QUARTUS_PATH=<.quartus-log.txt
@ DEL .quartus-log.txt
@ SET QUARTUS_PATH=%QUARTUS_PATH:\quartus.exe=%

@ REM derive paths to system-console.exe and quartus_pgm.exe
@ SET SYSCONSOLE_PATH=%QUARTUS_PATH%\..\..\syscon\bin
@ SET QUARTUSPGM_PATH=%QUARTUS_PATH%

%QUARTUSPGM_PATH%\quartus_pgm.exe -m jtag -c 1 -o "p;add.sof@2"

@ IF %ERRORLEVEL% EQU 0 (
    PAUSE
    C:\intelFPGA\23.1std\quartus\bin64\quartus_sh.exe -t test_add.tcl
    @REM "%SYSCONSOLE_PATH%\system-console.exe" -cli --rc_script=test_add.tcl
) ELSE (
    EXIT /B 1
)