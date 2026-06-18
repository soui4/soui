# Add HarmonyOS/OpenHarmony Port

## Summary

Add an OpenHarmony/HarmonyOS native port path for SOUI by introducing an OHOS backend in `swinx`, keeping SOUI's existing Win32-style programming model and DirectUI rendering pipeline.

The first implementation step adds platform detection, build-system routing, and a minimal `swinx/src/platform/ohos` backend skeleton. Later tasks will replace the stubbed OHOS window and buffer functions with real XComponent, NativeWindow, NAPI, input, IME, and pasteboard integrations.

## Motivation

SOUI already runs cross-platform by routing non-Windows builds through SwinX. HarmonyOS/OpenHarmony supports native C/C++ libraries, CMake toolchains, XComponent, NativeWindow, and ArkTS/Node-API bridges, making it practical to port SOUI without rewriting the control framework in ArkUI.

The migration should follow the existing SwinX architecture:

- preserve SOUI controls, layouts, resources, and animation code;
- implement Win32-compatible window/message/GDI semantics in a new OHOS backend;
- use a DevEco/ArkTS app shell only for lifecycle, permissions, XComponent creation, and selected platform bridges.

## Scope

In scope:

- Add OpenSpec documentation for the OHOS port.
- Add CMake platform detection for `OHOS`, `OpenHarmony`, and `HarmonyOS`.
- Add `swinx/ohos.cmake`.
- Add initial OHOS platform backend files under `swinx/src/platform/ohos`.
- Provide a minimal cairo image-surface based backend skeleton for future NativeWindow commits.
- Keep all existing Windows/Linux/macOS behavior unchanged.

Out of scope for the first code change:

- Real `OH_NativeXComponent` lifecycle registration.
- Real `OH_NativeWindow` buffer request/flush.
- Full IME, pasteboard, file picker, drag/drop, and multi-window support.
- Skia/GPU rendering optimization.
- DevEco sample HAP project.

## Impact

The change is isolated behind `CMAKE_SYSTEM_NAME` checks. Existing platforms continue to use current Windows, Linux, and macOS paths.

For OHOS, the first build will still require iterative fixes inside a real DevEco/OpenHarmony SDK environment. The initial backend intentionally returns conservative stubs for unsupported platform functions so that core integration points are explicit.
