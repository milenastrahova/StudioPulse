# Performance Methodology

StudioPulse is a small real-time dashboard, so performance work focuses on predictable update cost, network behaviour, and avoiding unnecessary per-frame work.

## Existing Design Decisions

- Data ownership is centralized in a `UGameInstanceSubsystem`.
- UI updates are event-driven through a multicast delegate.
- Reconnect work uses scheduled timers instead of busy polling.
- Automatic fallback avoids unstable repeated visual state changes.
- The dashboard is built once through C++ WidgetTree.
- Activity history is bounded to a fixed sample count.

## Measurement Plan

The next measured profiling pass will capture:

1. `stat unit`
2. `stat game`
3. `stat gpu`
4. `stat memory`
5. `stat net`
6. Unreal Insights CPU trace
7. WebSocket reconnect timing
8. HTTP and WebSocket payload sizes
9. packaged Shipping build memory use

## Results Policy

No performance figure is published until it has been measured in a repeatable scenario. This document intentionally contains no invented FPS, memory, or network numbers.

## Planned Test Scenarios

| Scenario | Purpose |
|---|---|
| Live feed | Normal message parsing and dashboard updates |
| Warning feed | Alert rendering and high-latency state |
| Offline feed | Connection-loss state |
| Automatic fallback | Silent reconnect while simulated data remains visible |
| Service restoration | Automatic transition from fallback to live data |