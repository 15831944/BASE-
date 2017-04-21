@echo off

erase /q /f V:\Project\bin\exe\VProject.exe >nul 2>&1
erase /q /f V:\Project\bin\exe\Logger.dll >nul 2>&1
erase /q /f V:\Project\bin\exe\Login.dll >nul 2>&1
erase /q /f V:\Project\bin\exe\WorkByDateUser.dll >nul 2>&1
erase /q /f V:\Project\bin\exe\contract-pkz.dll >nul 2>&1

echo ----------------------------------- Compiling in Qt
set PATH=C:\Qt\Qt5.5.0\5.5\msvc2012\bin;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86

set INCLUDE=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include;%INCLUDE%
set LIB=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib;%LIB%
set PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin;%PATH%

rem ------------------------------------------------
call CompPro UsersDlg UsersDlg.pro
if %ERRORLEVEL% neq 0 goto end

call CompPro PlotLib PlotLib.pro
if %ERRORLEVEL% neq 0 goto end

call CompPro ProjectLib ProjectLib.pro
if %ERRORLEVEL% neq 0 goto end

call CompPro VProject VProject.pro
if %ERRORLEVEL% neq 0 goto end

rem ------------------------------------------------
rem Dinamyc libraries

call CompPro Logger Logger.pro
if %ERRORLEVEL% neq 0 goto end

call CompPro Login Login.pro
if %ERRORLEVEL% neq 0 goto end

call CompPro SaveLoadLib SaveLoadLib.pro
if %ERRORLEVEL% neq 0 goto end

call CompPro WorkByDateUser WorkByDateUser.pro
if %ERRORLEVEL% neq 0 goto end

call CompPro contract-pkz contract-pkz.pro
if %ERRORLEVEL% neq 0 goto end

:end
