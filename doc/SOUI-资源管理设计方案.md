# SOUI4 资源管理设计方案

> **文档定位**：本文是 SOUI4 **资源子系统**的实现级设计文档，承载机制、数据结构、逐条源码证据与测试覆盖边界。总体评价与设计层价值见 `doc/SOUI4项目评估报告-2026-10-14.md` §3.4；本文与评估报告分工是——**报告只回答"这个设计值多少"，本文回答"它具体怎么工作、边界在哪里"**。
>
> **实测基线**：主仓 HEAD `889f2d6c8`，`swinx` 子模块 gitlink `7d91784`（1.8）；文中行号与计数均可用所列命令复核。
>
> **相关既有专篇**（不在此重复）：`doc/cross-platform-resource-compilation.md`（371 行，`.rc`→COFF→ELF/Mach-O）、`tools/src/uiresbuilder/README.md`（382 行，三产物与 ID 稳定性）、`doc/SOUI 系统资源图片说明.md`（421 行，内置皮肤图鉴）、`swinx/doc/跨平台rc编译及资源管理.md`（409 行）、`doc/SOUI中使用SVG.md`（256 行）、`doc/SOUI5-Android开发入门指南.md:396-425`、`doc/SOUI5-鸿蒙开发入门指南.md:360-371`。

---

## 一、设计目标与切分依据

资源子系统要同时回答四个问题，SOUI 的答案是**把它们分到三层，彼此不越界**：

| 问题               | 归属层              | 不做什么                                   |
| ---------------- | ----------------- | -------------------------------------- |
| **从哪里取字节**        | 资源提供器 `IResProvider` | 不知道谁覆盖谁，也不认识"皮肤/样式"这类 UI 概念            |
| **谁覆盖谁**         | 提供器管理器 `SResProviderMgr` | 不解析内容，只做定位、派发与句柄缓存                     |
| **取到之后怎么用**      | UI 定义池 `SUiDef` + `IUiDefInfo` | 不接触文件与归档，只把"命名值"变成运行时对象（skin/style/color…） |
| **怎么找到它**        | 贯穿三层的定位键 `SResID`  | 不用数值 ID、不用语言 ID（见 §三）                    |

这个切分的直接收益是：**换打包形态不改代码，换皮肤/语言/主题不改 C++**。开发者写的是 `LOADXML("layout:main")`、`GETSKIN(_skin.sys.btn.normal)`，底层是 PE 资源段、ZIP、裸目录还是 APK assets，由应用启动配置决定，控件与业务代码对此无感。

与常见框架的差别，用一句话概括各自的定位键：

| 框架                | 运行期定位键                        | 覆盖机制                                  | 后果                                    |
| ------------------ | ------------------------------ | ------------------------------------- | ------------------------------------- |
| **SOUI4**          | `类型串:逻辑名`（字符串，小写化后哈希）         | **提供器链尾优先**，后挂载盖住先挂载                  | 同一份索引服务八种打包形态；主题包/热更目录是框架能力          |
| Qt（`.qrc`）        | 资源路径 `:/prefix/name`           | 无内建覆盖层（需自绘 QSettings/搜索路径）             | 资源必须在构建期进 `qrc`；运行期替换需另建文件搜索路径机制    |
| Android           | **数值 ID** `R.layout.main`      | 资源限定符目录（`-night`/`-zh`/`-xxhdpi`）由系统选择 | ID 随构建变化，跨模块引用需 R 类；热更只能落到文件路径      |
| Flutter           | 包内路径字符串 `assets/x.png`         | 无内建覆盖层                               | 资源清单在 `pubspec.yaml` 编译期固定            |
| Electron          | 文件系统路径（常封在 asar 归档里）          | 路径 + 自定义 fallback                     | 依赖 Node fs 语义；覆盖逻辑由应用自己写              |
| WPF               | Pack URI + 程序集资源              | 无内建覆盖层（`ThemeDictionary` 提供主题字典切换）    | 主题切换依赖 `Themes` 字典与程序集边界              |

SOUI 选的字符串键路线，代价与收益都写在 §六、§七。

---

## 二、三层结构与各自职责

| 层                | 类型 / 接口                                                    | 位置                                                                        | 职责                                                                                                     |
| ---------------- | ---------------------------------------------------------- | ------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------ |
| **资源提供器**（从哪里取字节） | `IResProvider : IObjRef`                                   | `SOUI/include/interface/SResProvider-i.h`（180 行）                            | `Init` / `HasResource` / `LoadIcon` / `LoadBitmap` / `LoadCursor` / `LoadImage` / `LoadImgX` / `GetRawBufferSize` / `GetRawBuffer` / `EnumResource` / `EnumFile`（`:90-175`） |
| **提供器管理器**（谁覆盖谁）  | `SResProviderMgr`                                          | `SOUI/include/res.mgr/SResProviderMgr.h`（227 行）+ `SOUI/src/res.mgr/SResProviderMgr.cpp`（486 行） | 按挂载顺序组织多提供器并做覆盖式查找；光标句柄缓存；`RemoveAll` 时释放                                                             |
| **UI 定义池**（取到之后怎么用） | `SUiDef` + `IUiDefInfo`                                    | `SOUI/include/res.mgr/SUiDef.h`（377 行）+ `SOUI/src/res.mgr/SUiDef.cpp`（719 行） | 把资源里的"命名值"解析成运行时对象：skin / style / template / gradient / color / string / dimension / font / objattr 九域（`UDI_*` 位，`SUiDef.h:19-27`） |

主干规模：`SOUI/{include,src}/res.mgr/` = 13 头 + 12 cpp / **5,651 行**；资源相关接口 5 个 = **732 行**；皮肤 / 样式 / 尺寸核心（`SSkin.h` 1,478 + `SSkin.cpp` 2,037 + `SSkinObjBase` + `SWndStyle.h` 273 + `SLayoutSize` 358）= **4,846 行**；布局管理器 `SOUI/{src,include}/layout/` = **6,444 行**；ZIP + 7ZIP 两组件 = **4,041 行**。

---

## 三、资源定位键：类型串 + 逻辑名，无语言 ID、无数值 ID

- `SResID`（`SOUI/include/helper/SResID.h:15-42`）= `szType[MAX_RES_TYPE=30]` + `szName[MAX_RES_NAME=100]`，**构造时对两者都 `_tcslwr` 小写化**（`:28-30`）；哈希为 Horner `(h<<5)+h+c`，先 type 后 name（`:56-74`）。
- 直接推论：`uires.idx` 里标签大小写无关（`IMG` 与 `img` 同族），`layout:` 与 `LAYOUT:` 等价；**语言不是定位键的一部分**（多语言另走 `tr` 表，§六）。
- 查找是**覆盖式**的：`SResProviderMgr::GetMatchResProvider`（`SOUI/src/res.mgr/SResProviderMgr.cpp:45-61`）从链表**尾部**向前遍历，`HasResource` 命中即返回——**后挂载的提供器优先于先挂载的**。这正是主题包 / 热更目录能"盖住"内置资源的机制，也是 `demos/fun_test/test_soui.cpp:433-461` 演示的三提供器叠挂语义（目录 → 再叠 ZIP，ZIP 生效）。
- 逻辑名到资源由 `SApplication::ParseResID`（`SOUI/src/SApp.cpp:590-597`）解析为 `type:name` 二段式；宏 `LOADXML` / `LOADIMAGE` / `LOADICON` / `STR2ID` 定义在 `SOUI/include/SApp.h:33-39`。

---

## 四、打包形态：8 种资源描述 × 6 类提供器 × 六端映射

`SAppCfg` 的 `ResType` 有 **8 个值**（`SOUI/include/SAppCfg.h:33-43`：ResFile / PeHandle / PeFile / ZipFile / 7zFile / AndroidAsset / OhosRawFile / Unknown），系统资源与应用资源各有一组 setter（`:70-86`，实现 `:278-353`）。内核内置 **4 类**提供器（`BUILTIN_RESTYPE`，`SResProvider-i.h:44-50`：PE / 目录 / Android asset / OHOS rawfile），ZIP 与 7ZIP 以 COM 组件插入（`components/commgr2.h:189`、`:199`）。

| 形态                   | 提供器                                                                                             | 落地方式                                                                                                                                                                                        | 平台                       |
| -------------------- | ----------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------ |
| PE / ELF / Mach-O 内嵌 | `SResProviderPE`（`SOUI/include/res.mgr/SResProvider.h:43`；实现 `SOUI/src/res.mgr/SResProvider.cpp:54-267`） | 复用 Windows 资源 API：`swinx/include/resapi.h`（191 行）+ `swinx/src/resapi.cpp`（479 行）以 `dlsym` 取 `_binary_soui_coff_o_start/_end`（`:102-108`），`swinx/src/coffparser.cpp`（667 行）按"类型→名称→语言"三级目录解析 `.rsrc`——即评估报告 §2.8 的编译产物 | Windows / Linux / macOS / OHOS 裸机 |
| 目录                   | `SResProviderFiles`（同头 `:228`；实现 `:351-531`）                                                     | `Init` 时把 `uires.idx` **一次性全量装表**进 `m_mapFiles`（`SResProvider.cpp:431-475`；成员声明 `SResProvider.h:340`），之后只做哈希查表                                                                                    | 全平台（开发期 / 未打包发布）            |
| ZIP                  | `components/resprovider-zip`（提供器 56 + 292 行，归档层 808 + 199 行，图标解码 772 行）                        | 从包内 `m_childDir + "uires.idx"` 建索引（`SResProviderZip.cpp:219-244`）；支持密码，支持文件 / 内存 / PE 资源三种来源（`ZIPRES_PARAM`，`zipresprovider-param.h:8-26`）                                                     | 全平台（桌面发布包 / 主题包）            |
| 7ZIP                 | `components/resprovider-7zip`                                                                    | `Init(hInstance, …)` 分支**仅 `_WIN32` 有效**，其它平台直接返回 `FALSE`（`SResProvider7Zip.cpp:112-120`）——"从 PE 资源里读 7z"尚未跨端                                                                                      | Windows（+ 文件形态）          |
| APK assets           | `SResProviderAndroidAsset`（116 + 324 行，整文件 `#ifdef __ANDROID__`）                              | `wParam = AAssetManager*`；`prefix/uires.idx` 必须存在；`path="uidef\init.xml"` 归一化为 `uidef/init.xml`（`.h:1-45`、`.cpp:86-105`）；asset 由 JVM 持有、提供器**不拥有**                                              | Android                  |
| HAP rawfile          | `SResProviderOhosRawFile`（120 + 333 行，`#ifdef __OHOS__`）                                        | 经 `OH_ResourceManager_InitNativeResourceManager` 取 `NativeResourceManager*`（`.h:44-49`），`OpenRawFile` / `CloseRawFile` 配对（`.cpp:105`）                                                             | OpenHarmony              |

> ⚠️ **iOS 没有专用提供器**：`ResType` 无 iOS 项，`SouiFactory::CreateResProvider`（`SOUI/src/SouiFactory.cpp:64-90`）只 switch 到上述四类。iOS 上资源只能走 `RES_FILE`（bundle 目录 + 手写 `uires.idx`）或 `RES_PE`（Mach-O 经评估报告 §2.8 链入），本仓**无 iOS 资源示例**可佐证（评估报告 §9.2 风险 13）。
>
> **一处实现级缺陷**：`SOUI/include/SAppCfg.h:190` 的 `Zip7File(&param, m_renderFactory, _T("uires.zip"), pszZipPwd)` 把包名写死、**忽略了入参 `pszZipFile`**（对比 `LoadResFromZip` `:202` 正确传参），即 `SetSysRes7z(path)` 目前不加载调用方指定的文件。ZIP 路径不受影响。

---

## 五、资源索引 `uires.idx`：手写 XML，一次描述、多处消费

- 全仓 **42 个** `uires.idx`，索引条目合计 **1,182** 条 `<file>`，归一化后 **25** 种类型标签（`layout` / `xml` / `img` / `image` / `svg` / `icon` / `cursor` / `gif` / `png` / `apng` / `anim` / `animator` / `valueani` / `skinxml` / `smenu` / `smenuex` / `lang` / `translator` / `values` / `lua` / `rtf` / `bkimg` / `imgtraymenu` / `uidef` 等）。样例：`demos/demo/uires/uires.idx`（266 行 / 214 条 / 21 种标签）。
- **没有任何工具生成它**：它由开发者手工维护；`tools/src/uiresImporter`（239 行）只做**重建 / 追加**（读入 → 备份 `uires.bak.idx` → 回写 → 把新条目追加进 `values/skin.xml`），移动端文档明确写"需手工维护、不会自动扫描目录"（`doc/SOUI5-Android开发入门指南.md:401-425`、`doc/SOUI5-鸿蒙开发入门指南.md:364`）。
- 同一份索引的**离线消费者**：`tools/src/uiresbuilder`（自有代码 1,493 行，其中 `residbuilder.cpp` 1,222 行）生成 `.rc2` / `resource.h` / `R.js` 三产物供桌面三端编译期使用——ID 起点 `KStartID = 0x00010000`（`residbuilder.cpp:49`），按资源文件时间戳做**增量 ID 稳定**（`:939-952`），内容未变则不重写文件（`:379`，`.rc2` 首行 `//stamp:`）；另有 `tools/src/xml2bin` 与 `tools/src/xmlsnap`（564 行）消费同一索引。产物样本：`demos/demo/res/soui_res.rc2`（219 行，`DEFINE_UIRES(name, type, file_path)` 声明式表）、`demos/demo/res/resource.h`（2,315 行，按类型嵌套的 C++ 常量与结构体）。

> **设计取舍**：**运行期按"类型串 + 逻辑名"查找，数值 ID 只在桌面编译期存在**。这是同一份 `uires.idx` 能同时服务 Windows 资源段、ELF / Mach-O 内嵌段、APK assets、HAP rawfile 与裸目录的原因——数值 ID 一旦进入运行期查找键，移动端与目录形态就得各写一套。代价是拼写错误只能在运行期暴露（`SResProviderMgr::LoadImage` 为此区分"索引未找到"与"内容读取失败"两类日志，`SOUI/src/res.mgr/SResProviderMgr.cpp:225-264`）。

---

## 六、皮肤 / 样式 / 多语言：九域值池与 scale 派生

- 九个 `UDI_*` 域对应九类**命名值池**，对外入口是宏 `GETUIDEF` / `GETSTYLE` / `GETSKIN(p, scale)` / `GETBUILTINSKIN` / `GETCOLOR` / `GETSTRING` / `GETLAYOUTSIZE`（`SOUI/include/res.mgr/SUiDef.h:120-128`）。
- 值池底层是**排序数组 + 二分**而非哈希表：`SNamedValue`（`SOUI/include/res.mgr/SNamedValue.h:34-166`）`Init` 后 `qsort`（`:66`）、查询 `bsearch`（`:94`）、另有按下标 O(1) 的 `GetAt`（`:106-109`）。这与"资源池读多写少、初始化后不可变"的形态一致——写侧只有 `Merge`（`:119-137`，去重后再排序）。
- 皮肤按 **(名字, scale) 双键**缓存：`SSkinPool`（`SOUI/include/res.mgr/SSkinPool.h` 231 行 + `SOUI/src/res.mgr/SSkinPool.cpp` 236 行）的 `GetSkin`（`:163-224`）在找不到精确 scale 时搜索可整除的源皮肤并 `Scale()` 派生一张插回缓存（上限 `kMaxBuiltinScales = 20`，`:176-177`）；内置皮肤名 **43** 个（`SYS_SKIN` 枚举 `:21-74`）。皮肤对象另有**惰性加载**（`lazyLoad`）与 **SVG 栅格化缓存**（`cacheSvg` 默认 TRUE，`SOUI/include/core/SSkin.h:212-222` / `:247-248`，缓存键为逐状态尺寸 `m_szPerStateCache`），染色前保留位图备份。
- 多语言**不改资源键**：`EnableMultiLang(langResId, …)`（`SAppCfg.h:92`，实现 `:419-424`）→ `LoadTranslator` / `InstallTranslator`（`SOUI/src/SApp.cpp:625-656`），翻译发生在 **XML 解析期**（`SWindow::tr()`，`Swnd.cpp:1629-1634`），并顺带 `GETUIDEF->SetDefFontInfo()` 覆盖默认字体（`SApp.cpp:651`）与广播 `UM_SETLANGUAGE`（`:654`）——机制与实证见评估报告 §6.1。

---

## 七、XML 资源 → 控件树

从"一条 `layout:` 逻辑名"到"一棵可交互控件树"的完整路径（与 `doc/SOUI-事件体系设计方案.md` §六的订阅登记在同一条链上）：

1. **载入**：`LOADXML(doc, "layout:xxx")`（`SOUI/include/SApp.h:33`）→ `SApplication::_LoadXmlDocment`（`SOUI/src/SApp.cpp:558-573`：`GetRawBufferSize` + `GetRawBuffer` → `load_buffer(..., enc_auto)` → `SXmlDoc::GetErrDesc`）。
2. **建根**：`SHostWnd::OnCreate`（`SOUI/src/core/shostwnd.cpp:809-865`，`CreateRoot()` 在 `:832`）→ `OnLoadLayoutFromResourceID`（`:793-800`）→ `SWindow::InitFromXml`（`SOUI/src/core/Swnd.cpp:1637-1714`；`InitFromResId` `:1716-1723`）。
3. **展开子树**：`SWindow::CreateChildren`（`Swnd.cpp:1484-1602`）四条路径——`<include>` 递归（`:1493-1527`）、内联 `<template>` 定义（`:1531-1537`）、`t:` 命名空间模板实例化（先查局部再回退 `GETUIDEF->GetTemplateString`，`:1538-1586`；占位符替换在 `CreateFromTemplate` `:1465-1482`）、普通节点 `CreateChild`（`:1589`）。建完对新子窗补发 `UM_SETSCALE` / `UM_SETCOLORIZE`（`:1593-1600`），**保证后建子树自动继承当前缩放与染色**。
4. **名字 → 类**：`SObjectFactoryMgr::CreateObject`（`SOUI/src/core/SObjectFactory.cpp:79-89`）；工厂按对象名哈希（`SOUI/include/core/SObjectFactory.h:53-58`），注册名支持 `|` 分隔别名（`SObjectFactory.cpp:47-57`）；未知对象回退 + `SetSwndDefAttr`（`:106-139`）沿基类→派生类叠加默认属性，XML 的 `class` 属性优先。
5. **每窗口私有命名空间**：`SHostWnd::EnablePrivateUiDef`（`shostwnd.cpp:372-386`）与 `SHostWnd::InitFromXml`（`:388-407`）现场 `SUiDef::CreateUiDefInfo()` + `Init2(pNode, FALSE)`——窗口根的 `<uidef>` 只对本窗口可见，栈式 push/pop（`SUiDef.h:195` / `:203`），查找自栈尾向栈头。

---

## 八、缓存点清单与"重载"的真实边界

| 缓存             | 位置                                                                                                                              | 粒度             |
| -------------- | ------------------------------------------------------------------------------------------------------------------------------- | -------------- |
| 提供器文件映射        | `SResProviderFiles::m_mapFiles`（`SResProvider.h:340`）；ZIP 同构 `SResProviderZip.h:43`                                                 | 索引全量装表，条目级哈希   |
| 光标句柄           | `SResProviderMgr::m_mapCachedCursor`（`SResProviderMgr.h:215-216`），`RemoveAll` 逐个 `DestroyCursor`（`SResProviderMgr.cpp:36-42`）         | 按 (名, 尺寸)      |
| 皮肤             | `SSkinPool.cpp:163-224`                                                                                                         | (名字, scale)    |
| 皮肤位图 / SVG     | `SSkin.h:212-222`                                                                                                               | 单皮肤对象 × 逐状态尺寸 |
| 命名值池           | `SNamedValue.h:34-166`                                                                                                          | 每个 uidef 域     |
| 引用计数           | 全部资源对象走 `AddRef/Release`                                                                                                          | 对象级            |
| 调试期使用计数        | `#ifdef _DEBUG` 下 `m_mapResUsageCount`（`SResProviderMgr.h:220-223`）+ `CheckUsage` 日志 `"resource of [x] was not used."`（`SResProviderMgr.cpp:423-437`）；皮肤侧 `m_mapSkinUseCount` | 条目级，可发现"挂了没人用" |

**必须如实记录的边界**：框架**没有** dirty / version / 条目级失效机制，也**没有** `ReloadSkin` / `ReloadStyle` / `ReloadResource` / `ITheme` 这类重载 API（`SOUI/include/res.mgr/` 与 `SOUI/src/res.mgr/` 全文 grep 为 0）。可用的清除只有整池级——`ISkinPool::RemoveAll`（`SOUI/include/interface/SSkinPool-i.h:65`）、`IResProviderMgr::RemoveAll`（`SResProviderMgr.h:38`）、`SUiDef` 的栈析构。热更新因此是**既有应用层范式**而非框架承诺：

- **换肤**：`demos/demo/skin/SSkinLoader.cpp:22-40`——复用同一个 `RES_FILE` 提供器**重新 `Init` 新主题目录**（依赖 `SResProviderFiles::Init` 中对路径拼接的处理，`SOUI/src/res.mgr/SResProvider.cpp:459-462`）→ 私有皮肤池 `RemoveAll()` → `LOADXML` → `LoadSkins`。已打开窗口的刷新**不重载资源而是改属性**：广播注册消息（`g_dwSkinChangeMessage`，`demos/demo/skin/SetSkinWnd2.cpp:8-10`）→ `CMainDlg::OnSkinChangeMessage`（`demos/demo/MainDlg_Skin.cpp:322-331`）→ `ApplyThemeColors` 遍历控件树改 `style.m_crBg/m_crBorder` 后 `Invalidate()`，再 `pRoot->DoColorize()`（内核侧 `SWindow::DoColorize`，`SOUI/src/core/Swnd.cpp:4132`，广播 `UM_SETCOLORIZE`；新建子窗在 `CreateChildren` 末尾继承，`:1593-1600`）。
- **主题包热下载**：`games/cnchess/client/ThemeDownloadManager.cpp:557-652` 把 **ZIP 提供器当"归档读取器"**用（`CreateResProvider_ZIP` → `Init` → 两遍 `EnumFile` 统计与解压）落盘到 `theme_cache/`，运行期仍由 `RES_FILE` 提供器读目录——这是评估报告 §4.1"皮肤热换"的底层细节，也是 §7.2 跨线程下载完成后的交接点。

---

## 九、测试覆盖与缺口

**已覆盖**：`test_integration_resprovider.cpp`（33 行 / **全仓唯一的 `soui-integration` 用例**：带密码 ZIP 挂载 → 裸条目 `GetRawBufferSize/GetRawBuffer` → `SXmlDoc::LoadBuffer` → `LoadIcon(_T("ICON_LOGO"),64,64) != NULL`，一次穿过"归档→索引→XML→图标解码"四道交界）；`test_e2e_core.cpp`（3 条，注释 `:9-10` 自陈定位即"indexed UI resource → application resource manager → XML window factory → named controls → observable state change"，并含 `<template>` 实例化一条）；`test_e2e_inplace_edit.cpp`（7 条，夹具用 `InitFromResId` 建树）；`test_soui_core_unit.cpp` 的 `soui_layout_size.*`（3 条，`values/dim` 资源域）与 `soui_xml.*`（12 条，`SXmlDoc` 接口对象缓存重绑定）；`test_lua.cpp`（24 条，夹具挂 `RES_FILE` 以便 `anim:` / `valueAni:` 取字节）；`test_check_sync.cpp`（6 条，外部皮肤类注册）。

**未覆盖（不可声称）**：**PE / ELF / Mach-O 内嵌提供器**、**Android asset / OHOS rawfile 提供器**、**7ZIP 提供器**均无无头用例（`RES_PE` / `SetSysResPe*` / `FindResource` / `RES_ANDROID_ASSET` / `CreateResProvider_7ZIP` 在 `demos/fun_test/` 内命中为 0）；`SUiDef` 值池与 `SSkinPool` 的 scale 派生算法（`:163-224`）无断言覆盖；`uiresbuilder` 无自动化测试，三产物正确性靠人工比对。

> ⚠️ 文档漂移：`cross-platform-resource-compilation.md:17` 与 `uiresbuilder/README.md:21` 仍指向 `swinx/src/winres.cpp`——该文件**已不存在**（实现为 `resapi.cpp`）。

---

## 十、如果要改这块，需要知道的约束

1. **不要往运行期查找键里塞数值 ID**——一旦引入，八种打包形态里至少有三种（目录 / APK assets / HAP rawfile）需要另写一套定位逻辑，这是本节设计成立的地基。
2. **覆盖顺序即语义**：新增提供器时明确它应该盖住谁（尾优先），并把顺序写进启动配置的可读位置；换肤/热更能力都依赖这条。
3. **`addEvent` / `GetEventSet()` 一类的"能力先登记"模式同样适用于资源**：条目只有进索引才可见，运行期没有"扫描目录补索引"的路径（`SResProviderFiles::Init` 是唯一的装表点）。
4. **改动值池请沿用排序数组 + 二分**，不要改成每查询哈希——写侧只有 `Merge`，读多写少的形态是这个子系统的既有事实。
5. **补测试的优先顺序**：`SSkinPool` 的 scale 派生（无断言、逻辑最容易错）→ `SUiDef` 九域查找与私有 uidef 栈 → PE 提供器的跨端一致性（依赖 §2.8 的编译链，属集成层）。
