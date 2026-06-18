# Design: HarmonyOS/OpenHarmony Port

## Architecture

The port uses a three-layer structure:

```text
HarmonyOS HAP
  ArkTS Ability/Page
    XComponent
      libsoui_ohos.so
        NAPI/XComponent bridge
        swinx platform/ohos
        SOUI core
        SOUI components
```

The ArkTS shell owns application lifecycle and XComponent creation. The native layer owns SOUI initialization, Win32-compatible message dispatch, rendering, resources, and component loading.

## Build Selection

`swinx/CMakeLists.txt` selects platform cmake files in this order:

1. `ohos.cmake` for `OHOS|OpenHarmony|HarmonyOS`
2. `macos.cmake` for `Darwin`
3. `linux.cmake` otherwise

This avoids leaking Linux/XCB dependencies into OHOS builds.

## Backend Shape

The OHOS backend follows the macOS backend pattern:

- `SConnection` owns message queues, timers, focus/capture state, cairo surfaces, and clipboard/tray placeholders.
- `SOhosWindow` is the native-window facade. MVP uses virtual HWND handles and local rectangles.
- `SClipboard` starts as an in-process clipboard and will later bridge to HarmonyOS pasteboard.
- `napi_bridge` is an explicit placeholder for XComponent and ArkTS integration.

## Rendering Plan

Initial rendering uses cairo image surfaces:

```text
SOUI render-gdi -> SwinX HDC -> cairo image surface -> future NativeWindow buffer commit
```

`commitOhosWindow()` is currently a no-op placeholder. A later task will map cairo surface memory into `OH_NativeWindow_NativeWindowRequestBuffer()` output and flush the dirty region.

## Message/Input Plan

The backend provides an internal Win32-style message queue. Native XComponent callbacks will later translate surface, touch, mouse, keyboard, and IME events into `MSG` records.

The first skeleton supports:

- `PostMessage` style enqueue/dequeue;
- timer-generated `WM_TIMER`;
- window create/destroy/show/resize bookkeeping;
- `WM_PAINT` invalidation enqueue.

## Compatibility Strategy

Unsupported platform capabilities should fail closed:

- tray icon: return `FALSE`;
- drag/drop: return `E_NOTIMPL`;
- system clipboard: in-process only initially;
- multi-screen/window: return primary virtual display defaults;
- IME: no-op context association until NDK/ArkTS bridge is added.
