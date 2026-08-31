//
// SOUI for OpenHarmony/HarmonyOS NEXT —— ArkTS 层适配器
//
// 以下文件为【可粘贴到 Stage 工程】的独立代码片段（不包含 hvigor/package.json
// 等构建配置，请在 DevEco Studio 生成的 Stage 工程骨架上，将这些文件
// 放置到 entry/src/main/ets/soui/ 目录下，按下方说明接入）。
//
// 用法：
//   1. 将本目录所有 .ets 文件复制到 entry/src/main/ets/soui/
//   2. 在你的 UIAbility 的 onCreate 中调用：
//        SouiPlatformBridge.init(this.context);
//      onDestroy 调用：
//        SouiPlatformBridge.uninit();
//   3. 在页面（例如 Index.ets）中放置：
//        SouiScreen({ screenId: 1, layoutName: "layout:dlg_main" })
//   4. entry/src/main/cpp/CMakeLists.txt 链接 libsoui4ohos.so
//        （由 soui-ohos-lib 编译产物提供）

export * from './SouiPlatformBridge';
export * from './INativeWindow';
export * from './SouiSurface';
export * from './SouiSurfaceView';
export * from './SouiScreen';
export * from './NativeEditView';
export * from './NativeEditViewView';
export * from './NativeWindowDelegate';
export * from './SouiAbsLayout';
export * from './UiThreadUtils';
