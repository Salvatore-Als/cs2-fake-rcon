@echo off
set PATH=C:\Program Files (x86)\Windows Kits\10\bin\x86;%PATH%
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

call %VCVARS% x86_x64

if not exist build mkdir build
cd build
@echo on
python ../configure.py --plugin-name=test --sdks cs2 --targets=x86_64 --plugin-name=fake_rcon --plugin-alias=fake_rcon

ambuild
PAUSE