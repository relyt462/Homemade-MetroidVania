@echo off
mkdir ..\build
pushd ..\build
cl -Zi ..\code\win32_MetroidVania.cpp user32.lib gdi32.lib
popd
