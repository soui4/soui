# SOUI HarmonyOS/OpenHarmony Port Specification

## ADDED Requirements

### Requirement: OHOS Build Routing

The build system SHALL detect `CMAKE_SYSTEM_NAME` values matching `OHOS`, `OpenHarmony`, or `HarmonyOS` and route SwinX to an OHOS platform backend instead of the Linux/XCB backend.

#### Scenario: Configure OHOS Build

- **GIVEN** CMake is invoked with an OHOS/OpenHarmony/HarmonyOS toolchain
- **WHEN** `swinx` is configured
- **THEN** `swinx/ohos.cmake` is included
- **AND** Linux-only dependencies such as XCB, xkbcommon, dbus, and ALSA are not required by the SwinX target.

### Requirement: Isolated OHOS Backend

The OHOS backend SHALL live under `swinx/src/platform/ohos` and SHALL not affect Windows, Linux, or macOS source selection.

#### Scenario: Non-OHOS Build

- **GIVEN** a Windows, Linux, or macOS build
- **WHEN** the project is configured
- **THEN** no OHOS backend source file is added to existing platform targets.

### Requirement: Minimal Win32 Message Compatibility

The OHOS backend SHALL provide a `SConnection` implementation capable of creating virtual HWND handles, enqueueing/dequeueing `MSG` values, servicing timers, and generating repaint requests.

#### Scenario: Post and Receive Message

- **GIVEN** an OHOS `SConnection`
- **WHEN** a Win32-style message is posted
- **THEN** `peekMsg` or `getMsg` can retrieve that message using the existing SwinX message API.

### Requirement: Cairo Surface Placeholder

The OHOS backend SHALL create cairo image surfaces for window render targets until a real NativeWindow buffer backend is implemented.

#### Scenario: Create Window Surface

- **GIVEN** a created OHOS virtual HWND
- **WHEN** `CreateWindowSurface` is called
- **THEN** a non-null `cairo_image_surface` is returned for positive dimensions.

### Requirement: Conservative Unsupported Platform Behavior

Unsupported OHOS platform features SHALL return explicit no-op or failure values rather than using Linux/XCB or macOS APIs.

#### Scenario: Tray Icon Before Native Support

- **WHEN** `NotifyIcon` is called on the OHOS backend
- **THEN** it returns `FALSE`.
