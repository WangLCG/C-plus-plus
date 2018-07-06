@echo off
set "WORKING_DIR=%cd%"
set "CMAKE=cmake"
set "SRC_DIR=%WORKING_DIR%\src"
set "SLN_DIR=%WORKING_DIR%\sln"
if not exist %SLN_DIR% md %SLN_DIR%
cd %SLN_DIR%
%CMAKE% %SRC_DIR%
pause