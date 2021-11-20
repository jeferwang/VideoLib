@echo off

set BUILD_DIR="build/win64"

rmdir /s /q %BUILD_DIR%
cmake -B %BUILD_DIR% -S . -A x64 -G "Visual Studio 16 2019"
cmake --build %BUILD_DIR% --target XVideo --config Debug
@REM cmake --build %BUILD_DIR% --target XVideo --config Release
