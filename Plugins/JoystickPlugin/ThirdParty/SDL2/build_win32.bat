@echo off

if not exist "C:\Program Files (x86)\CMake\" goto INSTALL_CMAKE

rem if not exist "C:\Program Files\CMake\" goto INSTALL_CMAKE

call "C:\Programme\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
call "C:\Program Files\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"

call "C:\Programme\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"

SET SDL_DIR=SDL
SET BUILD_DIR=build-%SDL_DIR%_x86

mkdir %SDL_DIR%\Lib32

mkdir %BUILD_DIR%

echo "copy patches..."
copy SDL_Patches\*.* %SDL_DIR%\

cd %BUILD_DIR%

"C:\Program Files (x86)\CMake\bin\cmake.exe" -DSDL_STATIC=ON -DSDL_SHARED=ON -DLIB_C=ON-DFORCE_STATIC_VCRT=ON -DEPIC_EXTENSIONS=OFF --build-64bit --config Release ../%SDL_DIR%

msbuild sdl2.sln /t:ALL_BUILD /p:Configuration="Release"
rem cls

echo "copy libs"
copy Release\*.* ..\%SDL_DIR%\Lib32\

echo "copy dlls"
mkdir ..\..\..\Binaries\Win32
copy Release\*.dll ..\..\..\Binaries\Win32\

cd ..

echo "SUCCESS"
goto ENDE

:INSTALL_MERCURIAL
cls
echo "Please install Mercurial x64"
pause

:INSTALL_CMAKE
cls
echo "Please install cmake"
pause

:ENDE
echo "finish."
pause
