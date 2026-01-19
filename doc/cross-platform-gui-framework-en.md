# Native GUI Framework Programming on Windows, Linux and macOS: SOUI
Developing native GUI frameworks on Windows and Linux platforms usually presents different technical selection challenges. SOUI cross-platform framework provides a unique solution for developers.

SOUI is a completely open-source cross-platform client UI framework that has been continuously developed and maintained for 14 years. Its core compiles to approximately 2MB for windows, and additionally about 7MB for linux and macOS. The framework draws inspiration from mainstream frameworks such as WTL, Android, Qt, and Flash, making it suitable for high-performance desktop application development. The code is completely open-source and hosted on:
- GitHub: https://github.com/soui4/soui
- Gitee: https://gitee.com/setoutsoft/soui4 

SwinX is the cross-platform compatibility layer that enables SOUI to run on non-Windows platforms. It is also open-source and available at:
- GitHub: https://github.com/soui4/swinx
- Gitee: https://gitee.com/setoutsoft/swinx

## Current State of Native GUI Framework Programming on Windows, Linux and macOS

In traditional cross-platform GUI development, developers typically need to select different technology stacks for different platforms:

- **Windows**: Usually uses native technologies such as Win32 API, WTL, MFC, WPF, WinForms
- **Linux/macOS**: Usually uses cross-platform frameworks such as GTK, Qt, wxWidgets

In this situation, developers need to learn and maintain multiple code bases, increasing development costs and maintenance difficulties.

## Comparison of SOUI with Traditional Cross-Platform Frameworks

Compared to traditional cross-platform development frameworks (such as Qt, GTK), SOUI has the following significant advantages:

### Comparison with Qt

- **Smaller Size**: SOUI's core compiles to approximately 2MB, compared to Qt's hundreds of MB, taking up less space and offering easier deployment
- **Shallower API Wrapping**: SOUI provides closer-to-native API wrapping (events are passed to SOUI for processing after system events are captured, requiring only one forwarding), meaning higher runtime efficiency and lower performance overhead
- **Faster Startup Speed**: Due to its smaller size, SOUI applications start faster

### Comparison with GTK

- **Native C++ Development**: SOUI uses C++ development, which is more suitable for modern C++ development compared to GTK's C language style
- **Native macOS Support**: Through the SwinX compatibility layer, SOUI can natively run on macOS, while GTK's support on macOS is relatively limited
- **More Complete Cross-Platform Experience**: SOUI provides consistent development experience on Windows, Linux, and macOS platforms

## Advantages of SOUI Cross-Platform Framework

SOUI brings the following significant advantages to developers through its unique cross-platform architecture:

### 1. Unified Programming Interface

SOUI implements compatibility for Windows API on Linux and macOS platforms based on its self-developed SwinX cross-platform compatibility layer. This means:

```cpp
#include <windows.h>  // Using the same header files on all platforms
// No need to modify code for different platforms, the same C++ code can be compiled and run on three major desktop platforms
```

SwinX is a Wine-like compatibility layer that specifically implements necessary Windows client APIs for SOUI development needs, allowing Windows client code to run on Linux and macOS as if on Windows.

### 2. Maintaining Windows Development Experience

SOUI allows developers to continue using familiar Windows development interfaces, without learning new APIs or changing programming habits:

- Maintain the Win32 API programming model
- No need to learn new platform GUI frameworks
- Unified debugging and development environment

### 3. Efficient Cross-Platform Code Reuse

With SOUI, developers only need to write one set of code to run on multiple platforms. Core functions include:

- XML-driven UI interface definition
- Rich control library (buttons, lists, trees, rich text, etc.)
- High-performance rendering (GDI/Skia/D2D)
- Lua/JavaScript script extensions
- Image decoding (STB/GDI+/WIC)
- Network communication (HTTP, WebSocket)

### 4. Adopting Android Development Patterns

SOUI adopts many Android development patterns, making desktop app development as simple as Android development:

- **XML Layout**: Define UI interfaces through XML to achieve separation of UI and logic
- **Native C++ Code**: Business logic is implemented using native C++, ensuring high performance
- **Componentized Architecture**: Similar to Android's componentized design, facilitating modular development

### 5. COM-like Object Management

SOUI's overall design uses COM-like approach to effectively manage object lifecycles:

- Automatic reference counting management
- Greatly reduces common memory leaks in C++ development
- Provides stable object lifecycle management mechanisms

### 6. Flexible Component Architecture

SOUI provides a modular component architecture, allowing developers to choose different components as needed:

```cpp
// Component configuration example
#define COM_IMGDECODER  _T("imgdecoder-stb")     // Image decoder
#define COM_RENDER_GDI  _T("render-gdi")         // Rendering engine
#define COM_SCRIPT_LUA  _T("scriptmodule-lua")   // Script support
#define COM_TRANSLATOR  _T("translator")         // Internationalization support
```

### 7. Rich Extension Features

- Supports APNG animations and property animations
- Built-in task loops and IPC mechanisms
- Visual UI designer (souieditor)
- Multi-language bindings (Lua/JS/Python)
- Resource management (ZIP/7z compressed package resource loading)

### 8. Powerful Animation System

SOUI has a comprehensive animation system supporting multiple types of animation effects to add liveliness to UI:

- **Property Animation**: Based on Android's property animation design concept, supports animation of various control properties such as position, size, transparency, rotation angle, etc.
- **Frame Animation**: Supports APNG format animated images, capable of achieving complex visual effects like particle effects and character actions
- **Window Animation System**: Supports animating windows with effects such as fade, scale, rotation, etc., suitable for window transition animations and other scenarios. Various animation combinations can be defined through XML, thereby achieving complex animation effects.

All three types of animations are unifiedly driven by SOUI's built-in animation timeline system, avoiding fragmented refresh requests and ensuring efficient animation operation.


### 9. Multi-Language Development Support

SOUI not only supports C++ development, but also provides multiple language bindings:
- SOUI4JS - JavaScript version
- SOUI4PY - Python3 version

## How to Use SOUI

### Compilation Process

SOUI's cross-platform characteristics are also reflected in the uniformity of its build system:

- **Windows**: Use CMake to generate Visual Studio projects or compile with MinGW
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019"  # Or your preferred generator
cmake --build . --config Release
```
- **Linux & macOS**: Pull the SwinX submodule via `git submodule update --init`, then compile with CMake
```bash
git submodule update --init
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Basic Application Example

Here is a basic example of a SOUI application's main function:

```cpp
#include <souistd.h>

// Main dialog class inheriting from SHostDialog
class CMainDlg : public SHostDialog, public SDpiHandler<CMainDlg>
{
{
public:
    CMainDlg() : SHostDialog(_T("layout:main_dlg"), NULL)
    {
    }

protected:
    // Event handler for button clicks
    void OnBtnClick()
    {
        SMessageBox(this->m_hWnd, _T("Hello from SOUI!"), _T("SOUI"), MB_OK);
    }

    // Initialize child windows after host is created
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam)
    {
        // Set up event handlers
        FindChildByName(L"btn_hello")->GetEventSet()->subscribeEvent(EventID::Event_Button_Click, 
            Subscriber(this, &CMainDlg::OnBtnClick));
        return TRUE;
    }

    BEGIN_MSG_MAP_EX(CMainDlg)
        CHAIN_MSG_MAP(SDpiHandler<CMainDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

};

// Main entry point
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
    // Initialize SOUI application
    SApplication application(hInstance);
    SAppCfg cfg;
    SStringT appDir = getSourceDir();

    cfg.SetRender(Render_Skia)
        .SetImgDecoder(ImgDecoder_Stb)
        .SetLog(TRUE)
        .SetSysResPeFile(appDir + _T("/soui-sys-resource.dll"))
        .SetAppResPeHandle(hInstance);
    if (!cfg.DoConfig(&application))
    {
        return -1;
    }
    // Create and display main dialog
    int ret = -1;
    {
        CMainDlg dlg;
        dlg.Create(NULL);
        dlg.SendMessage(WM_INITDIALOG);
        dlg.CenterWindow();
        dlg.ShowWindow(SW_SHOW);

        // Start message loop
        ret = application.Run(dlg.m_hWnd);
    }
    return ret;
}
```

## Summary

Through the SwinX compatibility layer, the SOUI framework successfully implements the operation of Windows native GUI programming interfaces on Linux and macOS, enabling developers to use one set of code, one technology stack, and one development model to develop native GUI applications that run on multiple platforms. This solution not only greatly reduces the complexity of cross-platform development, but also maintains the performance and experience of Windows platform native development, making it an efficient solution for cross-platform GUI development.

SOUI is completely open-source with code hosted at:
- Gitee: https://gitee.com/setoutsoft/soui4
- GitHub: https://github.com/soui4/soui

SwinX cross-platform compatibility layer is also open-source with code hosted at:
- Gitee: https://gitee.com/setoutsoft/swinx
- GitHub: https://github.com/soui4/swinx

With SOUI, developers can focus on business logic implementation without worrying about underlying platform differences, truly achieving the development goal of "write once, run on multiple platforms". Its powerful animation system provides rich UI interaction possibilities for developers, allowing cross-platform applications to have smooth and vivid user experiences as well.