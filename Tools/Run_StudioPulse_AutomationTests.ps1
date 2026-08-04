param(
    [string]$ProjectRoot = "C:\Users\user\Documents\Unreal Projects\StudioPulse"
)

$ErrorActionPreference = "Stop"

$UProjectPath = Join-Path $ProjectRoot "StudioPulse.uproject"
$Desktop = [Environment]::GetFolderPath("Desktop")
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$TestLog = Join-Path $Desktop "StudioPulse_AutomationTests_$Timestamp.log"

$EngineRoot = "C:\Program Files\Epic Games\UE_5.8"
$EditorCmd = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

if (-not (Test-Path -LiteralPath $UProjectPath)) {
    throw "StudioPulse.uproject was not found: $UProjectPath"
}

if (-not (Test-Path -LiteralPath $EditorCmd)) {
    throw "UnrealEditor-Cmd.exe was not found: $EditorCmd"
}

& $EditorCmd `
    "$UProjectPath" `
    -unattended `
    -nop4 `
    -NullRHI `
    -NoSound `
    -NoSplash `
    -stdout `
    -FullStdOutLogOutput `
    -UTF8Output `
    "-abslog=$TestLog" `
    "-ExecCmds=Automation RunTests StudioPulse;Quit" `
    "-TestExit=Automation Test Queue Empty"

$ExitCode = $LASTEXITCODE

if (-not (Test-Path -LiteralPath $TestLog)) {
    throw "Automation test log was not created."
}

$LogText = Get-Content -LiteralPath $TestLog -Raw

if ($ExitCode -ne 0 -or
    $LogText -match "Result=\{Fail\}|Result=\{Error\}|Automation Test Failed|Test Failed") {
    Start-Process notepad.exe $TestLog
    throw "StudioPulse automation tests failed."
}

Write-Host ""
Write-Host "STUDIOPULSE AUTOMATION TESTS PASSED" -ForegroundColor Green
Write-Host "Log: $TestLog" -ForegroundColor White