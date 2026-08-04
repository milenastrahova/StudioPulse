# Virtual Studio Core Plugin

Virtual Studio Core is a reusable Unreal Engine 5.8 C++ plugin for real-time studio telemetry and backend communication.

## Modules

### VirtualStudioCore

Runtime module providing:

- HTTP JSON snapshot requests;
- WebSocket live telemetry;
- reconnect and exponential backoff;
- simulated fallback while reconnecting;
- Blueprint-spawnable connection component;
- Blueprint delegates and telemetry structs;
- Project Settings configuration;
- deterministic JSON validation;
- automation tests.

### VirtualStudioCoreEditor

Editor module providing:

- Tools menu integration;
- direct access to plugin settings;
- endpoint configuration self-test;
- editor-only tooling isolated from the runtime module.

## Blueprint API

Add `VirtualStudioConnectionComponent` to an Actor and bind:

- `On Telemetry Updated`
- `On Connection State Changed`
- `On Transport Error`

Callable functions:

- `Connect`
- `Disconnect`
- `Request Snapshot`
- `Send Text Message`

## Default Endpoints

```text
HTTP:      http://127.0.0.1:8090/snapshot
WebSocket: ws://127.0.0.1:8091
```

## Tests

```text
VirtualStudioCore.Reconnect.ExponentialBackoff
VirtualStudioCore.Parser.ValidPayload
VirtualStudioCore.Parser.AliasPayload
VirtualStudioCore.Parser.InvalidPayload
VirtualStudioCore.Configuration.EndpointValidation
VirtualStudioCore.Configuration.DefaultSettings
VirtualStudioCore.Performance.ComponentDoesNotTick
VirtualStudioCore.API.BlueprintSurface
```

## Author

Milena Strahova
