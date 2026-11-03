$ErrorActionPreference = 'Stop'

# Run this script from any directory; it automatically switches to the project root.
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectRoot

$QtPrefix = 'C:/msys64/ucrt64'
$QtConfig = Join-Path $QtPrefix 'lib/cmake/Qt6/Qt6Config.cmake'

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw 'CMake was not found. Install mingw-w64-ucrt-x86_64-cmake in MSYS2 UCRT64 and add C:\msys64\ucrt64\bin to PATH.'
}
if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    throw 'Ninja was not found. Install mingw-w64-ucrt-x86_64-ninja in MSYS2 UCRT64 and add C:\msys64\ucrt64\bin to PATH.'
}
if (-not (Test-Path $QtConfig)) {
    throw "Qt 6 was not found at $QtConfig. Install mingw-w64-ucrt-x86_64-qt6-base in MSYS2 UCRT64."
}

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$QtPrefix"
if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }

cmake --build build --parallel
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }

Write-Host ''
Write-Host 'Build successful.' -ForegroundColor Green
Write-Host 'Run the application with: .\build\ExpenseTracker.exe'
