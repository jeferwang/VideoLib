@echo off

set BUILD_DIR="build/win64"

rmdir /s /q %BUILD_DIR%
cmake -B %BUILD_DIR% -S . -A x64
cmake --build %BUILD_DIR% --target VideoPlayerCore --config Debug
cmake --build %BUILD_DIR% --target VideoPlayerCore --config Release
