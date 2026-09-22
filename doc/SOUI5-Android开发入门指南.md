# SOUI5 Android 开发入门指南

> **适用版本**：SOUI 5.x（仓库目录名仍为 `soui4`；`ChangeLog` 中 5.3 已完成 Android / iOS / 鸿蒙适配）
> **目标读者**：准备用 SOUI5 开发 Android 应用的工程师（有 Windows 端 SOUI / WTL 经验者几乎零迁移成本）
> **配套示例**：`demos/android-demo`（最小可运行工程）、`games/cnchess/client/android`（完整业务：象棋 + 联网对战）
> **深入阅读**：`doc/SOUI_ANDROID_DEV_GUIDE.md`（内部实现与完整调用链）、`doc/SOUI-for-Android-适配方案技术文档.md`（适配方案）

---

## 1. 先理解 Android 端由什么组成

SOUI5 在 Android 上不是"另起一套 UI"，而是把 SOUI 依赖的 **Win32 窗口 / 消息 / GDI 模型**映射到 Android 的 **View + Canvas + Handler**。因此 Windows 端的布局 XML、皮肤、`EVENT_*` / 消息映射、`SetTimer` 等业务代码可以**源码级复用**。

| 组成 | 位置 | 交付形态 |
|---|---|---|
| Java 壳（平台桥） | `soui-android-lib/src/main/java/com/soui/android/` | **AAR**（Android library 模块） |
| native 适配层源码 | `soui-android-lib/src/main/cpp/` | **源码**（不预编译，见 1.1） |
| native 构建片段 | `soui-android-lib/src/main/cpp/soui4_android.cmake` | app 通过 `include()` 复用 |
| 平台入口接口 | `soui-android-lib/src/main/cpp/include/soui4android.h` | 业务层实现 `Soui4AndroidEntry` |
| SOUI 核心 | `SOUI/`、`utilities/`、`swinx/`、`components/`、`third-part/` | 由构建片段一并编译成 `.so` |

对应的核心概念映射（细节见 `doc/SOUI_ANDROID_DEV_GUIDE.md`）：

| Windows 概念 | Android 等价物 |
|---|---|
| `HWND` | `jlong nativeId`（自增 64 位整数，全链路同一个值，**不要当指针强转**） |
| `SHostWnd` | `SouiScreen`（View，承载 SOUI 主窗口） |
| `SendMessage / PostMessage` | swinx `WndMgr` + 内部消息队列 |
| `WM_LBUTTONDOWN / WM_KEYDOWN` | `SouiViewNative::onTouchEvent / onKeyEvent` 转成 Win32 消息 |
| `SetTimer / KillTimer` | `SouiPlatformBridge.setTimer / killTimer`（`Handler.postDelayed`） |
| `InvalidateRect` | `SouiPlatformBridge.invalidateRect → View.invalidate()` |
| `CreateWindowEx`（子窗口） | `SouiPlatformBridge.createWindow`（真实窗口 / 输入法编辑框等） |

### 1.1 关键约定：native 由业务 app 自己编译

`soui-android-lib` 的 `build.gradle` **不设置 `externalNativeBuild`**——AAR 里只有 Java 类，`.so` 由业务 app 的 CMake 统一编译。

原因是 ABI 安全：业务 `.so` 需要**直接链接 SOUI 的 C++ 符号**（`SHostWnd` / `SWindow` / `SApplication`…）。如果把 `libsoui4android.so` 预编译成产物分发，使用方必须严格同步 `config.h`、`WCHAR_SIZE`、`SOUI_*` 编译宏，任何一处不一致就是链接错误或运行期崩溃。所以约定为：**共享一个构建片段，各 app 各自编译**。鸿蒙端（`soui4_ohos.cmake`）采用同一策略。

### 1.2 `Soui4AndroidEntry`：平台层与业务层的唯一接口

业务 C++ 代码只需实现 4 个虚函数（`soui4android.h`）：

```cpp
struct Soui4AndroidEntry {
    virtual BOOL InitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir) = 0; // 创建 SApplication + 配置资源
    virtual void UninitApp() = 0;                                          // 释放 SApplication
    virtual HWND ScreenStartup(long screenId, LPCSTR pszLayout) = 0;       // 按 layout 创建主窗口
    virtual void ScreenShutdown(long screenId) = 0;                        // 关闭该 screenId 的窗口
};

void InitSoui4AndroidEntry(Soui4AndroidEntry* entry);  // 业务侧在静态构造中注册
```

`screenId` 让**多 Activity / 多屏**各自对应一个 SOUI 窗口系统（每个 Activity 用自己的常量 ID，全局唯一）。

---

## 2. 环境准备

| 项 | 要求 | 说明 |
|---|---|---|
| Android Studio | 支持 AGP 7.2.1+ | 示例工程 `demos/android-demo` 用 AGP 7.2.1 |
| Android SDK | compileSdk / targetSdk 34，minSdk 28 | 可按业务调整，示例为 28/34 |
| NDK | 与 AGP 匹配的 LTS（如 25.x / 26.x） | SDK Manager 安装；Gradle 通过 SDK 目录自动定位 |
| CMake | 3.18.1 及以上 | 由 `android.externalNativeBuild.cmake.version` 指定 |
| JDK | 11 / 17 | 与 AGP 版本匹配 |

拉取源码时注意子模块：

```bash
git submodule update --init --recursive   # swinx/、third-part/ 是子模块
```

---

## 3. 五分钟跑通现成示例

### 3.1 最小示例 `demos/android-demo`

```bash
cd demos/android-demo
./gradlew :app:assembleDebug            # Windows: gradlew.bat :app:assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

首次构建会从源码编译 SOUI 核心（soui4 / utilities4 / swinx / skia / 各组件），耗时较长；之后是增量编译。若只想要纯 UI 效果、不需要联网能力，可以在 `app/build.gradle` 的 cmake arguments 里加 `-DSOUI_BUILD_WS=OFF` 显著缩短配置与编译时间（示例工程已经这么做了）。

该工程演示了两种承载方式：

- `DefaultHostActivity`：代码方式创建 —— `new SouiScreen(this, SCREEN_ID, "layout:dlg_main")`
- `XmlHostActivity`：在 Android layout XML 里声明 `SouiScreen` 及其属性

### 3.2 完整业务示例 `games/cnchess/client/android`

包含共享业务源码（`games/cnchess/client/*.cpp`）、AI 算法、SOUI 资源与联网对战，是"真实项目长什么样"的最佳参照。

---

## 4. 从零创建自己的 Android 工程

### 4.1 目录结构

推荐让 app 工程与 SOUI 源码仓库**并列**，通过相对路径引用，避免把框架源码拷进业务仓库：

```text
<workspace>/
├── soui4/                                  # SOUI5 源码仓库（含 soui-android-lib/）
└── MyApp/                                  # 你的 Android 工程
    ├── settings.gradle
    ├── build.gradle
    └── app/
        ├── build.gradle
        └── src/main/
            ├── AndroidManifest.xml
            ├── assets/
            │   ├── uires/                  # 业务资源（布局/皮肤/字符串 + uires.idx）
            │   └── soui_sys_res/           # SOUI 系统资源（控件皮肤 + uires.idx）
            ├── cpp/
            │   ├── CMakeLists.txt
            │   ├── MyApp.cpp               # Soui4AndroidEntry 实现
            │   └── MainDlg.h / MainDlg.cpp # SHostWnd 子类（业务主窗口）
            ├── java/com/yourco/myapp/
            │   ├── MyApplication.java      # 初始化 SOUI
            │   └── MainActivity.java       # 承载 SouiScreen
            └── res/                        # 常规 Android 资源（图标、主题、layout）
```

### 4.2 `settings.gradle`：把 `soui-android-lib` 作为模块引入

```groovy
rootProject.name = "MyApp"

include ':app'
include ':soui-android-lib'
// 指向 SOUI 仓库里的适配库（按实际相对路径调整）
project(':soui-android-lib').projectDir = new File('../soui4/soui-android-lib')
```

### 4.3 顶层 `build.gradle`

```groovy
plugins {
    id 'com.android.application' version '7.2.1' apply false
    id 'com.android.library'     version '7.2.1' apply false
}
```

### 4.4 `app/build.gradle`：关键在 cmake arguments

```groovy
plugins { id 'com.android.application' }

android {
    namespace 'com.yourco.myapp'
    compileSdk 34

    defaultConfig {
        applicationId "com.yourco.myapp"
        minSdk 28
        targetSdk 34
        versionCode 1
        versionName "1.0"

        externalNativeBuild {
            cmake {
                cppFlags "-std=c++17 -fexceptions -frtti"
                arguments "-DANDROID_STL=c++_shared",
                        // SOUI 仓库根目录
                        "-DSOUI_ROOT_DIR=${rootDir.absolutePath.replace('\\', '/')}/../soui4",
                        // soui-android-lib 的 native 目录（构建片段所在处）
                        "-DSOUI_ANDROID_LIB_CPP_DIR=${project(':soui-android-lib').projectDir.absolutePath.replace('\\', '/')}/src/main/cpp",
                        // 不需要联网能力时关掉，可大幅缩短编译时间
                        "-DSOUI_BUILD_WS=OFF"
            }
        }
        ndk {
            // 只保留需要的 ABI，减少构建时间与包体积
            abiFilters 'arm64-v8a', 'armeabi-v7a'
        }
    }

    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
            version "3.18.1"
        }
    }

    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_8
        targetCompatibility JavaVersion.VERSION_1_8
    }

    buildTypes {
        release {
            minifyEnabled false
            signingConfig signingConfigs.debug   // 正式发布请换成自己的签名配置
        }
    }
}

dependencies {
    // 引用适配库（Java 壳 + native 头文件/源码）
    implementation project(':soui-android-lib')
}
```

> `replace('\\', '/')` 是必要的：Windows 路径里的反斜杠传给 CMake 会被当转义符。

### 4.5 `app/src/main/cpp/CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.18.1)

project("myapp")

# 变量来自 Gradle 的 externalNativeBuild.cmake.arguments
#   SOUI_ROOT_DIR            - SOUI 仓库根目录
#   SOUI_ANDROID_LIB_CPP_DIR - soui-android-lib/src/main/cpp（构建片段所在目录）
if(NOT EXISTS ${SOUI_ROOT_DIR})
    message(FATAL_ERROR "SOUI_ROOT_DIR: ${SOUI_ROOT_DIR} is not existed")
endif()

# 引入可复用的构建片段：内部会 add_subdirectory 编译 SOUI 核心，
# 并 add_library(soui4android SHARED) 编译 Android JNI 桥
include(${SOUI_ANDROID_LIB_CPP_DIR}/soui4_android.cmake)

# 自己的业务 native 代码
file(GLOB MYAPP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(myapp SHARED ${MYAPP_SRC})

target_include_directories(
        myapp
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SOUI_ANDROID_LIB_CPP_DIR}/include      # soui4android.h
        ${SOUI_ROOT_DIR}/SOUI/include
        ${SOUI_ROOT_DIR}/utilities/include
        ${SOUI_ROOT_DIR}/swinx/include
        ${SOUI_ROOT_DIR}/components
        ${PROJECT_BINARY_DIR}/config             # config.h 生成在这里
)

target_link_libraries(
        myapp
        PUBLIC
        soui4android                             # 内部已 PUBLIC 链接 soui4/utilities4/swinx
        android
        log
        jnigraphics
)
```

> **库名要与 `System.loadLibrary` 一致**：`add_library(myapp ...)` ⇒ 产出 `libmyapp.so` ⇒ Java 侧 `System.loadLibrary("myapp")`。
> `soui4android` 这个名字是约定的（JNI 静态注册符号前缀 `Java_com_soui_android_*` 固定在 soui-android-lib 里），**不要改名**。

### 4.6 Java 侧：初始化与页面承载

**Application —— 进程内只初始化一次：**

```java
package com.yourco.myapp;

import android.app.Application;
import com.soui.android.SouiPlatformBridge;

public class MyApplication extends Application {
    static {
        System.loadLibrary("myapp");   // 触发业务 so 加载（soui4android 由它依赖）
    }

    @Override
    public void onCreate() {
        super.onCreate();
        // 注册 AndroidPlatformAPI；assets 直读，无需把资源拷到 filesDir
        SouiPlatformBridge.getInstance().init(this, getAssets(), getFilesDir().getAbsolutePath());
    }

    @Override
    public void onTerminate() {
        SouiPlatformBridge.getInstance().destroy();
        super.onTerminate();
    }
}
```

**Activity —— 用 `SouiScreen` 铺满即可：**

```java
public class MainActivity extends AppCompatActivity {
    public static final long SCREEN_ID = 0x0000_D001L;      // 每个 Activity 一个，全局唯一
    public static final String LAYOUT_DEMO = "layout:dlg_main";  // 对应 uires.idx 里的 LAYOUT name

    private SouiScreen mScreen;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mScreen = new SouiScreen(this, SCREEN_ID, LAYOUT_DEMO);

        FrameLayout decor = new FrameLayout(this);
        decor.addView(mScreen, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        setContentView(decor);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (mScreen != null) {
            mScreen.requestLayout();
            mScreen.post(() -> {                     // 旋转后同步尺寸给 SOUI
                View root = (View) mScreen.getParent();
                mScreen.syncScreenSize(root.getWidth(), root.getHeight());
            });
        }
    }
}
```

`SouiScreen` 在 View 测量完成、`onAttachedToWindow` 时**自动**调用 `startSoui(screenId, layout, w, h)`，不需要手写 post 样板代码。

### 4.7 C++ 侧：入口实现 + 主窗口

**入口实现（`app/src/main/cpp/MyApp.cpp`）：**

```cpp
#include <souistd.h>
#include <SAppCfg.h>
#include <soui4android.h>
#include "MainDlg.h"

using namespace SNS;

class MyAppEntry : public Soui4AndroidEntry {
    SApplication* m_app = nullptr;
    std::map<long, SAutoRefPtr<SHostWnd>> m_screens;
public:
    MyAppEntry() { InitSoui4AndroidEntry(this); }     // 静态构造里注册自己

    BOOL InitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir) override {
        m_app = new SApplication((HINSTANCE) nullptr);
        SAppCfg cfg;
        cfg.SetRender(Render_Skia)
           .SetImgDecoder(ImgDecoder_Stb)
           .SetAppDir(S_CA2T(pszAssetDir, CP_UTF8))
           .SetSysResAndroidAsset(assetMgr, _T("soui_sys_res"))
           .SetAppResAndroidAsset(assetMgr, _T("uires"));
        return cfg.DoConfig(m_app);
    }

    void UninitApp() override { m_app->Release(); m_app = nullptr; }

    HWND ScreenStartup(long screenId, LPCSTR pszLayout) override {
        SAutoRefPtr<SHostWnd> host = new CMainDlg();      // 你的 SHostWnd 子类
        if (!host->Create(nullptr, 0, 0, 0, 0)) return 0;
        host->ShowWindow(SW_SHOW);
        m_screens[screenId] = host;
        return host->m_hWnd;
    }

    void ScreenShutdown(long screenId) override { m_screens.erase(screenId); }
};

static MyAppEntry theApp;    // 静态实例 ⇒ 进程启动即完成注册
```

**主窗口（`MainDlg.h/cpp`）：** 与 Windows 端写法**完全一致**——继承 `SHostWnd`，绑定布局，处理事件与定时器：

```cpp
class CMainDlg : public SHostWnd {
public:
    CMainDlg() : SHostWnd("layout:dlg_main") {}

    void OnInit(IEvtArgs* e);        // SEvent::EventInit
    void OnTimer(UINT_PTR idEvent);
    void OnBtnClick();               // 按钮命令

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_TIMER(OnTimer)
    SOUI_MSG_MAP_END()
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_ok", OnBtnClick)
    EVENT_MAP_END()
};
```

`SHostWnd` 的 `Create / ShowWindow / DestroyWindow`、`SetTimer`、模态、事件订阅等语义在 Android 上与 Windows 一致，业务代码可以直接搬。

---

## 5. SOUI 资源怎么放

- 资源放在 **`app/src/main/assets/`** 下，两个目录各一套：
  - `assets/soui_sys_res/` —— SOUI **系统资源**（控件默认皮肤，可从 `soui-sys-resource/` 拷贝）
  - `assets/uires/` —— **业务资源**（`xml/` 布局、`values/` 皮肤与字符串、`image/`…）
- **`uires.idx` 是资源索引清单，需要开发者手工维护**。它是 UTF-8 XML、根节点 `<resource>`、按资源类型分组，每项一条 `<file name="逻辑名" path="相对 idx 的文件路径"/>`：

  ```xml
  <?xml version="1.0" encoding="utf-8"?>
  <resource>
    <UIDEF>
      <file name="xml_init" path="uidef/init.xml"/>
    </UIDEF>
    <values>
      <file name="skin"   path="values/skin.xml"/>
      <file name="string" path="values/string.xml"/>
    </values>
    <LAYOUT>
      <file name="dlg_main" path="xml/dlg_main.xml"/>
    </LAYOUT>
  </resource>
  ```

  **新增、改名或删除资源文件后，必须手动同步修改 `uires.idx`** —— 移动端不会去扫描目录，漏改就会在运行期按逻辑名查不到资源。布局引用形式为 `"layout:dlg_main"`，其中 `dlg_main` 即上面 `<LAYOUT>` 里的 `name`。

  > 图片这类成批新增的条目，可以用 `tools/uiresImporter.exe`（Windows 控制台工具）辅助维护：它**读入已存在的** `uires.idx`，按 `-s "IMGX|GIFX"` 之类的子目录名重新扫描并重建这些类型节点、按 `-i <图片目录>` 追加图片，再回写 `uires.idx`（备份为 `uires.bak.idx`）并把图片补进 `values/skin.xml`。注意它**不新建** `uires.idx`、也不处理 `<LAYOUT>` / `<values>` 等需手写的类型。
- **`tools/uiresbuilder` 不生成 `uires.idx`，也不参与移动端构建**。它把 `uires.idx` 当**输入**（`-i`），产出 `.rc2`（`-r`）、`resource.h`（`-h`）和可选的 `R.js`（`-j`）——把资源以 `RCDATA` 打进可执行文件、生成 C++/JS 侧的资源常量表。**三个桌面端（Windows / Linux / macOS）都用它**：Windows 由 MSVC `rc.exe` 直接把 `.rc2` 编进 PE；Linux / macOS 则用 MinGW `windres` 把同一份 `.rc2` 编译成 COFF，再由 `__cmake/windres.cmake` 的 `target_compile_resources()` 转成 ELF / Mach-O 链进可执行文件，运行期统一由 swinx 的 Win32 资源 API 读取。**只有 Android / 鸿蒙不产生也不使用这三份产物**，布局直接按字符串名加载。
- Android 端由 `SResProviderAndroidAsset` 在**运行期解析 `assets/uires/uires.idx`**（经 `AAssetManager` 直读 APK 内 assets），据此建立"类型 + 逻辑名 → 物理路径"映射，所以 `uires.idx` 必须随 assets 一起打进 APK。**不需要**像早期方案那样把资源解压到 `filesDir`；资源更新后重新 `assembleDebug` 即可生效。

---

## 6. 构建、安装、调试

```bash
# 构建
./gradlew :app:assembleDebug                     # 产物 app/build/outputs/apk/debug/app-debug.apk
# 安装并启动
./gradlew :app:installDebug
adb shell am start -n com.yourco.myapp/.MainActivity
# 日志（SOUI 的日志走 logcat）
adb logcat -s soui-demo-jni:V host:V AndroidRuntime:E
```

常用技巧：

- **只编译 native**：Android Studio 里单独构建 `app` 模块的 `externalNativeBuildDebug`，比整包快很多。
- **中断在 native**：Android Studio 可 attach native debugger 调试 `libmyapp.so`（Debug 构建带符号）。
- **改 CMake 后必须重新 Sync**：Gradle 会重新跑 CMake configure；新增源文件靠 `file(GLOB)`，需触发一次重配（改任意 CMake 文件或 Clean 都行）。

---

## 7. 常见问题

| 现象 | 原因 | 处理 |
|---|---|---|
| `java.lang.UnsatisfiedLinkError: dlopen failed: library "libmyapp.so" not found` | `System.loadLibrary` 名字与 `add_library` 目标名不一致，或 ABI 不含当前设备 | 统一库名；检查 `abiFilters` 是否覆盖设备 ABI（模拟器常为 `x86_64`） |
| 界面纯色/空白，日志无波形 | `Soui4AndroidEntry` 未注册（静态实例没建起来 / 该 TU 没参与编译） | 确认 `theApp` 静态实例存在且 `InitSoui4AndroidEntry(this)` 被调用 |
| 起窗口失败：`layout:xxx` 找不到 | `uires.idx` 里没登记该布局（新增 XML 后忘了手工补一行）、`name` 写错、`path` 与实际文件不符，或 `uires.idx`/XML 没随 assets 打进 APK | 打开 `assets/uires/uires.idx` 核对 `<LAYOUT>` 中是否有对应 `name` 且 `path` 正确；补上后重新 `assembleDebug`（`uiresbuilder` 不负责生成该文件） |
| 启动即崩、无 Java 栈 | native 侧断言/未捕获异常 | `adb logcat` 看 `DEBUG`/`libc`/`SIGSEGV` 行；先确保 `SOUI_ROOT_DIR`、`SOUI_ANDROID_LIB_CPP_DIR` 指向正确 |
| 旋转屏幕后花屏或尺寸不对 | 未把新尺寸同步给 SOUI | 参考 4.6 的 `onConfigurationChanged`：`requestLayout()` + `syncScreenSize(w, h)` |
| 构建奇慢 | 默认打开了 WS(OpenSSL/libcurl)、富文本等重量级模块；ABI 过多 | `-DSOUI_BUILD_WS=OFF`（不需要联网时）、`-DSOUI_BUILD_RICHEDIT=OFF`；`abiFilters` 只留目标 ABI |
| 改了 `soui-android-lib` 里的源码但 app 没变化 | 该模块的 native 是 app 编译的，Java 改动也需重新构建 | 重新 `assembleDebug`（必要时 Clean） |

---

## 8. 和 Windows 端共用什么、不共用什么

| 可以直接复用 | 需要按平台处理 |
|---|---|
| `SWindow` / 控件树 / 布局 XML / 皮肤 / 样式 | 顶层窗口创建方式（`SouiScreen` vs `CreateWindowEx`） |
| `SHostWnd` 子类、消息映射、事件订阅、模态 | 系统对话框 / 菜单 / 托盘等桌面专属能力 |
| `SetTimer` / `PostTask` / 异步任务（`games/cnchess` 有范例） | 真实窗口控件（`SRealWnd`，由 `IRealWndHandler` 桥接） |
| 业务逻辑、算法、资源（同一套 `uires`） | 输入法、剪贴板、文件对话框等平台 API |

---

## 9. 参考

- 示例工程：`demos/android-demo`、`games/cnchess/client/android`
- 适配库说明：`soui-android-lib/src/main/cpp/soui4_android.cmake`（片段契约）、`soui4android.h`（入口接口）
- 深入文档：`doc/SOUI_ANDROID_DEV_GUIDE.md`、`doc/SOUI-for-Android-适配方案技术文档.md`、`doc/SOUI_ANDROID_VS_FLUTTER_QT.md`
- 鸿蒙端对应文档：`doc/SOUI5-鸿蒙开发入门指南.md`
