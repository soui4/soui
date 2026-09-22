# soui-ohos-lib

SOUI for OpenHarmony/HarmonyOS NEXT 平台适配模块（**HAR 库**）。

- `src/main/ets/` —— ArkTS 层适配器（窗口、Surface、输入法桥、定时器等），经根部 `Index.ets` 统一导出。
- `src/main/cpp/` —— Native 层源码：`libsoui4ohos.so` 的 NAPI 桥与平台 API 实现，以及 NAPI 类型包 `src/main/cpp/types/libsoui4ohos`（`Index.d.ts`）。
- `src/main/module.json5` —— HAR 模块描述（模块名 `soui_ohos_lib`，注意模块名不允许连字符）。

> **重要**：本模块已改造为标准 HAR，`README.ets`（"把 .ets 复制到 app 里"）方式**已废弃**——不要再复制 `.ets` 源码，直接依赖本包即可。

## Native 层编译归属（与 Android 端先例一致）

本 HAR **只打包 ArkTS 层与 NAPI 类型声明**，`libsoui4ohos.so` 仍由各业务 app 在自己的
`entry/src/main/cpp/CMakeLists.txt` 中通过 `include(<soui-ohos-lib>/src/main/cpp/soui4_ohos.cmake)`
从源码编译（理由同 `soui-android-lib`：业务 `.so` 直接链接 SOUI 的 C++ 符号，跨模块分发
预编译 `.so` 需要同步 `config.h`、`WCHAR_SIZE`、编译宏，ABI 脆弱）。

## 仓库内 app 接入（源码依赖，推荐开发期使用）

1. 在 app 的 `entry/oh-package.json5` 中加入：

   ```json5
   "dependencies": {
     "@soui/ohos": "file:../../../soui-ohos-lib"   // 按实际相对路径
   }
   ```

2. **在 app 工程级 `build-profile.json5` 的 `modules` 列表中注册本模块**（缺失会导致
   ArkTS 报 00309001 "Cannot import files from an external module using relative paths" /
   10311002 "Failed to resolve OhmUrl"，`useNormalizedOHMUrl` 开启时尤其如此）：

   ```json5
   "modules": [
     { "name": "soui_ohos_lib", "srcPath": "../../../soui-ohos-lib",   // 模块名不允许连字符
       "targets": [ { "name": "default", "applyToProducts": [ "default" ] } ] },
     // ...原有 entry 等模块
   ]
   ```

3. 执行 `ohpm install`（或 DevEco Studio 里 Sync）。
4. 代码中直接 import：

   ```ts
   import { SouiPlatformBridge, SouiScreen } from '@soui/ohos';
   ```

   > ⚠️ 注意：包内 `.ets` 的相互引用使用相对路径；包名自引用（`@soui/ohos/src/main/ets/...`）
   > 在 `file:` 源码依赖形态下无法解析，**不要使用**。

## 仓库外 app 接入（HAR 产物）

```bash
cd soui-ohos-lib && hvigorw assembleHar
# 产物：build/default/outputs/@soui-ohos.har（或同类目录）
```

把 `.har` 放进 app 的 `libs/`，然后：

```json5
"dependencies": {
  "@soui/ohos": "file:./libs/@soui-ohos.har"
}
```

## Native 接入

app 的 `entry/src/main/cpp/CMakeLists.txt` 中（**不要** `add_subdirectory` 本模块的
`CMakeLists.txt`，统一 include 构建片段——与 `soui4_android.cmake` 同一约定）：

```cmake
set(SOUI_ROOT_DIR <soui4 仓库根的绝对或相对路径>)
set(SOUI_OHOS_LIB_CPP_DIR ${SOUI_ROOT_DIR}/soui-ohos-lib/src/main/cpp)
include(${SOUI_OHOS_LIB_CPP_DIR}/soui4_ohos.cmake)

# SOUI 配置头生成在 ${PROJECT_BINARY_DIR}/config，业务 so 需要时加入 include 目录
target_include_directories(<业务 so> PRIVATE ${PROJECT_BINARY_DIR}/config)
target_link_libraries(<业务 so> PUBLIC soui4ohos)
```

`soui4_ohos.cmake` 要求调用前设好 `SOUI_ROOT_DIR` 与 `SOUI_OHOS_LIB_CPP_DIR` 两个变量；
片段内部完成 SOUI 核心（soui4/utilities4/swinx/third-part/components）的 `add_subdirectory`
与 `soui4ohos` 共享库的编译。`SOUI_BUILD_RICHEDIT` / `SOUI_BUILD_WS` 两个重量级开关可由
业务侧经 `-D` 预设覆盖（默认开启）。

`libsoui4ohos.so` 的 NAPI 类型由本 HAR 的 `oh-package.json5` 以
`"libsoui4ohos.so": "file:./src/main/cpp/types/libsoui4ohos"` 声明并传递给 app；
app 的 `entry/oh-package.json5` 中**不再需要**单独声明 `libsoui4ohos.so` 类型映射。

## APP 侧最小接入代码

```ts
// UIAbility.onCreate
SouiPlatformBridge.init(this.context);
// UIAbility.onDestroy
SouiPlatformBridge.uninit();
```

```ts
// 页面
import { SouiScreen } from '@soui/ohos';

SouiScreen({ screenId: 1, layoutName: "layout:dlg_main" })
```
