# SOUI 跨平台资源编译技术

## 概述

SOUI 框架在 Linux/macOS 平台下实现了一套完整的资源编译与解码机制，通过将 Windows 平台的 .rc 资源文件编译到 ELF 格式的 COFF 数据段中，实现了跨平台统一的资源管理模式。这项技术使得开发者可以在不同平台上使用相同的资源管理方式，并且能够实现单个可执行文件的发布。

## 技术架构

整个跨平台资源系统由三个核心部分组成：

1. **Windows 资源 API 兼容层** - 在 swinx 模块中实现
2. **COFF 格式解析器** - 用于解析编译后的资源数据
3. **CMake 编译工具链** - 将 .rc 文件编译并链接到目标程序

## 核心组件详解

### 1. Windows 资源 API 兼容层 (winres.cpp)

在 `swinx/src/winres.cpp` 中实现了一套完整的 Windows 资源 API 兼容接口，包括：

- `FindResourceW/A` - 查找资源
- `LoadResource` - 加载资源
- `LockResource` - 锁定资源数据
- `SizeofResource` - 获取资源大小
- `EnumResourceNamesW/A` - 枚举资源名称
- `EnumResourceTypesW/A` - 枚举资源类型
- `EnumResourceLanguagesW/A` - 枚举资源语言
- `LoadStringW/A` - 加载字符串资源

这些 API 在 Linux/macOS 平台上提供了与 Windows 平台完全一致的接口，使得上层代码无需修改即可跨平台运行。

**关键实现**：

```cpp
static const char *kCoof_o_start = "_binary_soui_coff_o_start";
static const char *kCoof_o_end = "_binary_soui_coff_o_end";
```

通过这两个导出符号定位资源数据段的起始和结束位置，使用 `dlsym` 函数在运行时动态获取资源数据。

### 2. COFF 格式解析器 (coffparser.cpp)

`WindResResourceParser` 类负责解析 COFF 格式的资源数据，主要功能包括：

- **段查找**：在 COFF 文件中查找 `.rsrc`、`.rdata` 或 `.data` 段
- **目录解析**：递归解析资源目录结构（类型-名称-语言三级结构）
- **资源查找**：根据类型、名称和语言查找特定资源
- **数据提取**：从资源数据段中提取实际的资源数据

**资源目录结构**：

Windows 资源采用三级目录结构：
1. 第一级：资源类型（如 RT_BITMAP、RT_STRING 等）
2. 第二级：资源名称（ID 或字符串名称）
3. 第三级：语言 ID

解析器按照这个结构递归遍历，构建出内存中的资源映射表。

### 3. CMake 编译工具链 (windres.cmake)

`target_compile_resources` 函数实现了完整的资源编译流程，包含以下步骤：

#### 步骤 1：编译 .rc 到 COFF 格式

```cmake
${WINDRES_EXE} --target=pe-x86-64 ${include_flags} -i "${rc_abs_path}" -o "${coff_obj}" -O coff
```

使用 MinGW 的 `windres` 工具将 .rc 文件编译为 64 位 Windows PE 格式的 COFF 对象文件。

#### 步骤 2：链接 COFF 到 ELF 格式

```cmake
${LD_EXECUTABLE} -r -b binary "${coff_obj}" -o "${elf_obj}"
```

使用 `ld` 工具将 COFF 格式的二进制文件转换为 ELF 格式的对象文件。

#### 步骤 3：符号重命名

```cmake
${NM_EXECUTABLE} -g ${elf_obj} > ${symbol_list}
${OBJCOPY_EXECUTABLE} --redefine-syms=${rename_script} ${elf_obj} ${final_obj}
```

提取符号信息并重命名，确保使用固定的 `soui` 前缀，便于运行时定位。

#### 步骤 4：链接到最终产物

```cmake
target_sources(${target} PRIVATE ${resource_objects})
target_link_options(${target} PRIVATE 
    -rdynamic
    -Wl,--version-script=${PROJECT_SOURCE_DIR}/__cmake/export_res_sym.lst
)
```

将编译好的资源对象文件添加到目标，并配置链接器导出资源符号。



## 技术优势

### 1. 统一的资源管理

- 在所有平台上使用相同的资源 API
- 资源文件格式完全一致（.rc 文件）
- 资源查找和加载逻辑统一

### 2. 单文件发布

- 资源数据直接嵌入可执行文件
- 无需额外的资源文件或配置文件
- 简化部署和分发流程

### 3. 性能优化

- 资源数据在编译时嵌入，运行时直接访问
- 避免文件 I/O 操作
- 减少启动时间

### 4. 开发效率

- 无需为不同平台维护不同的资源文件
- 使用熟悉的 Windows 资源编辑工具
- 降低跨平台开发的学习成本

## 使用方法

### 在 CMake 中启用资源编译

```cmake
include(windres)

# 为目标添加资源文件
target_compile_resources(your_target 
    resources/app.rc
    resources/icons.rc
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

### 在代码中使用资源

```cpp
// 查找并加载资源
HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_MYRESOURCE), RT_RCDATA);
DWORD size = SizeofResource(NULL, hRes);
HGLOBAL hData = LoadResource(NULL, hRes);
void* pData = LockResource(hData);

// 使用资源数据...
```

### 与 SOUI 集成

依托 SOUI 内置的 `SResProviderPE` 模块，可以直接从嵌入的资源数据段创建 UI：

```cpp
// 创建资源提供器
CAutoRefPtr<IResProvider> pResProvider;
CreateResProvider(RES_PE, (IObjRef**)&pResProvider);
pResProvider->Init((WPARAM)GetModuleHandle(NULL), 0);

// 加载 UI 资源
m_pRenderFactory->SetResProvider(pResProvider);
```
## 实际应用示例

以下以 `soui4.git/demos/demo` 项目为例，详细介绍如何使用这套跨平台资源编译技术。

### 1. CMakeLists.txt 配置

在项目的 CMakeLists.txt 中，资源编译的配置非常简单：

```cmake
# 收集所有资源文件（.rc 和 .rc2）
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

# 添加可执行目标
add_soui_exe(demo ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

# 启用资源编译（非 Windows 平台）
target_compile_resources(demo ${CURRENT_RC})
```

**关键点**：
- `file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)` 自动收集所有资源文件
- `target_compile_resources` 函数在非 Windows 平台会自动处理资源编译
- Windows 平台原生支持 .rc 文件，此函数不会产生额外操作

### 2. 资源文件结构

demo 项目的资源文件包含：

```
demo/
├── demo.rc              # 主资源文件
├── res/
│   ├── soui_res.rc2     # SOUI 资源定义
│   └── resource.h       # 资源 ID 定义
└── uires/               # UI 资源目录
    ├── uires.idx        # 资源索引
    ├── xml/             # 布局文件
    ├── image/           # 图片资源
    └── values/          # 颜色、字符串等值
```

### 3. 代码中的资源加载配置

在 `demo.cpp` 中，通过条件编译实现跨平台的资源加载：

```cpp
// 定义资源加载类型
#define RESTYPE_FILE  0  // 从文件中加载资源，加载失败再从 PE 加载
#define RESTYPE_PE    1  // 从 PE 资源中加载 UI 资源
#define RESTYPE_ZIP   2  // 从 zip 包中加载资源
#define RESTYPE_7Z    3  // 从 7zip 包中加载资源

// 根据构建类型选择资源加载方式
#ifdef _DEBUG
#define RES_TYPE RESTYPE_FILE      // Debug 模式从文件加载
#else
#define RES_TYPE RESTYPE_PE        // Release 模式从 PE 资源加载
#endif
```

**系统资源配置**：

```cpp
// 加载系统资源
#ifdef ENABLE_BUILD_RESOURCE
    // 启用了资源编译时，从当前模块的 PE 段加载
    #if (defined(LIB_CORE) && defined(LIB_SOUI_COM))
        cfg.SetSysResPeHandle(hInstance);
    #else
        cfg.SetSysResPeFile(SYS_NAMED_RESOURCE);
    #endif
#else
    // 未启用资源编译时，从文件系统加载
    cfg.SetSysResFile(appDir + kPath_SysRes);
#endif
```

**应用程序资源配置**：

```cpp
#ifdef ENABLE_BUILD_RESOURCE
    // 从 PE 资源段加载
    cfg.SetAppResPeHandle(hInstance);
#elif (RES_TYPE == RESTYPE_ZIP)
    // 从 ZIP 包加载
    cfg.SetAppResZipFile(appDir + _T("/uires.zip"), "souizip");
#elif (RES_TYPE == RESTYPE_7Z)
    // 从 7z 包加载
    cfg.SetAppRes7ZipFile(appDir + _T("/uires.zip"), "souizip");
#else
    // 从文件加载
    cfg.SetAppResFile(appDir + _T("/uires"));
#endif
```

### 4. 资源 ID 的集成

在代码中使用资源 ID：

```cpp
// 包含自动生成的资源头文件
#define INIT_R_DATA
#include "res/resource.h"

// 在应用程序初始化时注册命名 ID
app.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));

// 在代码中使用资源 ID
COLORREF crRed = GETCOLOR(R.color.red);
SStringW strTitle = GETSTRING(R.string.title);
```

### 5. 完整的初始化流程

```cpp
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
    // 初始化 COM
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    // 设置工作目录
    SStringT appDir = getSourceDir();
    SetCurrentDirectory(appDir);

    // 创建应用程序实例
    SApplication2 app(hInstance);
    SAppCfg cfg;
    
    // 配置渲染器和资源加载方式
    cfg.SetRender(nType == IDYES ? Render_Skia : Render_Gdi)
       .SetImgDecoder(ImgDecoder_Stb)
       .SetAppDir(appDir)
       .SetLog(TRUE, 2, "demo")
       .EnableMultiLang(_T("translator:lang_cn"), TRUE);

    // 配置系统资源加载方式
#ifdef ENABLE_BUILD_RESOURCE
    cfg.SetSysResPeHandle(hInstance);
#else
    cfg.SetSysResFile(appDir + kPath_SysRes);
#endif

    // 配置应用程序资源加载方式
#ifdef ENABLE_BUILD_RESOURCE
    cfg.SetAppResPeHandle(hInstance);
#else
    cfg.SetAppResFile(appDir + _T("/uires"));
#endif

    // 执行配置
    if (!cfg.DoConfig(&app))
    {
        return -1;
    }

    // 初始化命名 ID
    app.InitXmlNamedID((const LPCWSTR *)&R.name, (const int *)&R.id, sizeof(R.id) / sizeof(int));
    int nRet = 0;
    {
        // 运行应用程序
        CMainDlg dlgMain;
        dlgMain.Create(GetActiveWindow(), 0, 0, 888, 650);
        dlgMain.ShowWindow(SW_SHOWNORMAL);

        nRet = app.Run(dlgMain.m_hWnd);
    }
    OleUninitialize();
    return nRet;
}
```

## 技术细节

### 符号导出机制

系统通过导出两个特殊符号来定位资源数据段：

- `_binary_soui_coff_o_start` - 资源数据段起始地址
- `_binary_soui_coff_o_end` - 资源数据段结束地址

这些符号由链接器自动生成，运行时通过 `dlsym` 获取。

### 符号导出脚本控制

`export_res_sym.lst` 文件控制哪些符号需要导出，确保资源符号正确暴露给运行时系统。

### 平台兼容性

- **Linux**：使用 MinGW-w64 工具链编译资源
- **macOS**：使用交叉编译的 MinGW 工具链
- **Windows**：原生支持，无需特殊处理

## 总结

SOUI 的跨平台资源编译技术通过巧妙的架构设计，实现了：

1. 完全兼容 Windows 资源 API 的接口层
2. 高效的 COFF 格式解析器
3. 自动化的 CMake 编译工具链

这套技术方案不仅保持了跨平台的一致性，还提供了优秀的性能和开发体验，是 SOUI 框架跨平台能力的重要组成部分。开发者可以使用熟悉的 Windows 资源开发方式，在 Linux/macOS 平台上获得相同的功能和体验，同时享受单文件发布的便利性。

通过 demo 项目的实际应用可以看到，这套技术的使用非常简单，只需在 CMakeLists.txt 中添加一行 `target_compile_resources`，在代码中通过条件编译选择合适的资源加载方式，即可实现跨平台的资源管理。无论是开发调试还是最终发布，都能获得良好的体验。
