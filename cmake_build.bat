@ECHO OFF
mkdir build
cd build
cmake -G Ninja -DCMAKE_C_COMPILER="clang" -DCMAKE_BUILD_TYPE="Debug" -DRENDER_API="OPENGL" ..
if %ERRORLEVEL% GEQ 1 EXIT /B 1
ninja
cd ..
