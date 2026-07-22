# SOUI4 Android 端开发指南（基于 soui-android-lib）

> 适用版本：soui4 仓库当前 HEAD（含 `d:\work\soui4\soui-android-lib\` 适配库与 `swinx` 跨平台窗口抽象层）
> 目标读者：有 SOUI（Windows DirectUI）开发经验，需要把 SOUI 业务迁移/发布到 Android 的工程师

---

## 1. 前言 & 设计目标

`soui-android-lib` 是 SOUI4 的 Android 官方适配层，它把 SOUI 原本依赖的 Windows 消息机制 + GDI/HWND 抽象成 Android 的 **View + Canvas + Handler**，使得 SHostWnd / SWindow 树、布局 XML、skin、EVT_* 事件、SetTimer 等 Windows 端的 API 和业务代码，在 Android 上 **源码级复用**。

核心设计原则：

| 概念（Windows） | Android 等价物 | 在哪里实现 |
|---|---|---|
| HWND（窗口句柄） | `jlong nativeId`（自增 64 位整数） | `SouiViewNative::initialize()` 分配 |
| SHostWnd（宿主窗口） | `SouiView`（绘制 + 输入） + `SouiViewGroup`（容器） | `soui-android-lib/src/main/java/com/soui/` |
| `::SendMessage / ::PostMessage` | swinx 抽象层 `SendMessageW/PostMessageW` + `WndMgr::fromHwnd` + 内部消息队列 | `swinx/src/winuser.cpp`、`swinx/src/wndobj.cpp` |
| `WM_LBUTTONDOWN / WM_KEYDOWN / WM_CHAR` | `SouiViewNative::onTouchEvent / onKeyEvent` 把 Android MotionEvent/KeyEvent 转成 Win32 消息 | `soui-android-lib/src/main/cpp/SouiViewNative.cpp` |
| `SetTimer / KillTimer` | `SouiPlatformBridge.setTimer/killTimer`（基于 `Handler.postDelayed`） | `SouiPlatformBridge.java` + `AndroidPlatformAPI.cpp` |
| `InvalidateRect` | `SouiPlatformBridge.invalidateRect → View.invalidate()` | `SouiPlatformBridge.java` |
| `::CreateWindowEx`（子窗口） | `SouiPlatformBridge.createWindow`（在 SouiViewGroup 里 addView 绝对定位 View） | `SouiPlatformBridge.java` |

**Handle 单一来源**：`nativeId`（jlong，自增起点 100）= `SHostWnd.Attach(m_nativeId)` 的 HWND 值 = `SouiPlatformBridge.registerView(nativeId, SouiView)` 的 key = `native-lib.cpp` 里 `s_nativeViewMap<jlong, shared_ptr<SouiViewNative>>` 的 key。**全链路都是同一个整数，永远不要把 nativeId 当指针强转**。

---

## 2. 架构总览（调用链全景）

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              Android Application                            │
│                                                                             │
│   Application.onCreate()                                                    │
│     └─ SouiPlatformBridge.getInstance().init(ctx)  ←── (1) 全局平台桥初始化 │
│                                                                             │
│   Activity.setContentView(R.layout.xxx)                                     │
│     ├─ <com.soui.SouiViewGroup> (或代码 new SouiViewGroup)                  │
│     │    └─ 内部自动 new SouiView（= SHostWnd 表面）并填满容器              │
│     │                                                                       │
│     └─ souiViewGroup.initSouiLayout("dlg_main")  ←── (2) 绑定布局+创建原生│
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│ JNI Boundary                                                                 │
│  nativeCreate("dlg_main")  →  (3) 查 ISHostWndFactory → 构造 CMainDlg 子类  │
│  nativeDestroy(nativeId)   →  (7) map erase → shared_ptr 析构 → SHostWnd    │
│  nativeOnTouchEvent / nativeOnKeyEvent / nativeOnSizeChanged / nativeRender │
├─────────────────────────────────────────────────────────────────────────────┤
│ C++ Native Layer (soui-android-lib/src/main/cpp)                            │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │  native-lib.cpp                                                    │    │
│  │   s_nativeViewMutex + unordered_map<nativeId, shared_ptr<SouiViewNative>>│
│  │   (4) 所有 native* 方法都先做查表，不把 nativeId 当指针转          │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │  SouiViewNative (每个 SHostWnd 1:1 实例)                            │    │
│  │   ├─ initialize(): 分配 nativeId(=HWND) → hostWnd->Attach(nativeId) │    │
│  │   ├─ onTouchEvent() → 生成 WM_LBUTTONDOWN/UP/MOVE + SetCapture/RC   │    │
│  │   ├─ onKeyEvent()   → 生成 WM_KEYDOWN/UP + WM_CHAR                  │    │
│  │   └─ render()       → Android Bitmap 包一层 IRenderTarget → 让 SOUI │    │
│  │                      把内容画到 ARGB_8888 Bitmap                     │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │  AndroidPlatformAPI  (swinx 的 PlatformAPI 在 Android 上的实现)      │    │
│  │   └─ setTimer / createWindow / moveWindow / InvalidateRect     │    │
│  │        全部反向回调到 Java 层 SouiPlatformBridge.* 方法              │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
├─────────────────────────────────────────────────────────────────────────────┤
│ SWINX (跨平台 Windows API 抽象层, swinx/)                                   │
│  ::SendMessage((HWND)nativeId, WM_LBUTTONDOWN, wParam, lParam)              │
│    └─ WndMgr::fromHwnd(nativeId) → 找到 WndObj → WindowProc thunk          │
│         └─ SHostWnd::ProcessWindowMessage → 消息映射(MESSAGE_RANGE_HANDLER) │
│             └─ DoFrameEvent → SWindow 树分发事件 → EVT_* 业务回调 (5)       │
├─────────────────────────────────────────────────────────────────────────────┤
│ SOUI Framework (SOUI/)                                                      │
│  CMainDlg (你的自定义 SHostWnd 子类)                                        │
│    ├─ OnInitDialog: FindChildByName / SetWindowText / SetTimer              │
│    ├─ EVT_CMD(IDOK, OnOK): 读 SEdit 文本、网络请求、弹提示等业务 (6)        │
│    └─ 所有 Windows/SOUI 原生写法直接用                                      │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. 环境要求与工程集成

### 3.1 环境要求

| 组件 | 最低版本 | 备注 |
|---|---|---|
| Android Studio | Hedgehog (2023.1) 及以上 | 带 CMake/Native 调试 |
| Android Gradle Plugin (AGP) | 8.x | 与 Gradle 8.x 匹配 |
| Gradle | 8.0+ | |
| NDK | 25.2.9519653（库中写死，见 build.gradle） | 可改为本机已装版本，只要 r25+ |
| CMake | 3.18.1+ | C++11 必选，推荐 C++17 |
| compileSdk / targetSdk | 34 | |
| minSdk | 21 | Android 5.0（5.0 以下 Bitmap 锁像素 API 受限，不支持）|
| SOUI 源码 | 仓库根目录 `D:\work\soui4\` | 含 SOUI/、swinx/、utilities/、components/、third-part/ |

### 3.2 目录结构建议

```
your-android-project/
├─ settings.gradle              // include ':soui-android-lib' 并指定 projectDir
├─ build.gradle (project 级)   // AGP 8.x
├─ app/
│   ├─ build.gradle            // implementation project(':soui-android-lib')
│   └─ src/main/
│       ├─ AndroidManifest.xml
│       ├─ java/com/your/company/app/
│       │   ├─ App.kt (或 App.java)   // Application 类
│       │   └─ MainActivity.kt
│       ├─ res/
│       │   └─ layout/
│       │       └─ activity_main.xml
│       └─ assets/             // ⚠️ 推荐：SOUI 资源放 assets 下（见第 8 章）
│           └─ soui/
│               ├─ uires.idx
│               ├─ layout/
│               │   ├─ dlg_main.xml
│               │   └─ settings.xml
│               ├─ skin/
│               └─ sys/        // soui-sys-resource 相关皮肤
└─ soui-android-lib/  ← 把 d:\work\soui4\soui-android-lib\ 原样拷贝或软链到此
    ├─ build.gradle
    └─ src/main/cpp/
        └─ CMakeLists.txt      // 内部已经引用 D:\work\soui4 下的 SOUI\swinx\...
```

### 3.3 settings.gradle 配置

```groovy
pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "MySouiApp"
include ':app'
include ':soui-android-lib'
// 如果你把 soui-android-lib 放在和 settings.gradle 同级，下面一行可以省略；
// 否则按实际路径指定：
// project(':soui-android-lib').projectDir = new File(rootDir, '../soui4/soui-android-lib')
```

### 3.4 app/build.gradle 配置

```groovy
plugins {
    id 'com.android.application'
    id 'kotlin-android'  // 可选，Java 也行
}

android {
    namespace "com.yourcompany.app"
    compileSdk 34

    defaultConfig {
        applicationId "com.yourcompany.app"
        minSdk 21
        targetSdk 34
        versionCode 1
        versionName "1.0"

        // SouiView native 层用 c++_shared（和 soui-android-lib 保持一致）
        externalNativeBuild {
            cmake {
                cppFlags "-std=c++17 -fexceptions -frtti"
                arguments "-DANDROID_STL=c++_shared"
                abiFilters "arm64-v8a", "armeabi-v7a", "x86_64"
            }
        }
        ndk {
            abiFilters "arm64-v8a", "armeabi-v7a", "x86_64"
        }
    }

    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }

    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_8
        targetCompatibility JavaVersion.VERSION_1_8
    }

    // 只有 app 需要打包 libsoui-android.so，库本身已经声明了 externalNativeBuild
    // 这里留空即可，AGP 会把 AAR 里的 .so 合并进去
}

dependencies {
    implementation 'androidx.core:core-ktx:1.13.1'
    implementation 'androidx.appcompat:appcompat:1.6.1'
    implementation 'com.google.android.material:material:1.11.0'
    implementation project(':soui-android-lib')   // ⭐ 核心依赖
}
```

### 3.5 若 soui4 不在 `D:\work\soui4`

`soui-android-lib/src/main/cpp/CMakeLists.txt` 第 26 行写死了：
```cmake
set(SOUI_ROOT_DIR "D:/work/soui4")
```
把它改成实际路径（或在 app 层覆盖这个变量，加一个 gradle task 注入）。推荐用环境变量：
```cmake
set(SOUI_ROOT_DIR $ENV{SOUI4_ROOT} "D:/work/soui4")
```

---

## 4. App 初始化（启动时只做一次）

### 4.1 Java 侧 Application

```kotlin
class App : Application() {
    override fun onCreate() {
        super.onCreate()
        // 1) 初始化全局 SOUI 平台桥（注册 JNI PlatformAPI、创建 Handler、初始化 HWND-View map）
        SouiPlatformBridge.getInstance().init(applicationContext)

        // 2) 如果你有自定义 SHostWnd（见第 7 章），这里要先注册 native 侧工厂：
        NativeAppFactory.registerAppFactory()  // 这个 JNI 你自己写，第 7.2 节有代码
    }

    override fun onTerminate() {
        super.onTerminate()
        // 退出时销毁所有注册关系 / 未销毁的定时器引用
        SouiPlatformBridge.getInstance().destroy()
    }
}
```

AndroidManifest.xml 里把 Application 指向它：
```xml
<manifest ...>
    <application
        android:name=".App"
        ...>
```

### 4.2 初始化顺序（非常重要）

```
Application.onCreate 必做 1、2
    │
    ├─ 1. SouiPlatformBridge.init()      // 必须第一个：AndroidPlatformAPI::init → RegisterAndroidPlatformAPI()
    │                                         否则 SWINX 里的 ::SetTimer/CreateWindowEx 会找不到 Java 方法
    │
    └─ 2. 注册 ISHostWndFactory（如果有自定义 SHostWnd）
                                              // 必须在任何 initSouiLayout 之前。
                                              // 因为 nativeCreate 里 new SouiViewNative 会立即调 getHostWndFactory()。
之后任何 Activity/Fragment 才能 souiViewGroup.initSouiLayout(...)
```

> ❌ 错误顺序：先 initSouiLayout → 再注册 factory → 得到的都是默认 `SHostWnd`，你的 CMainDlg 不会被构造。

---

## 5. 两种使用模式

| 模式 | 适用场景 | 需要写 C++ 吗？ |
|---|---|---|
| **A. 默认 SHostWnd + 布局 XML** | 快速验证布局、Demo、静态展示页 | ❌ 不用（纯 Java） |
| **B. 自定义 SHostWnd 子类（推荐正式项目用）** | 有按钮点击、EVT_* 回调、业务逻辑、定时器、适配器等 | ✅ C++ 写 SHostWnd 子类 + 注册 factory |

---

## 6. 模式 A：默认 SHostWnd + 布局 XML（5 分钟跑通 Hello World）

### 6.1 Activity 布局 XML（app/src/main/res/layout/activity_main.xml）

```xml
<androidx.constraintlayout.widget.ConstraintLayout
    xmlns:android="http://schemas.android.com/apk/res/android"
    xmlns:app="http://schemas.android.com/apk/res-auto"
    android:layout_width="match_parent"
    android:layout_height="match_parent">

    <com.soui.SouiViewGroup
        android:id="@+id/soui_container"
        android:layout_width="match_parent"
        android:layout_height="match_parent" />

</androidx.constraintlayout.widget.ConstraintLayout>
```

### 6.2 Activity 代码

```kotlin
class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val container = findViewById<SouiViewGroup>(R.id.soui_container)
        // "xml:demo_layout" 表示让 SOUI 资源管理器在 xml 包下寻找 demo_layout.xml 这一份 SOUI 布局
        // 如果 assets/soui/layout/demo_layout.xml，对应资源 id 写成 "layout:demo_layout"，具体看你 uires.idx 的命名
        container.initSouiLayout("layout:demo_layout")
    }
}
```

### 6.3 SOUI 布局（app/src/main/assets/soui/layout/demo_layout.xml）

```xml
<?xml version="1.0" encoding="utf-8"?>
<SOUI trCtx="demo" minSize="320,480">
    <root layout="LinearLayout" orient="vertical" gravity="center" margin="16dp,40dp,16dp,40dp">
        <text text="SOUI on Android 🔥"
              textSize="28" textColor="#1677FF"
              layout_width="match_parent" layout_height="wrap_content"
              gravity="center" marginBottom="24dp" />

        <img skin="_icon.info" layout_width="64dp" layout_height="64dp"
             layout_gravity="center" marginBottom="24dp"/>

        <button id="IDOK" name="btn_click"
                text="点我" textSize="18"
                skin="_skin.skin_btn_default"
                layout_width="match_parent" layout_height="48dp" />
    </root>
</SOUI>
```

编译运行你就能看到 SOUI 画出来的界面。但是按钮点击没有反应——因为默认 SHostWnd 不绑定任何业务。这时候进入模式 B。

---

## 7. 模式 B：自定义 SHostWnd 子类 + 业务逻辑（正式项目用）

目标：
- 点「点我」按钮时，读取输入框文字、Toast 到 Android 层
- 启动定时器每秒刷新一个时钟 text

### 7.1 写一个自定义 CMainDlg（C++ 业务侧，一般在 app 自己的 native 源码里）

先在 `app/src/main/cpp/` 下建你自己的源文件，然后 app 加 `externalNativeBuild` 来编译一个 `libmyapp.so`（也可以把代码 fork 进 soui-android-lib 里一起打包——两种都可，下面演示把自定义代码和 `soui-android-lib` 一起编译，最省事）。

如果你想完全不改 soui-android-lib，就把下面两个文件放到你 app 自己的 cpp 目录 + 自己的 CMakeLists.txt（最后再 `target_link_libraries(myapp soui-android)`），思路是一样的。

#### `app/src/main/cpp/MainDlg.h`

```cpp
#pragma once
#include <souistd.h>
#include <core/shostwnd.h>
#include <control/SEDit.h>
#include <helper/stimer.h>

class CMainDlg : public SNS::SHostWnd {
public:
    CMainDlg() : SHostWnd(_T("layout:dlg_main")), m_timerId(1001) {}
    virtual ~CMainDlg() override = default;

protected:
    // SHostWnd 的消息映射入口（SOUI Windows 写法原样搬）
    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

    BOOL OnInitDialog(HWND hWnd, LPARAM lParam);
    void OnTimer(UINT_PTR idEvent);

    // 按钮点击（SOUI EVT_CMD 绑定）
    void OnOK();
    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND(SNS::IDOK, OnOK)
    EVENT_MAP_END()

private:
    SNS::SEdit *m_editInput = nullptr;  // 不拥有，只是缓存（FindChildByName）
    SNS::SWindow *m_txtClock = nullptr;
    UINT_PTR m_timerId;
};
```

#### `app/src/main/cpp/MainDlg.cpp`

```cpp
#include "MainDlg.h"
#include <helper/SStringW.hpp>
#include <jni.h>
#include <string>

// 回调到 Java 层弹 Toast（可选，演示跨语言调用）
static void postAndroidToast(const SNS::SStringW &text) {
    extern JavaVM *g_javaVM;                 // 从 JNI_OnLoad 里存下来
    if (!g_javaVM) return;
    JNIEnv *env = nullptr;
    if (g_javaVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) return;

    jclass cls = env->FindClass("android/widget/Toast");
    jmethodID mId = env->GetStaticMethodID(cls, "makeText",
        "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
    // 1) 拿 Application Context：这部分需要先在 JNI_OnLoad 缓存 context 全局引用，
    //    或直接通过 Activity View 的 context。为简洁省略具体实现，读者可自行补。
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam) {
    SHostWnd::OnInitDialog(hWnd, lParam);  // 必须先调父类

    // 用 SOUI 原生 FindChildByName/FindChild 取控件指针
    m_editInput = FindChildByName2<SNS::SEdit>(L"edit_input");
    m_txtClock  = FindChildByName2<SNS::SWindow>(L"txt_clock");

    if (m_editInput) {
        m_editInput->SetWindowTextW(L"请输入...");
    }

    // 开定时器：走 SWINX 的 ::SetTimer → AndroidPlatformAPI → SouiPlatformBridge.setTimer → Handler.postDelayed
    // 所以不需要自己写 Android Handler
    if (m_txtClock) {
        SNS::SetTimer(m_hWnd, m_timerId, 1000, nullptr);  // 1s
    }

    return TRUE;
}

void CMainDlg::OnTimer(UINT_PTR idEvent) {
    if (idEvent != m_timerId || !m_txtClock) return;

    // SOUI 自带 STimerHelper 也可用，这里演示底层 MSG_WM_TIMER
    SYSTEMTIME st{};
    SNS::GetLocalTime(&st);
    SNS::SStringW buf;
    buf.Format(L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    m_txtClock->SetWindowTextW(buf);
}

void CMainDlg::OnOK() {
    if (!m_editInput) return;
    SNS::SStringW txt = m_editInput->GetWindowText();

    // 业务示例：把文本写回另一个控件 + 弹 Toast
    SNS::SWindow *pTip = FindChildByName2<SNS::SWindow>(L"txt_tip");
    if (pTip) {
        SNS::SStringW msg = L"你输入的是：";
        msg += txt;
        pTip->SetWindowTextW(msg);
    }
    postAndroidToast(txt);
}
```

### 7.2 实现并注册 ISHostWndFactory

`soui-android-lib` 已经在 `SouiViewNative.h:36-49` 定义了工厂接口：
```cpp
class ISHostWndFactory {
public:
    virtual ~ISHostWndFactory() = default;
    virtual SNS::SHostWnd* createHostWnd(const std::string& layoutResName) = 0;
};
```

我们要做两件事：
1. 写一个工厂实现，根据 Java 传进来的 `layoutResName`（如 `"layout:dlg_main"`）构造 `CMainDlg`，其它返回 `nullptr`（fallback 默认 SHostWnd）。
2. 写一个 `extern "C"` JNI 入口，在 App.onCreate 里调它 → `SouiViewNative::setHostWndFactory(&g_myFactory)`。

#### `app/src/main/cpp/NativeAppFactory.cpp`

```cpp
#include "MainDlg.h"
#include "SouiViewNative.h"   // ⭐ soui-android-lib 里的头文件

class CAppHostWndFactory : public SouiViewNative::ISHostWndFactory {
public:
    SNS::SHostWnd* createHostWnd(const std::string& layoutResName) override {
        // 把 Java 传的 ANSI/UTF-8 名字和你的业务类映射起来
        if (layoutResName == "layout:dlg_main" || layoutResName == "dlg_main") {
            return new CMainDlg();
        }
        // TODO: 其它页面，如 "layout:dlg_settings" → new CSettingsDlg();
        return nullptr;  // 返回空：库会 fallback new SHostWnd(layoutResName.c_str())
    }
};

static CAppHostWndFactory g_myFactory;   // 全局静态，进程级单例

JavaVM *g_javaVM = nullptr;              // 给 postAndroidToast 用

extern "C" JNIEXPORT void JNICALL
Java_com_yourcompany_app_NativeAppFactory_registerAppFactory(JNIEnv *env, jclass) {
    // 存 JavaVM 指针（跨线程拿 JNIEnv 用）
    env->GetJavaVM(&g_javaVM);
    // 注册工厂到 SouiViewNative
    SouiViewNative::setHostWndFactory(&g_myFactory);
}
```

#### Java 侧（app/src/main/java/com/yourcompany/app/NativeAppFactory.java）

```java
package com.yourcompany.app;

public class NativeAppFactory {
    static {
        // ⚠️ 这一步：如果你的业务代码单独打成 libmyapp.so，就写 "myapp"
        //     如果全部打包进 soui-android（直接放在 soui-android-lib/cpp 下一起编译），
        //     可以不写这句，因为 SouiPlatformBridge 已经 System.loadLibrary("soui-android")。
        //     但为了稳妥，加上不会有任何副作用。
        System.loadLibrary("soui-android");
    }

    public static native void registerAppFactory();
}
```

然后在 `App.onCreate` 第 2 步里调用它：
```kotlin
NativeAppFactory.registerAppFactory()
```

### 7.3 对应的 SOUI 布局 `app/src/main/assets/soui/layout/dlg_main.xml`

```xml
<?xml version="1.0" encoding="utf-8"?>
<SOUI trCtx="app_main" minSize="360,640">
    <root layout="LinearLayout" orient="vertical" gravity="top" padding="16dp,24dp,16dp,16dp">
        <!-- 时钟（1s 刷新） -->
        <text name="txt_clock" text="00:00:00" textSize="32" textColor="#FF3B30"
              gravity="center" layout_width="match_parent" layout_height="wrap_content"
              marginBottom="24dp" />

        <!-- 输入框 -->
        <edit name="edit_input" textSize="16" textColor="#222"
              cueText="输入点什么吧..." cueTextColor="#AAAAAA"
              skin="_skin.skin_edit"
              layout_width="match_parent" layout_height="44dp"
              marginBottom="16dp" />

        <!-- 按钮，id=IDOK 对应 EVENT_ID_COMMAND(SNS::IDOK, OnOK) -->
        <button id="IDOK" name="btn_ok"
                text="提交" textSize="18"
                skin="_skin.skin_btn_default"
                layout_width="match_parent" layout_height="48dp"
                marginBottom="24dp" />

        <!-- 结果提示 -->
        <text name="txt_tip" text="等你提交..." textSize="16" textColor="#555"
              gravity="left|top"
              layout_width="match_parent" layout_height="match_parent" />
    </root>
</SOUI>
```

### 7.4 Activity 里使用（和模式 A 完全一样！）

```kotlin
class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val container = findViewById<SouiViewGroup>(R.id.soui_container)
        // 传 dlg_main → 工厂命中 → 构造 CMainDlg 而非默认 SHostWnd
        container.initSouiLayout("layout:dlg_main")
    }
}
```

这样业务就打通了：
1. Activity 启动 → initSouiLayout
2. nativeCreate → 查表 factory.createHostWnd("layout:dlg_main") 返回 CMainDlg
3. `CMainDlg` 构造时它自己 `SetLayoutId("layout:dlg_main")`，SHostWnd 去加载这份布局 XML
4. SHostWnd 内部发送 WM_INITDIALOG → `CMainDlg::OnInitDialog` → 缓存控件指针 + SetTimer
5. 用户点 OK 按钮 → SOUI EVENT → `CMainDlg::OnOK` → 读 Edit → 写 tip
6. 每秒 WM_TIMER → 刷新时钟 text

> ✨ **最重要**：CMainDlg 这一份 C++ 代码，和 Windows 端项目里你写的 CMainDlg 是 99% 相同的（除了 postAndroidToast 这种平台特有的部分）。这就是 SOUI4 跨平台的价值。

---

## 8. SOUI 资源（Layout/Skin/Style）在 Android 上怎么放

SOUI 在 Windows 端通常把资源打包为 exe 内嵌的 `.res` 资源段，或使用 `IResProviderZip` 读 `uires.zip`。Android 上推荐两种：

### 方案 1（推荐，最简单）：assets + ZipProvider

把所有 SOUI 资源打成 **uires.zip**（里面根目录直接是 `layout/ skin/ sys/ ...` 不含外层目录），放到 `app/src/main/assets/soui/uires.zip`。

然后在 **JNI_OnLoad / registerAppFactory** 里注册一次 zip provider（示例）：

```cpp
#include <resprovider/SResProviderZip.h>   // SOUI 自带的 zip 资源 provider

static void initSouiRes(JNIEnv *env, jobject context) {
    using namespace SNS;
    // 1) 拿到 app 的 nativeLibraryDir 或 assets 路径
    //    方式 A：直接用 libzip / AAssetManager 解压；SOUI4 其实有 AAsset 版 IResProvider
    //    方式 B：首次启动把 uires.zip 从 assets 拷贝到 /sdcard/Android/data/xxx/cache/
    // 具体 AAsset-based IResProvider 实现可参考 swinx/src/ 下资源 provider 扩展，
    // 这里重点是 layout 名称要和 initSouiLayout() 传的字符串一致。
}
```

### 方案 2（快速 demo）：直接写磁盘（调试用）

把整个 `uires/` 目录 push 到 `/sdcard/Android/data/<pkg>/files/soui/`，然后用 `IResProviderFilesystem` 加载。不适合线上。

### 关于 `xml:` / `layout:` 前缀

这是 SOUI 资源 provider 的 **包名/类型名**，跟 `uires.idx` 里声明的 `type` 完全对应。如果你不知道传什么给 `initSouiLayout()`，在 Windows 端打开对应的 SOUI 工程看 `SHostWnd::Create(_T("xxxx"))` 里的 `xxxx` 是什么字符串，这里就传什么。

---

## 9. 事件 & 输入完整链路（定位 bug 的速查表）

### 9.1 触摸事件

```
用户手指按屏幕
  → ViewRootImpl 派发
    → SouiView.onTouchEvent(MotionEvent)        [SouiView.java:134]
      → action 转成 0..6 整数 + 坐标/指针ID/时间戳
        → nativeOnTouchEvent(nativeId, a, x, y, pid, ts) [JNI]
          → native-lib.cpp NativeViewMap_Lookup(nativeId)  // 第 5 章修复，不再把 ID 当指针
            → SouiViewNative::onTouchEvent                  [SouiViewNative.cpp:104]
              ├─ action 映射
              │    ACTION_DOWN     (0) → WM_LBUTTONDOWN  wParam=MK_LBUTTON + SetCapture(hHost)
              │    ACTION_UP       (1) → WM_LBUTTONUP    ReleaseCapture
              │    ACTION_MOVE     (2) → WM_MOUSEMOVE    wParam=MK_LBUTTON
              │    ACTION_CANCEL   (3) → WM_MOUSELEAVE   ReleaseCapture
              │    ACTION_POINTER_DOWN(5) → WM_RBUTTONDOWN （第二根手指，模拟右键）
              │    ACTION_POINTER_UP  (6) → WM_RBUTTONUP
              │
              ├─ LPARAM lParam = MAKELPARAM(ix, iy)
              │
              └─ ::SendMessage((HWND)m_nativeId, msg, wParam, lParam)   [swinx SendMessageW]
                    │
                    └─ WndMgr::fromHwnd(m_nativeId) → WndObj::WindowProc
                         └─ SHostWnd::ProcessWindowMessage
                              MSG_WM_LBUTTONDOWN → SHostWnd::OnMouseEvent → SHostWnd::DoFrameEvent(WM_LBUTTONDOWN,...)
                                   └─ 命中 SButton (HTCLIENT) → OnLButtonDown → 发送 EVENT_CMD → CMainDlg::OnOK
```

### 9.2 键盘 / 编辑框输入

```
软键盘按键 → InputConnection.commitText / sendKeyEvent
  → SouiView.onKeyDown(int keyCode, KeyEvent event)          [SouiView.java:176]
    → nativeOnKeyEvent(nativeId, keyCode, 0=ACTION_DOWN, metaState, time)
      → NativeViewMap_Lookup
        → SouiViewNative::onKeyEvent                              [SouiViewNative.cpp:173]
          ├─ ACTION_DOWN → WM_KEYDOWN
          │  ACTION_UP   → WM_KEYUP (lParam 31 位置 1)
          │
          ├─ convertKeyCode(keyCode) → VK_xxx (VK_BACK/VK_RETURN/VK_LEFT..)
          │
          ├─ ::SendMessage((HWND)m_nativeId, WM_KEYDOWN, wParam, lParam)
          │
          └─ 如果 ACTION_DOWN + 可打印字符
                再发一条 ::SendMessage(hHost, WM_CHAR, charCode, lParam)   ← 这步才是 SEdit 收字符的关键
                    已覆盖：'0'-'9' (keyCode 7..16), 'a'-'z'/'A'-'Z' (29..54, 支持 Shift/Caps),
                           ' ' (space, 62), '\r' (Enter, 66), '\b' (Back, 67)
```

> 对于复杂输入法（中文、表情），理想方式是用 InputConnection.commitText 直接发文本到 SEdit，而不是走 WM_CHAR 单字符。这块后续可按需扩展（在 SouiView 自定义 InputConnection → JNI `nativeCommitText(nativeId, editViewName, str)` 再拿到 SEdit 指针 `SetSel` + `ReplaceSel`）。

### 9.3 定时器（SetTimer）

```
SOUI 业务调用 SNS::SetTimer(hWnd=m_nativeId, nIDEvent, uElapse, nullptr)
  → swinx ::SetTimer
    → AndroidPlatformAPI::setTimer                             [AndroidPlatformAPI.cpp]
      → 调 Java SouiPlatformBridge.setTimer(hWnd, nIDEvent, uElapse)  [JNI]
        → Handler.postDelayed(TimerRunnable, interval)         [SouiPlatformBridge.java:541]
          → TimerRunnable.run()
            → nativeOnTimerExpired(hWnd, timerId)              [JNI]
              → AndroidPlatformAPI::onTimerExpired
                ├─ 注册 TIMERPROC 了就调 proc
                └─ 否则 ::PostMessage(hWnd, WM_TIMER, timerId, 0)
                     → SHostWnd::OnTimer → DispatchMessage → CMainDlg::OnTimer
```

> ⚠️ 销毁页面时：如果 CMainDlg 自己没 KillTimer 也没事，`SouiViewNative` 析构 → SHostWnd 销毁 → swinx `SNativeWnd::OnDestroy()` 会自动 `::KillWindowTimers(m_hWnd)` → SouiPlatformBridge 也会把对应的 Runnable cancel 掉。不会泄漏。

---

## 10. 生命周期 & 线程安全

### 10.1 创建生命周期

```
1. SouiView.initSouiLayout("layout:dlg_main")  [Java]
   ↓
2. nativeCreate("layout:dlg_main")  [JNI]
   ↓
3. new SouiViewNative(env, thiz, layoutStr)
   ↓
4. SouiViewNative::initialize
     a. s_hostWndFactory ? createHostWnd(layout) : new SHostWnd(layout.c_str())
     b. 自增 nativeId = g_nextNativeId++ (起始 100)
     c. hostWnd->Attach(m_nativeId)   // 把 nativeId 当 HWND 绑定到 swinx 的 WndMgr
     d. m_hostWnd.reset(hostWnd)      // 用 unique_ptr 管生命周期
     e. return true
   ↓
5. NativeViewMap_Insert(nativeId, shared_ptr<SouiViewNative>)  // ⭐ 映射写表
   ↓
6. Java 侧 registerView(nativeId, this)  // SouiPlatformBridge HWND-View map
7. nativeOnSizeChanged(nativeId, w, h)   // 如果此时 View 已有尺寸 → 直接触发 SWINX WM_SIZE + SHostWnd 重新布局
8. createOffscreenBitmap(w,h)            // 分配 ARGB_8888 Bitmap，SOUI 画到它上面
```

### 10.2 销毁生命周期（完全防 UAF / double destroy）

```
1. 用户按返回键 / 切换 Activity → SouiView.onDetachedFromWindow   [SouiView.java:202]
   ↓
2. unregisterView(nativeId)  // HWND-View map 删除
   ↓
3. nativeDestroy(nativeId)   [JNI]
   ↓
4. NativeViewMap_Lookup(nativeId)   // native-lib.cpp
      - 找不到：LOGW + return (防重复 destroy)
   ↓
5. NativeViewMap_Erase(nativeId)    // 先从 map 移除（阻止新的 lookup 拿到它）
   ↓
6. 函数退出 → 局部 shared_ptr<SouiViewNative> 引用计数归零
   ↓
7. delete SouiViewNative
     → ~SouiViewNative()
       → m_hostWnd.reset()
         → SHostWnd::~SHostWnd → OnDestroy → KillWindowTimers → Detach(nativeId) 等 Windows 端完整流程
```

并发安全性（场景：主线程 onTouchEvent 还在跑，另一条业务线程调了 nativeDestroy）：
- Lookup 返回 shared_ptr 拷贝 → 调用期间引用计数 +1
- Erase 只是 map 的引用计数减一 → 真正 `delete SouiViewNative` 要等 **所有 lookup 持有者都退出作用域**才发生
- 所以不会出现「OnTouchEvent 跑到一半，对象析构了」的 UAF

### 10.3 线程黄金规则

| 操作 | 允许线程 | 不允许的做法 |
|---|---|---|
| `initSouiLayout / nativeDestroy` | **主线程 (UI 线程)** | 工作线程直接调 |
| `SouiViewNative::onTouchEvent / onKeyEvent / render` | 主线程 (Android View 回调线程) | 跨线程直接调这些方法 |
| `::SendMessage / ::PostMessage` to `(HWND)nativeId` | 任何线程（swinx 内部有加锁） | ✅ 可以，这是推荐的跨线程通信方式 |
| SHostWnd / SWindow 成员函数 | **只在主线程**（SOUI 不做内部加锁） | 工作线程直接 `pEdit->SetWindowTextW`；应该用 `::PostMessage` + 自定义 WM_USER + 在消息处理里改 UI |

---

## 11. 构建 & 调试

### 11.1 命令行构建

```powershell
# 在 app 根目录（Windows PowerShell 5）
cd D:\your-android-project
.\gradlew.bat --no-daemon :app:assembleDebug
```

产物在 `app/build/outputs/apk/debug/app-debug.apk`。

### 11.2 日志过滤（定位问题第一招）

```powershell
# 安装后看 SOUI 相关日志
adb logcat -c
adb logcat -s soui-native SouiViewNative AndroidPlatformAPI SouiPlatformBridge swinx SHOST
```

常见关键字：
- `SouiView_nativeCreate succeeded, nativeId=100` → JNI 创建成功
- `unknown nativeId=100` 出现在 onSizeChanged / onTouchEvent → 说明 destroy 已经执行，Java 层却还在给这个 View 发事件（正常 edge case，框架已忽略，不需要处理）
- `Host window created via APP ISHostWndFactory: 0x7f123456` → 你的工厂返回了自定义类 ✅
- `Host window created as default SHostWnd: 0x...` → 工厂没命中，检查传入字符串与工厂里映射名是否完全相等（包括 `layout:` 前缀）

### 11.3 C++ 断点调试

1. Android Studio 打开项目，顶部 Run 按钮左边的 debugger 选择 `Attach Debugger to Android Process` → 选 `Java + Native (Lldb)`
2. 下断点：
   - 业务断点：`CMainDlg::OnInitDialog`、`CMainDlg::OnOK`
   - 输入链路断点：`SouiViewNative.cpp:166`（SendMessage 一行，看 msg/wParam/lParam 值是否正确）
   - 生命周期断点：`native-lib.cpp NativeViewMap_Erase` / `SouiViewNative::~SouiViewNative`
3. 用手机运行 app，命中后可以检查：
   - `this->m_nativeId` 是不是正整数（100/101/…）不是则 Attach 失败
   - `(SWindow*)m_editInput` 是否非空（空的话去检查布局里 name 拼写和 FindChildByName2 字符串是不是完全一致，**大小写敏感**）

---

## 12. 常见 FAQ

### Q1: `SouiViewNative::getNativeId()` 返回 100 这种小整数，强转就 crash？

✅ 已修复。`native-lib.cpp` 现在用 `s_nativeViewMap<jlong, shared_ptr<SouiViewNative>>` 查表，不再 `reinterpret_cast<SouiViewNative*>(nativeId)`。升级到最新代码即可。

### Q2: 启动后屏幕一片空白？

排查顺序：
1. `nativeId != 0` 吗？（否则 nativeCreate 失败，看 log "SouiView_nativeCreate succeeded" 有没有）
2. `offscreenBitmap` 分配了吗？（width/height > 0 才会分配；如果 View 是 wrap_content 可能 0，手动在布局里给 SouiViewGroup match_parent）
3. SOUI 布局 XML 真的被加载了吗？如果 SHostWnd 构造时找不到指定的 resource，UI 树是空的画不出来。
   - 加 log：在 `CMainDlg::OnInitDialog` 打 `FindChildByName2(L"txt_clock")` 的返回值，nullptr = 布局没加载成功。
4. ARGB_8888 Bitmap 被正确画了吗？断点在 `SouiViewNative::render`，如果里面 skia render 根本没跑，看 SHostWnd 是否被正确 Show/UpdateWindow。

### Q3: 按钮点击没反应？

- 检查 id 对应：XML `<button id="IDOK">` 和 C++ `EVENT_ID_COMMAND(SNS::IDOK, OnOK)` 必须同一个数值常量（`IDOK` 是标准 Win32 常量 = 1；自定义 id 请在 resource.h 里用整数声明，两边保持一致）。
- 或者用 `EVENT_NAME_COMMAND(L"btn_ok", OnOK)` 基于 name 绑定（更推荐跨平台项目，不怕 id 对不上）。

### Q4: 键盘能弹起，但字符打不到 SEdit？

- 检查 `SouiViewNative::onKeyEvent` 里的 ACTION_DOWN 是否走到了 `::SendMessage(..., WM_CHAR, charCode, ...)`。
- 另外 SEdit 必须拿到焦点。可以在 CMainDlg::OnInitDialog 末尾 `m_editInput->SetFocus();`。
- 对中文输入法，需要扩展 InputConnection（见 9.2 章节末尾说明）。

### Q5: SetTimer 不触发？

- 先看 logcat 过滤 AndroidPlatformAPI：`setTimer` 时应该会打印 JNI 调用到 Java；如果 `m_setTimerMethod == nullptr`，说明 `RegisterAndroidPlatformAPI` 没跑到 → 回到第 4.2 节检查初始化顺序：**必须先 `SouiPlatformBridge.init()` 再干别的**。

### Q6: 自定义弹框 / 子 HWND 怎么弄？

跟 Windows 端完全一样：写一个 `CSettingsDlg : SHostWnd`，在 CMainDlg::OnOK 里 `m_dlgSettings.Create(GetContainer()->GetHwnd(), ...)` → 触发 swinx `::CreateWindowEx → AndroidPlatformAPI::createWindow → SouiPlatformBridge.createWindow` → 在当前 SouiViewGroup 里 new 一个 Android View 放进去（子 SHostWnd 的绘制走新的 nativeId / SouiView 实例链路，复用同样的机制）。

### Q7: 能在一个 Activity 里放多个 SouiViewGroup / 多个 SHostWnd 吗？

**可以**。每个 SouiViewGroup 自己会带一个独立的 SouiView、独立的 nativeId、独立的 C++ SouiViewNative 实例（map 里不同的 key，互不干扰）。Android 端页面内嵌多个 SOUI 子面板完全支持。

---

## 13. 项目骨架速查（目录清单）

```
MySouiApp/
├─ settings.gradle                                     // 含 include ':soui-android-lib'
├─ app/
│  ├─ build.gradle                                     // implementation project(':soui-android-lib')
│  └─ src/main/
│     ├─ AndroidManifest.xml                           // android:name=".App"
│     ├─ assets/soui/
│     │   ├─ uires.zip                                 // 全部 SOUI 资源（layout/skin/sys 等）
│     │   └─ 或
│     │       uires.idx + layout/ skin/ sys/
│     ├─ java/com/yourcompany/app/
│     │   ├─ App.kt                                    // SouiPlatformBridge.init / NativeAppFactory.registerAppFactory
│     │   ├─ NativeAppFactory.java                     // JNI 包装
│     │   └─ MainActivity.kt                           // initSouiLayout("layout:dlg_main")
│     ├─ cpp/
│     │   ├─ MainDlg.h / MainDlg.cpp                   // ⭐ 你的 SHostWnd 子类 + 业务
│     │   └─ NativeAppFactory.cpp                      // ISHostWndFactory 实现
│     └─ res/layout/activity_main.xml                  // 放 <com.soui.SouiViewGroup/>
└─ soui-android-lib/  (来自 soui4 仓库的 soui-android-lib，不要动它)
   ├─ build.gradle
   └─ src/main/
      ├─ java/com/soui/ (SouiView/SouiViewGroup/SouiPlatformBridge)
      └─ cpp/  (native-lib/SouiViewNative/AndroidPlatformAPI/CMakeLists.txt)
```

祝 SOUI Android 开发顺利。如果扩展了资源 provider / InputConnection 中文输入，也欢迎补到这个文档里。
