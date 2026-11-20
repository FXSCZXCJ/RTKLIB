@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
set QT=D:\Dev\SDK\Qt\6.5.3\msvc2019_64
set SRC=%~dp0build-qt\app\qtapp
set OUT=%~dp0bin\qtapp
if not exist "%OUT%" mkdir "%OUT%"
copy /y "%SRC%\*.exe" "%OUT%" >nul || exit /b 1
copy /y "%~dp0lib\rtklib.dll" "%OUT%" >nul || exit /b 1
copy /y "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Redist\MSVC\14.44.35112\x64\Microsoft.VC143.CRT\*.dll" "%OUT%" >nul || exit /b 1
for %%n in (rtkconv_qt rtkget_qt rtklaunch_qt rtknavi_qt rtkplot_qt rtkpost_qt srctblbrows_qt strsvr_qt) do (
  echo deploying %%n
  "%QT%\bin\windeployqt.exe" --release --no-translations --no-opengl-sw --compiler-runtime --dir "%OUT%" "%OUT%\%%n.exe" >nul || exit /b 1
)
echo done
dir /b "%OUT%"
