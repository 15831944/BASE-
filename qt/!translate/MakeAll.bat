@echo off
C:\Qt\Qt5.5.0\5.5\msvc2012\bin\lrelease ../logger/logger_ru.ts ../login/login_ru.ts ../PlotLib/PlotLib_ru.ts ../ProjectLib/ProjectLib_ru.ts ../UsersDlg/UsersDlg_ru.ts ../VProject/VProject_ru.ts -qm VProject_ru.qm

mkdir backup

copy /y ..\logger\logger_ru.ts backup
copy /y ..\login\login_ru.ts backup
copy /y ..\PlotLib\PlotLib_ru.ts backup
copy /y ..\ProjectLib\ProjectLib_ru.ts backup
copy /y ..\UsersDlg\UsersDlg_ru.ts backup
copy /y ..\VProject\VProject_ru.ts backup

copy /y VProject_ru.qm ..\..\bin\exe\
