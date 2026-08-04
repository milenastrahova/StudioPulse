# Virtual Studio Core Plugin

Virtual Studio Core is a reusable Unreal Engine C++ plugin developed as the next engineering milestone of StudioPulse.

## Why It Exists

The original StudioPulse implementation proved the dashboard use case. The plugin isolates backend communication, connection-state management and fallback behaviour into reusable cross-project modules.

## Module Boundary

### Runtime Module

`VirtualStudioCore` contains:

- HTTP JSON snapshot support;
- WebSocket live telemetry;
- exponential reconnect/backoff;
- simulated fallback;
- Blueprint-spawnable Actor Component;
- Blueprint delegates and telemetry structs;
- Project Settings configuration;
- deterministic parser and validation tests.

### Editor Module

`VirtualStudioCoreEditor` contains:

- Tools menu integration;
- plugin settings shortcut;
- endpoint configuration self-test;
- editor-only dependencies isolated from Shipping builds.

## Public API

Main class:

```text
UVirtualStudioConnectionComponent
```

Blueprint events:

```text
OnTelemetryUpdated
OnConnectionStateChanged
OnTransportError
```

Main functions:

```text
Connect
Disconnect
RequestSnapshot
SendTextMessage
```

## Automated Verification

The first plugin pass contains eight tests covering:

- reconnect/backoff mathematics;
- valid JSON parsing;
- alias-field parsing;
- invalid payload rejection;
- endpoint scheme validation;
- default settings;
- Tick-free component configuration;
- reflected Blueprint API.

## Mock Backend

```powershell
py -m pip install websockets
py ".\Tools\MockVirtualStudioServer.py"
```

Endpoints:

```text
HTTP:      http://127.0.0.1:8090/snapshot
WebSocket: ws://127.0.0.1:8091
```

## Next Phase

After successful integration inside StudioPulse, the plugin will be extracted into a standalone public repository and packaged independently.