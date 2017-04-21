@echo off

set OUT_NAME=..\commit\qt-%date:~-4,4%%date:~-7,2%%date:~0,2%-%time:~0,2%%time:~3,2%%time:~6,2%
set OUT_NAME=%OUT_NAME: =0%

xcopy . "%OUT_NAME%\" /d /e /y /h

cd %OUT_NAME%\VProjectFULL

call All.bat
