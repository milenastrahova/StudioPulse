# Testing

## Scope

StudioPulse uses Unreal Automation Framework tests located in:

`	ext
Source/StudioPulse/Private/Tests/
`

The current source tree contains:

- StudioPulseAutomationTests.cpp
- StudioPulseFinalAutomationTests.cpp

The suite validates deterministic data parsing and monitoring-state behaviour used by the dashboard.

## Verification Command

`powershell
powershell -ExecutionPolicy Bypass -File ".\Tools\Run_StudioPulse_AutomationTests.ps1"
`

The runner:

1. finds Unreal Engine 5.8;
2. launches UnrealEditor-Cmd.exe with NullRHI;
3. runs Automation RunTests StudioPulse;
4. writes an absolute Unreal log;
5. returns a non-zero exit code when failures are detected.

## Latest Verified Result

- Status: PASSED
- Successful result markers: 8
- Verification date: 2026-08-04 18:47
- Configuration: Win64 Development Editor, NullRHI, unattended
- Build verification: PASSED

The complete local Unreal test log is generated during verification and is intentionally not committed.