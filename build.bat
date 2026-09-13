@echo off
setlocal enableextensions enabledelayedexpansion

set "NDK_PATH=C:/Android/ndk"
set "ABI=arm64-v8a"
set "MIN_SDK=24"
set "BUILD_TYPE=Release"
set "OUTPUT_DIR=output"

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo [1/3] Configuring...
cmake -B build -G "Ninja" ^
  -DCMAKE_TOOLCHAIN_FILE="%NDK_PATH%/build/cmake/android.toolchain.cmake" ^
  -DANDROID_ABI=%ABI% ^
  -DANDROID_PLATFORM=android-%MIN_SDK% ^
  -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
  -DCMAKE_C_COMPILER_LAUNCHER=ccache ^
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache >nul

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed!
    pause & exit /b %ERRORLEVEL%
)

echo [2/3] Building...
cmake --build build --config %BUILD_TYPE% 2>&1 | findstr /i /c:"error" /c:"warning" /c:"FAILED"
set "BUILD_ERR=%ERRORLEVEL%"

if %BUILD_ERR% GTR 1 (
    echo [ERROR] Build failed!
    pause & exit /b 1
)

echo [3/3] Copying .so files...
robocopy "build\lib" "%OUTPUT_DIR%" *.so /njh /njs /ndl /nc /ns >nul
robocopy "build" "%OUTPUT_DIR%" *.so /s /njh /njs /ndl /nc /ns >nul

echo Done.
pause
