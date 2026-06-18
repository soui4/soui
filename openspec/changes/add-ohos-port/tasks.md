# Tasks

## 1. Proposal and Skeleton

- [x] Add OpenSpec proposal, design, and task list.
- [x] Add OHOS CMake platform detection.
- [x] Route `swinx` to `ohos.cmake`.
- [x] Add OHOS backend source skeleton.
- [x] Keep existing platform paths unchanged.

## 1.1 Toolchain and Build Script

- [x] Add `build_ohos.py` for DevEco SDK toolchain discovery, configure, build, strip, and copy.
- [x] Support DevEco `hms/native/build/cmake/hmos.toolchain.cmake`.
- [x] Support DevEco SDK CMake/Ninja and BiSheng clang.
- [x] Seed cross-compile `try_run` cache values required by cairo/fontconfig.
- [x] Verify `python build_ohos.py` builds arm64-v8a shared libraries and copies them to `output/ohos/arm64-v8a`.

## 2. DevEco/XComponent Bootstrap

- [x] Add a minimal DevEco ArkTS usage snippet.
- [x] Export a NAPI init entry for `libswinx.so`.
- [x] Obtain `OH_NativeXComponent` from module exports.
- [x] Register surface lifecycle callbacks.
- [x] Store `OHNativeWindow` and surface dimensions in the OHOS backend.

## 3. First Frame Rendering

- [x] Convert `commitOhosWindow()` to request and flush `OHNativeWindowBuffer`.
- [x] Handle buffer geometry, format, stride, and dirty rect copy.
- [x] Trigger demo repaint from surface-created and surface-changed callbacks.
- [ ] Validate first native XComponent frame on device/emulator.
- [ ] Validate first SOUI host window frame on device/emulator.

## 4. Basic Interaction

- [ ] Map XComponent touch events to mouse messages.
- [ ] Add mouse callbacks for PC/2-in-1 devices.
- [ ] Add keyboard callbacks and virtual-key mapping.
- [ ] Add basic `WM_CHAR` text input.
- [ ] Validate button/list/edit/combo/menu basics.

## 5. Platform Bridges

- [ ] Add ArkTS or NDK pasteboard bridge.
- [ ] Add IME bridge for self-drawn text controls.
- [ ] Add file picker bridge.
- [ ] Add lifecycle pause/resume handling.
- [ ] Decide RichEdit support level.

## 6. Productization

- [ ] Evaluate Skia/GPU path.
- [ ] Add automated smoke tests.
- [ ] Validate memory and frame pacing.
- [ ] Create packaging documentation.
- [ ] Audit third-party libraries and licenses for HAP distribution.
