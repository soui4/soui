# SOUI5 鸿蒙（OpenHarmony / HarmonyOS NEXT）开发入门指南

> **适用版本**：SOUI 5.x（仓库目录名仍为 `soui4`；`ChangeLog` 中 5.3 初步完成鸿蒙适配）
> **目标读者**：准备用 SOUI5 开发鸿蒙应用的工程师（有 Windows 端 SOUI / WTL 经验者几乎零迁移成本）
> **配套示例**：`demos/ohos-demo`（最小可运行工程）、`games/cnchess/client/ohos`（完整业务：象棋 + 联网对战）
> **适配库说明**：`soui-ohos-lib/README.md`
> **姊妹文档**：`doc/SOUI5-Android开发入门指南.md`（两端架构一一对应，可对照阅读）

---

## 1. 先理解鸿蒙端由什么组成

SOUI5 在鸿蒙上的做法与 Android 端同构：把 SOUI 依赖的 **Win32 窗口 / 消息 / GDI 模型**映射到 **ArkTS 组件 + NativeWindow + Skia/Cairo**，Windows 端的布局 XML、皮肤、事件映射、`SetTimer` 等业务代码**源码级复用**。

| 组成 | 位置 | 交付形态 |
|---|---|---|
| ArkTS 适配层 | `soui-ohos-lib/src/main/ets/` | **HAR**（包名 `@soui/ohos`，根部 `Index.ets` 统一导出） |
| NAPI 类型声明 | `soui-ohos-lib/src/main/cpp/types/libsoui4ohos/` | 随 HAR 分发（app 无需再声明） |
| native 适配层源码 | `soui-ohos-lib/src/main/cpp/` | **源码**（不预编译，见 1.1） |
| native 构建片段 | `soui-ohos-lib/src/main/cpp/soui4_ohos.cmake` | app 通过 `include()` 复用 |
| 平台入口接口 | `soui-ohos-lib/src/main/cpp/include/soui4ohos.h` | 业务层实现 `Soui4OhosEntry` |
| SOUI 核心 | `SOUI/`、`utilities/`、`swinx/`、`components/`、`third-part/` | 由构建片段一并编译成 `.so` |

核心概念映射：

| Windows 概念 | 鸿蒙等价物 |
|---|---|
| `HWND` | 长整数句柄（`SouiPlatformBridge` 内部 HWND ↔ 组件映射，**不要当指针强转**） |
| `SHostWnd` | `SouiScreen`（ArkTS 组件，承载 SOUI 主窗口）+ `SouiSurface`（绘制面） |
| `SendMessage / PostMessage` | swinx `WndMgr` + 内部消息队列 + ArkTS 侧消息排空调度 |
| `WM_LBUTTONDOWN / WM_KEYDOWN / WM_CHAR` | `SouiSurface` 触摸/按键事件转成 Win32 消息 |
| `SetTimer / KillTimer` | `SouiPlatformBridge.setTimer / killTimer`（ArkTS `setTimeout`） |
| `InvalidateRect` | `SouiPlatformBridge.invalidateRect`（按 HWND 找到 `INativeWindow` 触发重绘） |
| 输入法编辑框 | `NativeEditView`（原生输入法组件），文本经 slot 跨层传递 |

### 1.1 HAR 与 native 的分工（与 Android 端同一策略）

- **HAR 只打包 ArkTS 层 + NAPI 类型声明**：app 通过 `oh-package.json5` 的 `file:` 依赖（开发期源码依赖）或 `.har` 产物引用，**不再复制 `.ets` 源码**。
- **`libsoui4ohos.so` 由业务 app 编译**：`entry` 的 CMake 通过 `include(soui4_ohos.cmake)` 编译 SOUI 核心与 OHOS N-API 桥。
  理由同 Android：业务 `.so` 直接链接 SOUI 的 C++ 符号，预编译分发需要同步 `config.h`、`WCHAR_SIZE`、编译宏，ABI 脆弱。

### 1.2 `Soui4OhosEntry`：平台层与业务层的唯一接口

业务 C++ 代码只需实现 4 个虚函数（`soui4ohos.h`，与 Android 的 `Soui4AndroidEntry` 一一对应）：

```cpp
struct Soui4OhosEntry {
    virtual BOOL InitApp(SouiOhosAssetManager assetMgr, LPCSTR pszAssetDir) = 0; // 创建 SApplication + 配资源
    virtual void UninitApp() = 0;                                                // 释放
    virtual HWND ScreenStartup(long screenId, LPCSTR pszLayout) = 0;             // 建主窗口
    virtual void ScreenShutdown(long screenId) = 0;                              // 关窗口
};

void InitSoui4OhosEntry(Soui4OhosEntry* entry);   // 业务侧在静态构造中注册
```

### 1.3 三条"名字必须一致"

这是新手最容易踩的坑，**三处名字必须完全相同**：

1. CMake 目标名（决定产物 `libXxx.so` 的文件名）
2. ArkTS 侧 `import xxx from 'libXxx.so'`
3. NAPI 包的 `nm_modname`（native 侧 `napi_module` 结构体）

任一处不一致，编译能过，运行期会崩在 `Cannot read property init of undefined` 这类错误上（表现为"模块是 undefined"）。

---

## 2. 环境准备

| 项 | 要求 | 说明 |
|---|---|---|
| DevEco Studio | 6.x（HarmonyOS NEXT 版本） | 内置 hvigor / ohpm / JBR / SDK |
| HarmonyOS SDK | API 26 及以上 | DevEco 自带，或从 SDK Manager 安装 |
| 设备 | 模拟器或真机 | 模拟器装未签名 HAP 即可；真机需配置签名 |
| （可选）命令行 | `hvigorw` + `ohpm` + JBR | 用于 CI / 无 IDE 构建，见 7.2 |

### 2.1 拉取源码

```bash
git submodule update --init --recursive    # swinx/、third-part/ 是子模块
```

---

## 3. 五分钟跑通示例

### 3.1 DevEco 方式（推荐首次使用）

1. DevEco Studio 打开 `demos/ohos-demo`（或 `games/cnchess/client/ohos`）
2. 等待 hvigor / ohpm Sync 完成（首次会解析 `@soui/ohos` 源码依赖并编译 SOUI 核心，耗时较长）
3. 连接模拟器 / 真机，点 Run

### 3.2 命令行方式

```bash
cd demos/ohos-demo
ohpm install                      # 解析 @soui/ohos 源码依赖
hvigorw --mode module -p module=entry@default -p product=default -p buildMode=debug \
        assembleHap --no-daemon   # 产物 entry/build/default/outputs/default/entry-default-unsigned.hap

hdc install -r entry/build/default/outputs/default/entry-default-unsigned.hap
hdc shell aa start -a EntryAbility -b com.soui.ohos.demo2
```

未签名 HAP 可以直接安装到**模拟器**；真机安装需要在 DevEco 的 `File → Project Structure → Signing Configs` 配置自动签名后重新打包。

---

## 4. 从零创建自己的鸿蒙工程

### 4.1 新建工程

DevEco Studio → **New Project → Application → Empty Ability**（Stage 模型，API 26+）。约定工程与 SOUI 源码仓库并列：

```text
<workspace>/
├── soui4/                                   # SOUI5 源码仓库（含 soui-ohos-lib/）
└── MyOhosApp/
    ├── AppScope/
    │   ├── app.json5                        # bundleName / 版本 / app 图标、名称
    │   └── resources/base/
    │       ├── element/string.json
    │       └── media/{background.png, foreground.png, layered_image.json}
    ├── build-profile.json5                  # 工程级：products / signingConfigs / modules
    ├── oh-package.json5
    └── entry/
        ├── build-profile.json5
        ├── oh-package.json5
        └── src/main/
            ├── module.json5
            ├── ets/
            │   ├── entryability/EntryAbility.ets
            │   └── pages/Index.ets
            ├── cpp/
            │   ├── CMakeLists.txt
            │   ├── MyOhosApp.cpp            # Soui4OhosEntry 实现 + NAPI 模块注册
            │   └── MainDlg.h / MainDlg.cpp   # SHostWnd 子类
            └── resources/
                ├── base/media/{background.png, foreground.png, layered_image.json, startIcon.png}
                └── rawfile/
                    ├── uires/               # 业务资源（含 uires.idx）
                    └── soui_sys_res/        # SOUI 系统资源（含 uires.idx）
```

### 4.2 引入 HAR：`entry/oh-package.json5`

```json5
{
  "name": "entry",
  "version": "1.0.0",
  "dependencies": {
    "@soui/ohos": "file:../../../soui-ohos-lib"   // 按实际相对路径；仓库外可用打包好的 .har
  }
}
```

### 4.3 **工程级 `build-profile.json5` 必须注册该模块**（最容易漏的一步）

`file:` 依赖只解决"包能解析到"，**hvigor 还需要知道这个模块归属**，否则模块内部的相对路径 import 全部会被判成"外部模块导入"，直接报 `00309001 Cannot import files from an external module using relative paths`（有时伴随 `10311002 Failed to resolve OhmUrl`）：

```json5
{
  "app": { /* 略 */ },
  "modules": [
    {
      "name": "soui_ohos_lib",                       // 模块名不允许连字符（^[a-zA-Z][0-9a-zA-Z_.]*$）
      "srcPath": "../../../soui-ohos-lib",           // 允许指向工程外的目录
      "targets": [ { "name": "default", "applyToProducts": [ "default" ] } ]
    },
    {
      "name": "entry",
      "srcPath": "./entry",
      "targets": [ { "name": "default", "applyToProducts": [ "default" ] } ]
    }
  ]
}
```

> 说明：HAR 自身的 `src/main/module.json5`（`soui-ohos-lib` 已提供）要求顶层有 `"module": { ... }` 包装键，且 `name` 用下划线形式 `soui_ohos_lib`、`type` 为 `har`、`deviceTypes` 含 `default`（新版 SDK 不再用 `phone`）。
> 与本条无关的干扰项：HAR 内部不要用"包名自引用"（`@soui/ohos/src/main/ets/...`）——在 `file:` 源码依赖形态下解析不了，包内相互引用统一用相对路径。

### 4.4 安装依赖

```bash
ohpm install            # 或者在 DevEco 里 Sync
```

首次会生成 `entry/oh-package-lock.json5`，之后改动依赖再 Sync 会更新该文件，属正常现象。

### 4.5 `entry/src/main/cpp/CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.18.1)

project("myohosapp")

# SOUI 仓库根目录（按实际相对层级调整），soui-ohos-lib 与 SOUI/ 同级
set(SOUI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../..)
get_filename_component(SOUI_ROOT_DIR "${SOUI_ROOT_DIR}" ABSOLUTE)
set(SOUI_OHOS_LIB_CPP_DIR ${SOUI_ROOT_DIR}/soui-ohos-lib/src/main/cpp)

# 引入可复用的构建片段：内部会编译 SOUI 核心并产出 soui4ohos 共享库
# 片段要求调用前设好 SOUI_ROOT_DIR 与 SOUI_OHOS_LIB_CPP_DIR
include(${SOUI_OHOS_LIB_CPP_DIR}/soui4_ohos.cmake)

# 自己的业务 native 代码
file(GLOB MYOHOSAPP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# ⚠️ 目标名 = so 文件名 = ArkTS import 名（三处一致，见 1.3）
add_library(soui_myapp SHARED ${MYOHOSAPP_SRC})

target_include_directories(
        soui_myapp
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SOUI_OHOS_LIB_CPP_DIR}/include        # soui4ohos.h
        ${SOUI_ROOT_DIR}/SOUI/include
        ${SOUI_ROOT_DIR}/utilities/include
        ${SOUI_ROOT_DIR}/swinx/include
        ${SOUI_ROOT_DIR}/components
        ${PROJECT_BINARY_DIR}/config            # config.h 生成在这里
)

target_link_libraries(
        soui_myapp
        PUBLIC
        soui4ohos                               # 内部已 PUBLIC 链接 soui4/utilities4/swinx/组件
)
```

要点：

- **不要 `add_subdirectory(soui-ohos-lib/src/main/cpp)`**，统一 `include(soui4_ohos.cmake)`（与 Android 端的 `soui4_android.cmake` 同一约定）。
- `config.h` / `soui-config.cmake` 生成在 **宿主工程的 `${PROJECT_BINARY_DIR}/config`**（include 方式下 `PROJECT_BINARY_DIR` 属于你的 app，不再是子目录）。
- 片段默认打开 `SOUI_BUILD_RICHEDIT` / `SOUI_BUILD_WS`。不需要联网/富文本能力时，可在 `entry/build-profile.json5` 的 `buildOption.externalNativeOptions` 里把 CMake 参数传进去，显著缩短配置与编译时间：

  ```json5
  "buildOption": {
    "externalNativeOptions": {
      "path": "./src/main/cpp/CMakeLists.txt",
      "arguments": [ "-DSOUI_BUILD_WS=OFF" ],
      "abiFilters": [ "arm64-v8a", "x86_64" ]   // 真机 arm64-v8a；模拟器按实际（常见 x86_64）
    }
  }
  ```
- 平台编译宏（`SOUI_OHOS`、`__OHOS__`、`WCHAR_SIZE=4`、`SOUI_RENDER_USE_SKIA` 等）片段已统一 `add_definitions`；业务侧只需额外补自己的宏（例如 `__MOBILE__`）。

### 4.6 ArkTS 侧

**`entry/src/main/ets/entryability/EntryAbility.ets`** —— 生命周期 + 启动 SOUI：

```ts
import { UIAbility, AbilityConstant, Want, common } from '@kit.AbilityKit';
import { window } from '@kit.ArkUI';
import myapp from 'libsoui_myapp.so';                  // ← 与 CMake 目标名一致
import { SouiPlatformBridge } from '@soui/ohos';       // ← 来自 HAR

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    myapp.init();                                      // 触发业务 so 加载 + Soui4OhosEntry 注册
    SouiPlatformBridge.instance().init(this.context);  // 初始化平台桥（加载 libsoui4ohos.so）
    this.startSouiApp();                               // 资源从 HAP rawfile 直读，无需复制
  }

  private startSouiApp(): void {
    SouiPlatformBridge.instance().startSouiApp();
  }

  onDestroy(): void {
    SouiPlatformBridge.instance().stopSouiApp();
    SouiPlatformBridge.instance().uninit();
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    windowStage.loadContent('pages/Index');
  }
}
```

**`entry/src/main/ets/pages/Index.ets`** —— 页面里放 SOUI 渲染宿主：

```ts
import { SouiScreen } from '@soui/ohos';

@Entry
@Component
struct Index {
  build() {
    SouiScreen({ screenId: 1, layoutName: 'layout:dlg_main' })
      .width('100%')
      .height('100%')
  }
}
```

`SouiScreen` 首次尺寸就绪后会自动回调 `registerScreen` → C++ 侧 `ScreenStartup(screenId, layout)`，无需手工触发。

### 4.7 C++ 侧：入口实现 + NAPI 模块

```cpp
#include <souistd.h>
#include <SAppCfg.h>
#include <rawfile/raw_file_manager.h>
#include "soui4ohos.h"
#include "MainDlg.h"

using namespace SNS;

class MyAppEntry : public Soui4OhosEntry {
    SApplication* m_app = nullptr;
public:
    MyAppEntry() { InitSoui4OhosEntry(this); }        // 静态构造里注册自己

    BOOL InitApp(SouiOhosAssetManager assetMgr, LPCSTR pszAssetDir) override {
        m_app = new SApplication((HINSTANCE) nullptr);
        SAppCfg cfg;
        cfg.SetRender(Render_Skia)
           .SetImgDecoder(ImgDecoder_Stb)
           .SetAppDir(S_CA2T(pszAssetDir, CP_UTF8));
        // assetMgr 实际是 OH_ResourceManager_InitNativeResourceManager 返回的
        // NativeResourceManager*，据此直读 HAP rawfile 下的 uires / soui_sys_res
        return cfg.DoConfig(m_app);
    }

    void UninitApp() override { m_app->Release(); m_app = nullptr; }

    HWND ScreenStartup(long screenId, LPCSTR pszLayout) override {
        SAutoRefPtr<SHostWnd> host = new CMainDlg();   // 你的 SHostWnd 子类
        if (!host->Create(nullptr, 0, 0, 0, 0)) return 0;
        host->ShowWindow(SW_SHOW);
        return host->m_hWnd;
    }

    void ScreenShutdown(long screenId) override { /* 释放对应 host */ }
};

static MyAppEntry s_app;      // 静态实例 ⇒ 进程启动即完成注册

// ---- NAPI 模块：名字必须与 so 名一致（见 1.3）----
static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = MyModuleInit,       // 里面对外暴露 init() 等方法
    .nm_modname = "libsoui_myapp",          // 与 add_library 目标名一致
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterMyModule(void) {
    napi_module_register(&g_module);
}
```

`SHostWnd` 子类的写法（消息映射、事件订阅、`SetTimer`）与 Windows 端**完全一致**，可直接搬 `demos/ohos-demo/entry/src/main/cpp/MainDlg.*` 或 `games/cnchess/client/ohos` 的实现。

---

## 5. SOUI 资源怎么放

- 资源放在 **`entry/src/main/resources/rawfile/`** 下，两套目录：
  - `rawfile/soui_sys_res/` —— SOUI **系统资源**（控件默认皮肤，可从 `soui-sys-resource/` 拷贝）
  - `rawfile/uires/` —— **业务资源**（`xml/` 布局、`values/` 皮肤与字符串、`image/`…）
- **两套资源各有一份 `uires.idx`，都需要手工维护**。它是 UTF-8 XML、根节点 `<resource>`、按资源类型分组，每项一条 `<file name="逻辑名" path="相对 idx 的文件路径"/>`。**新增、改名或删除资源文件后必须手动补改这个文件**（不会自动扫描目录），否则运行期按 `"layout:dlg_main"` 这类逻辑名查不到资源 —— 其中 `dlg_main` 即 `<LAYOUT>` 里的 `name`。（成批新增图片条目可用 `tools/uiresImporter.exe` 辅助，它读入已有 `uires.idx` 重建指定类型节点后回写，但**不新建**该文件、也不处理布局。）
- **`tools/uiresbuilder` 不生成 `uires.idx`，也不参与鸿蒙构建**。它把 `uires.idx` 当**输入**（`-i`），产出 `.rc2`（`-r`）/ `resource.h`（`-h`）/ 可选 `R.js`（`-j`）。**三个桌面端（Windows / Linux / macOS）都用它**：Windows 由 MSVC `rc.exe` 把 `.rc2` 编进 PE；Linux / macOS 由 MinGW `windres` 把同一份 `.rc2` 编译成 COFF，再由 `__cmake/windres.cmake` 转成 ELF / Mach-O 链进可执行文件。**鸿蒙端不产生也不使用这三份产物**，布局直接按字符串名加载。
- C++ 侧通过 **`OH_ResourceManager_InitNativeResourceManager`** 拿到的资源管理器，由 `SResProviderOhosRawFile` 在**运行期解析 `rawfile/uires/uires.idx`** 建立"类型 + 逻辑名 → 物理路径"映射，因此 `uires.idx` 必须随 rawfile 打进 HAP，**不需要**把资源解压到 `filesDir`（早期版本曾这样做，现已弃用）。
- 资源更新（含 `uires.idx` 改动）后重新构建安装即可生效。

---

## 6. 应用图标与启动图

鸿蒙应用图标使用**分层图标（layered icon）**，与 Android 的一张 `ic_launcher.png` 不同，需要三张图：

| 文件 | 尺寸 | 内容 |
|---|---|---|
| `background.png` | 1024×1024 | 纯底色（启动器会按圆形/方圆遮罩裁切，所以要铺满、不要留白边） |
| `foreground.png` | 1024×1024 | 透明底 + 图标主体，**内容控制在中心 2/3（约 682px）安全区内** |
| `startIcon.png` | 1024×1024 | 启动窗口图（可用"底色 + 稍大的主体"） |

放置位置（两处都要）：

```text
AppScope/resources/base/media/{background.png, foreground.png, layered_image.json}   # app 级图标
entry/src/main/resources/base/media/{background.png, foreground.png, startIcon.png, layered_image.json}  # ability 图标 + 启动图
```

`layered_image.json` 内容固定，引用上面两张图：

```json5
{
  "layered-image": {
    "background": "$media:background",
    "foreground": "$media:foreground"
  }
}
```

两个 json5 各自引用：

```json5
// AppScope/app.json5
{ "app": { "bundleName": "com.yourco.myapp", "icon": "$media:layered_image", "label": "$string:app_name" } }
```

```json5
// entry/src/main/module.json5 → abilities[0]
{
  "name": "EntryAbility",
  "srcEntry": "./ets/entryability/EntryAbility.ets",
  "icon": "$media:layered_image",
  "label": "$string:EntryAbility_label",
  "startWindowIcon": "$media:startIcon",
  "startWindowBackground": "$color:start_window_background",
  "exported": true,
  "skills": [ { "entities": ["entity.system.home"], "actions": ["ohos.want.action.home"] } ]
}
```

工程模板默认给的就是鸿蒙默认四叶草图，替换掉 `background/foreground/startIcon` 三张 png 即可（`games/cnchess/client/ohos` 已用本应用印章图替换，可参考）。

---

## 7. 构建、安装、调试

### 7.1 DevEco Studio

- **Sync**：改完依赖 / 工程配置后先 Sync（等价 `ohpm install` + 重新生成 lock）
- **Run**：自动完成 ArkTS 编译 + native 编译 + HAP 打包 + 安装 + 启动
- **真机**：`File → Project Structure → Signing Configs` 勾选自动签名

### 7.2 命令行（CI / 无 IDE）

前置：把 DevEco 自带的工具链加进环境（Windows Git Bash 示例）：

```bash
export DEVECO_SDK_HOME="C:\\Program Files\\Huawei\\DevEco Studio\\sdk"
export PATH="/c/Program Files/Huawei/DevEco Studio/tools/hvigor/bin:\
/c/Program Files/Huawei/DevEco Studio/tools/ohpm/bin:\
/c/Program Files/Huawei/DevEco Studio/jbr/bin:$PATH"

cd <工程目录>
ohpm install
hvigorw --mode module -p module=entry@default -p product=default -p buildMode=debug \
        assembleHap --no-daemon
```

- `jbr/bin` 是必需的：HAP 签名阶段会 `spawn java`，缺失会报 `spawn java ENOENT`
- SDK 路径也可用 DevEco 安装目录下的 `sdk`（`DEVECO_SDK_HOME` 指向它即可）
- 输出：`entry/build/default/outputs/default/entry-default-unsigned.hap`

### 7.3 安装、启动与观察

```bash
HDC="C:/Program Files/Huawei/DevEco Studio/sdk/default/openharmony/toolchains/hdc.exe"

$HDC list targets                                   # 查看设备（模拟器通常是 127.0.0.1:5555）
$HDC install -r <path>/entry-default-unsigned.hap    # 安装（模拟器可装未签名包）
$HDC shell aa start -a EntryAbility -b com.yourco.myapp
$HDC shell pidof com.yourco.myapp                    # 进程还活着说明没崩
$HDC shell hilog -x > log.txt                        # 抓日志（本地过滤 SOUI/崩溃关键字）
$HDC shell snapshot_display -f /data/local/tmp/a.jpeg && $HDC file recv /data/local/tmp/a.jpeg a.jpeg  # 截屏看渲染
```

> **Git Bash 用户注意**：Git Bash 会把 `/data/local/tmp/...` 自动改写成 Windows 路径，命令前加
> `export MSYS2_ARG_CONV_EXCL='*' MSYS_NO_PATHCONV=1` 规避；`file recv` 的本地目标用**相对路径**最省事。

---

## 8. 常见问题

| 现象 / 报错 | 原因 | 处理 |
|---|---|---|
| `00309001 Cannot import files from an external module using relative paths`（可能伴随 `10311002 Failed to resolve OhmUrl`） | **HAR 未注册进工程级 `build-profile.json5` 的 `modules`** —— hvigor 无法归属其文件，把包内相对 import 全判成"外部模块导入" | 按 4.3 注册 `soui_ohos_lib`（注意与 `useNormalizedOHMUrl` 无关，注册后开关都正常） |
| `00303195 Unable to obtain the module information` / `module.json5` 解析失败 | HAR 的 `src/main/module.json5` 缺 `"module": { ... }` 顶层包装键 | 补包装键（`soui-ohos-lib` 已提供，自建 HAR 时注意） |
| 校验报模块名不合法 | 模块名不允许连字符（`^[a-zA-Z][0-9a-zA-Z_.]*$`） | 用 `soui_ohos_lib`，ArkTS 侧仍用包名 `@soui/ohos` |
| `deviceTypes` 校验失败 | 新版 SDK 用 `default` 而不是 `phone` | HAR 的 `deviceTypes` 用 `["default","tablet","2in1"]` |
| 运行崩溃 `Cannot read property init of undefined` | ArkTS import 的 `libXxx.so` 与 CMake 目标名（或 `nm_modname`）不一致，模块是 `undefined` | 按 1.3 对齐三处名字；看崩溃栈**列号**定位是哪个对象的哪个方法 |
| 首次 Sync 后多出 `oh-package-lock.json5` 变化 | 依赖解析结果刷新 | 正常现象，随改动提交即可 |
| `spawn java ENOENT`（HAP 打包阶段） | CLI 环境缺 JBR | PATH 加 `jbr/bin`（见 7.2） |
| HAP 里出现了旧的 `libXxx.so` | 构建中间产物残留（改名/删目标后旧文件仍在 `intermediates`） | 清掉 `entry/build/.../intermediates/{cmake,libs,stripped_native_libs}` 里的旧 so 后重出 HAP |
| 起窗口失败：`layout:xxx` 找不到 | `rawfile/uires/uires.idx` 里没登记该布局（新增 XML 后忘了手工补一行）、`name` 写错，或 `path` 与实际文件不符 | 打开该 `uires.idx` 核对 `<LAYOUT>` 中是否有对应 `name` 且 `path` 正确；补上后重新构建（`uiresbuilder` 不负责生成该文件） |
| 真机装不上 | 未签名 / 签名与设备不匹配 | DevEco 配自动签名后重新打包；模拟器可直接装未签名包 |

---

## 9. 和 Windows 端共用什么、不共用什么

| 可以直接复用 | 需要按平台处理 |
|---|---|
| `SWindow` / 控件树 / 布局 XML / 皮肤 / 样式 | 顶层窗口承载（`SouiScreen` / XComponent 面） |
| `SHostWnd` 子类、消息映射、事件订阅、模态 | 系统对话框 / 菜单 / 托盘等桌面专属能力 |
| `SetTimer` / `PostTask` / 异步任务 | 输入法（`NativeEditView`）、剪贴板、文件对话框 |
| 业务逻辑、算法、资源（同一套 `uires`） | 应用图标 / 启动图 / 权限声明等打包配置 |

---

## 10. 参考

- 示例工程：`demos/ohos-demo`、`games/cnchess/client/ohos`
- 适配库说明与接入细节：`soui-ohos-lib/README.md`
- 构建片段契约：`soui-ohos-lib/src/main/cpp/soui4_ohos.cmake`；入口接口：`soui-ohos-lib/src/main/cpp/include/soui4ohos.h`
- 适配方案背景：`doc/soui-harmonyos-porting-plan.md`、`doc/从Windows到鸿蒙-SOUI适配知乎文章.md`
- Android 端对应文档：`doc/SOUI5-Android开发入门指南.md`
