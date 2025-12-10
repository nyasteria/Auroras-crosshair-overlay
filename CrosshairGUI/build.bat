@echo off
REM Build script for CrosshairGUI using Qt and CMake

echo Building CrosshairGUI...

REM Check if Qt is installed
if not exist "C:\Qt\6.7.0" (
    echo Qt 6 not found. Please install Qt 6.7 or later.
    pause
    exit /b 1
)

REM Set Qt environment
set QT_PATH=C:\Qt\6.7.0\msvc2022_64
set CMAKE_PREFIX_PATH=%QT_PATH%

REM Create build directory
if not exist "build" mkdir build
cd build

REM Run CMake
echo Running CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH%

if %ERRORLEVEL% NEQ 0 (
    echo CMake failed!
    pause
    exit /b 1
)

REM Build with MSBuild
echo Building with MSBuild...
MSBuild.exe CrosshairGUI.sln /p:Configuration=Release /p:Platform=x64

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!
echo Executable: build\Release\CrosshairGUI.exe

cd ..
pause
