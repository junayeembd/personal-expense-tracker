$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectRoot

$Executable = Join-Path $ProjectRoot 'build/ExpenseTracker.exe'
if (-not (Test-Path $Executable)) {
    & (Join-Path $PSScriptRoot 'build.ps1')
}

Start-Process -FilePath $Executable -WorkingDirectory (Split-Path -Parent $Executable)
