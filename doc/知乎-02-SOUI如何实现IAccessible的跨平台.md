# 一套基于 IAccessible（COM）的 Windows 无障碍代码，怎么做到零改写、支持Linux, MacOS?

*（无障碍逻辑全程用 COM 的 `IAccessible` 这套 Windows 接口书写；跨平台时，这套上层代码不需要为 macOS / Linux 修改。）*

DirectUI 框架对无障碍是"天然不利"的，原因很具体：普通 GUI 里每个按钮、每个列表都有系统原生窗口，屏幕阅读器（Windows 的 Narrator / NVDA、macOS 的 VoiceOver 等）能逐个识别；而 SOUI 是**全自绘**的，所有控件都画在同一个宿主窗口上，系统眼里只有"一个大空白窗口"，屏幕阅读器读不到任何内容。

同时，三个平台的无障碍协议各不相同：Windows 是 MSAA，macOS 是 NSAccessibility，Linux 是 AT-SPI2。

要做到三平台通吃，需要先回答一个问题：**一套按 MSAA（Windows 习惯）写出来的无障碍实现，凭什么在 macOS / Linux 上不改动就能被使用？** 下面从 SOUI 的分层设计上摊开讲。

## 先拆题：这是两个不同的问题

"给自绘框架做无障碍"包含两个彼此独立的问题：

1. **语义问题**：怎么把"控件节点"组织成无障碍工具能读的"可访问树"（名字、角色、状态、位置、父子关系）。
2. **协议问题**：怎么让这份可访问树同时能被 MSAA、NSAccessibility、AT-SPI2 三个协议访问。

SOUI 的做法是把两者分到不同层去解：语义用一个独立对象统一实现，协议交给平台各层桥接。下面按"自上而下"四步展开。

## 一、MSAA 语义统一收进一个门面类

先看一个备选方案，以及它的问题：让每个控件类直接实现 `IAccessible`。

- `IAccessible` 继承 `IDispatch`，每个实现都要补 `Invoke`、`GetIDsOfNames` 等纯 COM 方法，而这些与控件业务无关；
- MSAA 用 `child id` 寻址子元素，"id ↔ 控件"的映射一旦塞进控件，会与控制自身的数据结构（如列表的可见项窗口、Tab 的页签数组）纠缠；
- 三个平台（Windows 原生、macOS/Linux 桥）对 `IAccessible` 的访问路径不同，控件如果直接暴露 `IAccessible`，平台差异会随之进入控件层。

SOUI 换了一个方案：**引入门面类 `SAccessible`，把全部 MSAA 语义收在它一个类里**。child id 寻址、`CHILDID_SELF`、`VT_I4` 与 `VT_DISPATCH` 的取舍、`S_FALSE` 约定、`IDispatch` 转发——这些规则只在它这里实现一次。控件、平台桥、业务代码都通过它来访问，不在别处重复。

## 二、用一个扁平 child id，同时覆盖两类子元素

MSAA 对子元素的寻址是一个连续的整数（child id）。SOUI 的控件树里实际存在两类"子元素"：

- **真实子控件**：是一个独立的 `SWindow`，有自己的可访问对象；
- **自绘子项**：没有原生窗口（如 Tab 的页签头、表格列头、虚拟列表的可见项），共享父控件的可访问对象，在 MSAA 里通过"父对象 + child id"寻址。

SOUI 把一组连续的 child id 分成两段：

```
child id  1 ...... real          real+1 ...... total
          ├── 映射到真实子控件 ──┤├── 映射到自绘子项 ──┤
          │  各自有独立 IAccessible │  共享此对象的 IAccessible │
```

对任意一个窗口，`1..real` 是它的子控件，`real+1..total` 是它内部的自绘项。上层无障碍工具拿到的仍然只是一个普通 child id，但会命中两套不同的实现。这样页签、表格、虚拟列表这类"没有原生窗口却要可被读屏访问"的元素，也被纳入同一个寻址体系。

## 三、控件系统不依赖 MSAA：查询统一走代理

门面解决了"MSAA 语义只实现一次"，隔离则来自代理层 `IAccProxy`。

`SAccessible` 在拿到目标控件后，不直接访问其内部，而是调用 `pChild->GetAccProxy()->get_accXxx(...)`，把属性查询委托给一个代理对象 `IAccProxy`：

- `IAccProxy` 不涉及 MSAA——它没有 `VARIANT`、没有 `BSTR`，只回答"该控件的名字 / 角色 / 位置 / 是否选中"这类问题；
- 控件类到代理的映射由**中央工厂** `CreateAccProxy()` 集中分派。为一个新控件增加支持，只需新增一个代理类并在工厂注册一行；
- 控件基类上只挂 3 个最小接口点（获取可访问对象、获取代理、上报事件）。除此之外，`SButton`、`STabCtrl`、`SListView` 的源码里不出现 MSAA 相关词汇。

工程上还有一个结果：ACC 是可选构建项。由于语义集中在独立文件中，关闭它时这些文件整批不参与编译，控件系统自身不变；相比"语义写入控件"会在关闭后留下大量 `#ifdef` 与死代码，这里的隔离更干净。

补充两点，都与"复用既有结构"一致：
- `accNavigate` 的导航复用 SOUI 窗口树的兄弟遍历（`GSW_NEXTSIBLING` 等），可访问树与真实窗口树一致，无需维护第二份结构；
- 门面每个方法先检查 `m_pWnd` 是否仍有效，配合下文"平台桥按需查询、不缓存指针"的做法，避免"控件已销毁、指针悬垂"该问题。

## 四、跨平台：一套 MSAA 面，多个平台下层

跨平台部分之所以能简化，前提是前两步已把语义隔离到位。

SOUI 的可访问实现（`SAccessible` + `IAccProxy`）只写一份，调用标准 oleacc API、使用标准 MSAA 常量。随平台变化的是最下层：

```
      SAccessible + IAccProxy （一份代码，标准 MSAA 语义）
                               │
                    标准 oleacc API 面（同一套声明）
                               │
      ┌────────────────────────┼────────────────────────┐
      ▼                        ▼                        ▼
  Windows 原生            macOS 桥                 Linux 桥
  oleacc.dll           NSAccessibility          AT-SPI2 / D-Bus
```

- Windows 使用系统自带的 `oleacc.dll`；
- macOS / Linux 由 swinx（见另一篇对 COM 兼容层的介绍）提供同一组 oleacc API，再用一个小桥把 MSAA 语义翻译成 `NSAccessibility` / `AT-SPI2`。

由此，"跨平台"被拆成两个边界清晰的子问题：

- **语义**：`SAccessible` 只认 MSAA，一份代码三平台复用，不写平台相关的分支；
- **协议**：各平台的翻译收在各自的小桥里，互不影响。

macOS / Linux 采用对象树寻址，与 MSAA 的"句柄 + child id"不同，桥按需向窗口重新获取对象、不缓存接口指针——这既保证语义取到的是当前对象，也避免悬垂指针。

## 小结

把这四层的职责归纳如下：

| 层次 | 承担什么 | 是否随平台变化 |
|---|---|---|
| `SAccessible` 门面 | MSAA 的全部语义 | 否（三平台共用） |
| `IAccProxy` 隔离层 | 控件语义 ↔ 可访问数据的转换 | 否 |
| oleacc API 面 | 统一的进程内 MSAA 接口 | 提供方随平台不同 |
| 平台桥 | MSAA → NSAccessibility / AT-SPI2 翻译 | 是，各平台独立 |

结论：**一套基于 `IAccessible` 编写、只依赖 MSAA 语义的上层代码，只要协议翻译被收在平台桥下层，就能在不修改上层实现的情况下获得三平台支持。** SOUI 通过把"语义"与"协议"分别锁定在不同层，实现了这一点——上层只关心 MSAA，平台差异全部落在桥与运行时上。

---

**SOUI4 项目主页：**

- Gitee：<https://gitee.com/setoutsoft/soui4>
- GitHub：<https://github.com/soui4/soui>