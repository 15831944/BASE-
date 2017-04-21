@echo off


if "%1"=="" goto noparams
if "%2"=="" goto noparams

if not exist "..\%1\%2" exit /b 0

rmdir /s /q "%1" >nul 2>&1
mkdir "%1" >nul 2>&1
cd "%1"
qmake.exe "..\..\%1\%2" -config silent -r -spec win32-msvc2012
set err=%ERRORLEVEL%
if not "%err%" == "0" goto cont1
nmake -f Makefile.Release
set err=%ERRORLEVEL%
:cont1
cd ..
rmdir /s /q "%1"

exit /b %err%

:noparams
exit /b 1
