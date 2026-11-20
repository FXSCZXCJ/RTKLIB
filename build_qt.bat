@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
if not exist build-qt (
  cmake -S . -B build-qt -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Dev/mingw64/bin/ninja.exe -DCMAKE_PREFIX_PATH=D:/Dev/SDK/Qt/6.5.3/msvc2019_64 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
  if errorlevel 1 exit /b 1
)
cmake --build build-qt --config Release
