//
// Index.ets —— @soui/ohos HAR 统一导出入口
//
// APP 侧接入方式：
//   import { SouiPlatformBridge, SouiScreen } from '@soui/ohos';
//
// 原先"复制全部 .ets 到 entry/src/main/ets/soui/"的接入方式已废弃，
// 详见本模块根目录 README.md。
//

export * from './src/main/ets/SouiPlatformBridge';
export * from './src/main/ets/INativeWindow';
export * from './src/main/ets/SouiSurface';
export * from './src/main/ets/SouiSurfaceView';
export * from './src/main/ets/SouiScreen';
export * from './src/main/ets/NativeEditView';
export * from './src/main/ets/NativeEditViewView';
export * from './src/main/ets/NativeWindowDelegate';
export * from './src/main/ets/SouiAbsLayout';
export * from './src/main/ets/UiThreadUtils';
export * from './src/main/ets/AudioPlayer';
