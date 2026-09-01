# uiresbuilder —— SOUI UI 资源编译工具

`uiresbuilder` 是 SOUI4 框架配套的**离线资源构建器**（CLI 工具）。它读取一份 XML 格式的资源索引文件 `uires.idx`（描述皮肤包内所有资源的清单：布局 XML、皮肤 XML、图标、位图、字符串、颜色、菜单等），生成**三份**构建产物，供不同平台和语言调用：

| 产物                       | 参数   | 典型文件名                         | 用途                                                                                                                                                                                                                         |
| ------------------------ | ---- | ----------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 资源打包清单（Win32 RC include） | `-r` | `winres.rc2` / `soui_res.rc2` | 被 MSVC `.rc` 脚本 `#include` 后，将整个 UI 资源以 `RCDATA` 自定义资源类型编译进 Windows PE `.exe` / `.dll`，运行期 SWinx 直接从模块资源里解包加载皮肤。                                                                                                           |
| C/C++ 资源访问头文件            | `-h` | `resource.h`                  | 定义 `struct _UIRES`（按资源类型分层访问原始文件路径/名，如 `UIRES.LAYOUT.maindlg`）与 `struct _R`（运行期读取布局中控件的 `R.name.xxx` / `R.id.xxx` / `R.color.xxx` / `R.string.xxx`）。C++ 额外提供 `R.ID.xxx` 匿名 `enum`，可用于 `switch/case`、数组维度、模板参数等**常量表达式**场景。 |
| JS/TS 资源访问模块（可选）         | `-j` | `R.js`                        | 导出 `R { id, name, arrName, arrId }` 四个集合，供 OpenHarmony / Web / Node.js 等非 C++ 环境在脚本层以字符串或 ID 访问布局控件。                                                                                                                       |

它是 SOUI4 资源系统\*\*从"人肉写 resource.h + 手工 #include rc2"升级为"一键生成、跨平台复用"\*\*的核心工具。Windows / Android / HarmonyOS / iOS 四端共用同一套 `uires.idx`，各自调用本工具生成对应产物。

***

## 编译构建

### Windows — Visual Studio

仓库中自带 Visual Studio 工程文件：

```
tools/src/uiresbuilder/
├── uiresbuilder.sln      # VS 2005+ 解决方案
├── uiresbuilder.vcproj   # VS 2005 工程（可升级到高版本）
└── ...
```

直接用 Visual Studio 打开 `uiresbuilder.sln` 并编译即可，产出 `uiresbuilder.exe`（Release / Win32）。

### Windows / Linux / macOS — CMake

```bash
cd tools/src/uiresbuilder
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# 产物位置：build/Release/uiresbuilder 或 build/uiresbuilder
```

CMake 支持 MinGW 交叉编译（CMakeLists.txt 已内建检测）。注意 CMake 规则中显式取消了 `UNICODE/_UNICODE` 定义，以保证跨平台生成的 ASCII `.rc2` / `.h` 与 Windows 版本完全一致。

***

## 命令行用法

```bash
uiresbuilder -i uires.idx [-p uires_prefix] [-r output.rc2] [-h resource.h] [-j R.js] [idtable]
```

### 参数

| 参数          | 是否必填       | 说明                                                                                                                                                                                                                                                                                              |
| ----------- | ---------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `-i <path>` | ✅ 必须       | `uires.idx` 索引文件的路径。工具用 `tinyxml` 解析该 XML，根节点必须为 `<resource>`。解析失败返回退出码 `2`。                                                                                                                                                                                                                    |
| `-p <path>` | 建议指定       | 资源根目录前缀，用于拼接 `.rc2` 里每条资源的相对路径（传给 Win32 `RCDATA` 的文件路径）。典型取值 `uires`、`res/uires` 或项目里实际的皮肤包目录。不指定时 `.rc2` 里会使用 `uires.idx` 内记录的原始 `path`。                                                                                                                                                       |
| `-r <path>` | Windows 必选 | 生成 `.rc2` 资源打包清单。典型输出 `uires/winres.rc2` 或 `res/soui_res.rc2`。配合 `-p` 生成的路径需与 RC 编译时的工作目录相对关系一致。                                                                                                                                                                                                |
| `-h <path>` | 桌面端必选      | 生成 `resource.h`（C/C++ 头）。含 `_UIRES` / `_R` 两个结构体及 `INIT_R_DATA` 分支。`WriteFile` 实现了时间戳增量写入：仅当索引 / 布局文件 / uidef 文件时间戳发生变更时才重写，不会触发无意义的增量编译。**生成的头文件顶部已标注"请勿手动修改"字样**。                                                                                                                             |
| `-j <path>` | 鸿蒙/Web 可选  | 生成 `R.js`（ESM `export const R = { id:{...}, name:{...}, arrName:[...], arrId:[...] }`）。JS 版本中 ID 与 Name 一一对应。                                                                                                                                                                                   |
| `idtable`   | 可选开关       | 位置参数（无前缀），置于命令行末尾。启用后 `resource.h` 的 `_R::ID` 匿名枚举生成逻辑会打开**增量稳定性**：仅当 `uires.idx` 以及所有 `<LAYOUT>` / `<SMENU>` / `<SMENUEX>` / 标记了 `buildId="1"` 的资源文件实际修改后（较 `resource.h` 的 `tmResource` 时间戳更新）才重新分配 ID。**强烈建议所有生产构建都加上** **`idtable`**，避免新增一行 XML 导致全部控件 ID 重排，而使得已发布的 `.rc` / `R.js` 发生不兼容变化。 |

### 最小示例

```bash
# Windows 桌面端完整用法（与 SOUI demos/demo 项目一致）
uiresbuilder.exe ^
    -p uires ^
    -i .\uires\uires.idx ^
    -r .\res\soui_res.rc2 ^
    -h .\resource.h ^
    idtable

# OpenHarmony ohos-demo：生成 JS 侧 R.js + C++ 侧 resource.h
uiresbuilder \
    -p uires \
    -i entry/src/main/resources/rawfile/uires/uires.idx \
    -h entry/src/main/cpp/include/resource.h \
    -j entry/src/main/ets/soui/R.js \
    idtable
```

无参数 / 缺 `-i` 会打印用法并退出（退出码 `1`）：

```
not specify input file, using -i to define the input file
usage: uiresbuilder -p uires -i uires\uires.idx -r ./uires/winres.rc2 -h ./uires/resource.h -j ./uires/R.js idtable
        param -i : define uires.idx path
        param -p : define path of uires folder
        param -r : define path of output .rc2 file
        param -h : define path of output resource.h file
        param -j : define path of output R.js file
```

***

## uires.idx 输入文件格式

`uires.idx` 是一份 UTF-8 编码的 XML，根节点 `<resource>`。**每个子节点标签名就是资源类型名**（会原样出现在 `UIRES.<类型名>.<资源名>` 访问路径上），内部用 `<file name="逻辑名" path="相对 idx 的物理路径"/>` 列出该类型下所有资源文件。

典型结构：

```xml
<?xml version="1.0" encoding="utf-8"?>
<resource>
  <!-- 全局 UI 初始化定义（在 R 生成阶段 ParseUIDefFile 解析出字符串/颜色的全局映射表） -->
  <UIDEF>
    <file name="xml_init" path="uidef/init.xml" />
  </UIDEF>

  <!-- values 类型：字符串、颜色、皮肤、模板 -->
  <values>
    <file name="string"   path="values/string.xml" />
    <file name="color"    path="values/color.xml" />
    <file name="skin"     path="values/skin.xml" />
    <file name="template" path="values/template.xml"/>
  </values>

  <!-- 图标资源 -->
  <ICON>
    <file name="ICON_LOGO" path="image/img_logo.ico" />
  </ICON>

  <!-- 位图资源 -->
  <IMGX>
    <file name="png_knob" path="image/knob.png" />
  </IMGX>

  <!-- 菜单（SMENU / SMENUEX 会参与控件 id/name 扫描，生成到 R.id / R.name） -->
  <SMENU>
    <file name="menu_edit"  path="smenu/menu_edit.xml" />
  </SMENU>
  <SMENUEX>
    <file name="menu_tray"  path="xml/menuex_test.xml" />
  </SMENUEX>

  <!-- 布局 XML（LAYOUT 会参与控件 id/name 扫描） -->
  <LAYOUT>
    <file name="maindlg"     path="xml/dlg_main.xml" />
    <file name="page_about"  path="xml/page_about.xml" buildId="1" />
    <!-- buildId="1"：强制该文件参与 id/name 扫描与 ID 枚举
         （LAYOUT/SMENU/SMENUEX 默认已经开启，此属性用于自定义类型的补充标记） -->
  </LAYOUT>

  <!-- 自定义类型：标签名任意，自动收入 UIRES.<类型名> -->
</resource>
```

### 类型分类

| 分类   | 类型名（XML 标签）                                           | 是否参与 ID/Name 扫描                       | 说明                                                                                                                                             |
| ---- | ----------------------------------------------------- | ------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| 强制扫描 | `LAYOUT`, `SMENU`, `SMENUEX`                          | ✅ 是                                   | 解析内部 XML，提取所有控件的 `id="..."`（整数或字符串数值）/ `name="..."`，生成 `R.id.<控件id>` 与 `R.name.<控件name>` 以及 `R.ID.<控件id>` 常量枚举。ID 起始值 `KStartID = 0x00010000`。 |
| 可选扫描 | 任意自定义类型                                               | ❌ 默认否，可在 `<file>` 上加 `buildId="1"` 开启 | 用于对非布局 XML（如子页面引用的外部 XML）同样做扫描纳入 `R`。                                                                                                          |
| 特殊解析 | `UIDEF`                                               | —                                     | 解析子文件内的 `<string>` / `<color>` 全局映射表，生成 `R.string.xxx` / `R.color.xxx`（含 C++ 与 JS 两端）。                                                         |
| 纯打包  | `values`, `ICON`, `IMGX`, `CURSOR`, `GIFX`, `FONT`, … | ❌ 否                                   | 只出现在 `.rc2` 的 `DEFINE_UIRES` 列表以及 `UIRES.<TYPE>.<name>` 路径里，不做内容解析。                                                                            |

***

## 输出产物详解

### 1. `.rc2`（RCDATA 资源打包清单）

每条资源一行 `DEFINE_UIRES(name, type, "file_path")`：

```cpp
#define DEFINE_UIRES(name, type, file_path)    name type file_path

DEFINE_UIRES(xml_init,    UIDEF,  "uires\uidef\init.xml")
DEFINE_UIRES(string,      values, "uires\values\string.xml")
DEFINE_UIRES(color,       values, "uires\values\color.xml")
DEFINE_UIRES(maindlg,     LAYOUT, "uires\xml\dlg_main.xml")
DEFINE_UIRES(ICON_LOGO,   ICON,   "uires\image\img_logo.ico")
...
```

Win32 端典型用法（.rc 文件末尾）：

```cpp
#include "res/soui_res.rc2"
```

RC 编译后，每一行展开成 `<name> <type> <file_path>`，即 Win32 `RCDATA`：资源 ID = `MAKEINTRESOURCE(name字符串)`，资源类型 = `type字符串`（注意 Win32 RC 下 LAYOUT/values 等既可以是整型自定义类型也可以是字符串类型，SOUI SWinx 内部统一按字符串类型 `FindResource` 查找）。SApplication 在启动时按类型字符串枚举所有资源并解包。

### 2. `resource.h`（C/C++ 头）

结构分三层：

#### a. `_UIRES` —— 访问原始资源清单

```cpp
struct _UIRES {
    struct _UIDEF { const char* xml_init; } UIDEF;
    struct _values { const char* string; const char* color; ... } values;
    struct _LAYOUT { const char* maindlg; const char* page_about; ... } LAYOUT;
    struct _ICON   { const char* ICON_LOGO; ... } ICON;
    ...
};
```

用法：

```cpp
SHostWnd mainWnd(UIRES.LAYOUT.maindlg);  // SHostWnd 构造时传入布局 XML 的逻辑名
```

#### b. `_R` —— 运行期访问控件 / 字符串 / 颜色

```cpp
struct _R {
    struct _name { const char* btn_ok; const char* edit_username; ... } name;
    struct _id   { int btn_ok; int edit_username; ... } id;
#ifdef __cplusplus
    // 仅 C++：匿名 enum，值与 id.* 完全相同，但属于编译期常量
    struct {
        enum { btn_ok = 10001, edit_username = 10002, ... };
    } ID;
#endif
    struct _color { /* values/color.xml 里的命名色 */ } color;
    struct _string{ /* values/string.xml 里的命名串 */ } string;
};
```

用法：

```cpp
EVENT_NAME_COMMAND(R.name.btn_ok,        OnBtnOk);     // 按"字符串名"订阅事件
EVENT_ID_COMMAND  (R.id.btn_ok,          OnBtnOk);     // 按"整数 ID"订阅事件
switch (nID) {
  case R.ID.btn_ok: ... break;                        // C++ 常量表达式，编译期枚举
}
```

#### c. `INIT_R_DATA` —— 实例化全局对象

项目**仅一个**编译单元（通常主 `.cpp`）在 `#include "resource.h"` 之前定义：

```cpp
#define INIT_R_DATA
#include "resource.h"    // 本单元会输出 R / UIRES 两个全局实例本体
```

其余编译单元直接 `#include "resource.h"` 即可（走 `extern` 声明分支）。或者用项目里已经内置的：

```cpp
#include "resource.h"
ROBJ_IN_CPP   // 等价于 R 和 UIRES 两个 const 实例定义（写在任意 .cpp 顶层）
```

> `ROBJ_IN_CPP` 宏定义在 resource.h 靠近顶部位置，是生成内容的一部分。

### 3. `R.js`（JS/TS ESM 模块）

```javascript
export const R = {
  name: { btn_ok: "btn_ok", edit_username: "edit_username", ... },
  id:   { btn_ok: 10001, edit_username: 10002, ... },
  arrName: [ ["btn_ok","btn_ok"], ... ],   // 有序对数组，遍历用
  arrId:   [ [10001,"btn_ok"], ... ]       // 有序对数组
};
```

在 OpenHarmony ArkTS / Node.js 里直接 `import { R } from './R.js'`。需要 TS 类型定义的话可自行生成同名 `.d.ts`：`declare export const R: { id: Record<string, number>, name: Record<string, string>, ... }`。

***

## ID 分配与增量稳定性

未加 `idtable` 时，每次运行都会从 `KStartID = 0x00010000` 重新按扫描顺序分配 ID。只要 `<LAYOUT>` 内新增一个带 `id` 的控件，**后续所有控件 ID 都会 +1 偏移**，导致新旧版本的 `R.id.xxx` / `R.ID.xxx` 不一致，严重影响：

1. 已发布模块间的整数 ID 契约（如游戏逻辑 dll 里硬编码了某个按钮 ID）被破坏
2. `.rc2` / `resource.h` 每次都被重写，触发 C++ 全量重编译
3. 鸿蒙侧 `R.js` 与 C++ 侧 `resource.h` 不同步

解决：**始终加上** **`idtable`** **参数**。开启后，`resource.h` 会在文件首部用 BOM 写入 `tmResource = (bBuildIDMap ? 1 : 0) + Σ GetLastWriteTime(被扫描文件)` 作为时间戳；如果 uires.idx 没变且所有被扫描文件都没有更新，**整个** **`resource.h`** **/** **`R.js`** **被跳过不重写**。

> 注意：如果把 `<file>` 物理文件从磁盘拷贝过来但修改时间被 "touch" 成较新的值，会触发重排。建议在 CI / 构建机上稳定运行 `idtable` 构建，不要手工跨机器拷贝替换个别 XML。

***

## 与 SOUI demos 集成方式

参考 `demos/demo`（SOUI4 经典桌面示例）、`demos/ohos-demo`（鸿蒙示例）、`games/cnchess`（跨平台示例）这三个工程的 CMakeLists / PreBuild 步骤，通常以**构建前事件**运行：

```cmake
# CMakeLists.txt 片段：编译 demo 前生成 resource.h + soui_res.rc2
add_custom_command(TARGET demo PRE_BUILD
  COMMAND uiresbuilder
            -p uires
            -i ${CMAKE_CURRENT_SOURCE_DIR}/uires/uires.idx
            -r ${CMAKE_CURRENT_SOURCE_DIR}/res/soui_res.rc2
            -h ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
            idtable
  DEPENDS uiresbuilder ${CMAKE_CURRENT_SOURCE_DIR}/uires/uires.idx
  COMMENT "[uiresbuilder] Generating resource.h + soui_res.rc2 for demo"
)
```

Visual Studio `.vcxproj` 可以用同样方式写 `<CustomBuildStep>`：

```xml
<Command>$(OutDir)uiresbuilder.exe -p uires -i uires\uieres.idx -r res\soui_res.rc2 -h resource.h idtable</Command>
<Outputs>res\soui_res.rc2;resource.h</Outputs>
<Inputs>uiresbuilder.exe;uires\uieres.idx</Inputs>
```

***

## 常见问题

### Q1：resource.h 里 `R.id.xxx` 的值怎么来的？能手动改吗？

A：从 `0x00010000` 起，按 LAYOUT/SMENU/SMENUEX（以及 `buildId="1"` 的自定义 XML）内定义出现的先后顺序递增分配，先出现的整数 ID 先拿到小值。**不建议手动改**，修改 `uires.idx` / `<file>` 后重新运行工具即可。真需要稳定值的，把 `buildId` 标记的文件固定顺序运行在同一台机器即可。

### Q2：`ParseLayoutFile` / `ParseUIDefFile` 扫描 XML 时抛错怎么办？

A：可能是 XML 本身不合法（`key`/`name`/`id` 有非 ASCII 字符且不带 UTF-8 BOM，或 XML 实体未闭合）。用 `xmllint` 或任何 XML validator 校验源文件。非 UTF-8 文件需**带 BOM 保存为 UTF-8**（与 SOUI4 工程约定一致）。Android/HarmonyOS 侧 assets 内的 XML 必须是 UTF-8 编码。

### Q3：`R.name.xxx` 和 `R.id.xxx` 的值一定对得上吗？

A：是的，二者都从同一次扫描的 `map<wstring,int> mapNameID` 生成：key = name 字符串、value = 分配到的 int。只要 `idtable` 开启且无文件改动，两次构建完全一致。

### Q4：R.js 导出的 arrName / arrId 用途？

A：在 JS 端做**双向映射**（int id ⇄ string name）时直接按 pair 遍历，不需要重复构建 `Object.entries(R.id)`。例如鸿蒙侧 `SouiPlatformBridge.ets` 的 `notifyFocusGained(hwnd)` 需要知道当前焦点是按字符串名还是按 int id 订阅，直接查 arr 构造反向字典即可。

### Q5：`-j` 导出的 ESM 模块，在 OpenHarmony ArkTS 里导入不了？

A：HarmonyOS API 12+ 的 ArkTS 默认支持 `import { R } from './R.js'`，如仍报错请检查 ets 的 `build-profile.json5` 中的 `etsLoader.clean` / `strictMode`。或者把 `R.js` 后缀改 `.ts` 并把首行 `export const R = {...}` 保留即可（生成内容是严格的 TS 子集）。

### Q6：Linux / macOS 构建找不到 `GetLastWriteTime`？

A：非 Win32 下代码使用 POSIX `stat()` + `mtime` 实现，编译时会自动走 `#ifndef _WIN32` 分支，无需额外配置。UTF-8 ↔ UTF-16 转换辅助函数已内置（`utf8_to_unicode` / `unicode_to_utf8`）。

### Q7：可以把 `resource.h` / `soui_res.rc2` 提交到仓库吗？

A：可以，但不推荐。推荐做法是把它们设为 `.gitignore` 忽略，由构建机在 `PRE_BUILD` 阶段自动生成。**如果要提交，请务必提交后不要手改**，否则下次构建会被无差别覆盖掉（顶部有"该文件由 uiresbuilder 生成，请勿手动修改"字样）。

***

## 依赖 / 许可

- `tinyxml`（Lee Thomason，zlib 风格许可）：解析 `uires.idx`、布局 XML、UIDef 文件。源码置于 `tinyxml/` 子目录。

- `XGetopt`（Linux/Unix 风格 getopt 移植版）：命令行参数解析，`XGetopt.cpp/.h`。

- 其余为 `stdafx` 预编译头与 Win32/Linux 公共 C 运行时，无第三方库。

