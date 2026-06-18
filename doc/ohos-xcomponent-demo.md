# SOUI OHOS XComponent Demo

This note describes the current minimal XComponent path.

## Build Native Libraries

Run from the SOUI root:

```powershell
python build_ohos.py
```

The arm64-v8a shared libraries are copied to:

```text
output/ohos/arm64-v8a
```

Copy these `.so` files into the DevEco module native library directory for the same ABI.

## ArkTS Page

Use `libswinx.so` as the XComponent native library. The native module registers the XComponent lifecycle callbacks during NAPI initialization and draws a simple frame when the surface is created or changed.

```ts
import swinx from 'libswinx.so';

@Entry
@Component
struct SouiXComponentPage {
  build() {
    Column() {
      XComponent({
        id: 'soui-xcomponent',
        type: XComponentType.SURFACE,
        libraryname: 'swinx'
      })
        .width('100%')
        .height('100%')
        .onLoad(() => {
          swinx.showDemo();
        })
    }
    .width('100%')
    .height('100%')
  }
}
```

Expected first frame:

- dark top band titled `SOUI HarmonyOS`
- white content panel
- blue `Native UI` button
- touch feedback circle while pressing/moving on the XComponent

## Native Exports

`libswinx.so` exports a NAPI module named `swinx`:

- `registerXComponent(xcomponent): boolean`
- `showDemo(): boolean`

When ArkUI supplies `__NATIVE_XCOMPONENT_OBJ__` through the XComponent `libraryname`, registration is automatic. `registerXComponent()` is kept for explicit registration paths.

## Current Limits

This is a native surface smoke test. It proves XComponent lifecycle, `OHNativeWindow` storage, buffer geometry/format setup, CPU pixel copy, and flush. It does not yet create a full SOUI `SHostWnd` tree from ArkTS resources.
