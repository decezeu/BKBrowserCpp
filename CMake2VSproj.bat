@echo off
echo This can build from cmake project a classic VS project
echo for that you need cmake installed from here: https://cmake.org/download/
echo this batch must be open in the same folder as the main CMakeLists.txt
echo this is harcoded to VS 2022 for C++17 x64. Please edit if you need a different copiler version or IDE/platform.
echo if all the above suits you/makes sense press any key. Otherwise close this window and restart after editing

pause

mkdir build
cd build
cmake . -G "Visual Studio 17 2022" -A x64
