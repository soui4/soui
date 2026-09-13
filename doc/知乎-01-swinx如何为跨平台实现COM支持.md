# 没有 MSVC 也没有 COM 运行时，怎么让跨平台代码用上 COM？

*（macOS / Linux 上没有 MSVC、没有 COM 运行时——swinx 却让 COM 在这上面仍然能正常使用。这篇聊聊它怎么做到的。）*

让一套 Windows 代码在 macOS / Linux 上跑起来，本来就够费劲了。更难的是，这代码里还带着一些**编译期魔法**——比如 `__uuidof`，以及它背后一整套 <unknwn.h>、<objbase.h> 的 COM 基础设施。

先弄清楚它到底在解决什么。swinx 要的**不是某个具体的 Windows API**，而是一个老程序员都熟的东西：**COM**。这是一套完整的接口约定与运行时——类型（`IUnknown` / `BSTR` / `VARIANT`）、接口宏、引用计数、`CoCreateInstance` 这种工厂。当 `swinx` 把这几样在 macOS / Linux 上重新立起来，上层代码就能像在 Windows 上一样写 COM，然后**一串实用的接口**——`IStream`、`IDataObject`、`IDropTarget / IDropSource`、`IDispatch`——顺着往上长。这篇文章从最硬的那个细节讲起：`__uuidof`。

gcc / clang 上**没有 `__uuidof` 这个关键词**、**没有 `__declspec(uuid)`**，更**没有 COM 运行时**。一句话：你想要的 COM，在这个环境里根本不存在。怎么办？

## 先分清，缺的是哪种"东西"

难题其实分两类，别混为一谈：

1. **语言特性的缺口**——`__uuidof` 这种"编译期映射"，平台不带。
2. **运行时的缺口**——`BSTR` / `VARIANT` / `IDispatch` 这些类型和函数，系统也不给。

这两类缺口的解决办法完全不同：一个是换种"写法"，一个是自己补一个"运行时"。我们一个一个说。

## 关键一：把编译期魔法，降级成一个普通的静态函数

MSVC 的 `__uuidof` 是什么？说穿了，它就是把"类型 → IID"的映射**在编译期写死**，让 `__uuidof(IStream)` 直接产出一个 IID 引用。

gcc / clang 没这能力。swinx 的做法很朴素——**把魔法降级成一次普通静态函数调用**：

```cpp
#define __suidof(ClassName)  ClassName::GetIID()
```

每个接口类内部用宏定义 `static const GUID &GetIID()`，于是：

- **调用点的写法跟 MSVC 一模一样**，源码零改写；
- 返回的是 `const GUID&`，和 MSVC 一致，参数类型天然对得上。

**魔法变成了一条平平无奇的函数。** 就这么点 trick，代码却能原样在 gcc / clang 上编译通过。代价只有一条约束：接口必须用配套的宏体系来声明。对上层来说，约等于没有代价。

## 关键二：自己造一个"够用"的 COM 地基

语言特性补上了，但 `BSTR`、`VARIANT`、`SysAllocString`、`IDispatch` 这些运行时也没有。swinx 不是去移植整个 OLE，而是**从零立起一整套 COM 地基**——把"任何一个 COM 程序都会用到"的那片基础能力补齐，剩下的交给上层按需实现接口：

| 缺的东西 | 补法 |
|---|---|
| GUID / IID 类型与比较 | 一个小头文件，自带类型；比较用 `memcmp` 比一个结构体 |
| `__uuidof` / `__declspec(uuid)` | 展开成 `X::GetIID()`（上面那条） |
| 接口声明宏（`DECLARE_INTERFACE_` / `STDMETHOD` …） | 一套 `basetyps.h` 宏体系 |
| `IUnknown` 引用计数 | `SUnkImpl<T>` 模板 |
| `BSTR` / `VARIANT` / `IDispatch` | 三个小实现文件 |

里面有个细节其实很漂亮：**GUID 弱符号**。头文件里定义 GUID 时用了 `__attribute__((weak))`，允许多个编译单元各带一份同名定义、链接器任选其一——这样在头文件里写 `DEFINE_GUID` 才不会到处报"重复定义"。平凡，但解决了一个真实大坑。

在这套地基之上，swinx 再按需实现了**一组有限的接口**——`IStream`、`IDataObject`、`IEnumFORMATETC`、`IDropTarget`、`IDropSource`、`IDragSourceHelper`、`IDispatch`……接口设定是"够用即止"，每个都只实现业务真要用的部分。地基是通用且完整的，接口是精选并有限的。

## 一整个"能用的 COM"

真正能代表这套东西的，不是某一个接口，而是**整个 COM 运行时**：

- `BSTR` 一族（`SysAllocString / Len / ByteLen / Free …`）
- `VARIANT` 与 `VariantInit / Clear / Copy`
- `SAFEARRAY`、`STGMEDIUM`、`FORMATETC`
- `CoCreateInstance` / `CoTaskMemAlloc` / `CoGetMalloc`
- `CoCreateGuid` / `CLSIDFromProgID` / `OleInitialize / OleUninitialize`

这些都是**从零实现**、不依赖任何 Windows 系统库的自足代码。在上面，实实在在跑着几个桌面开发天天用的场景：

| 接口 | 干的是什么事 |
|---|---|
| `IStream` | 一段能流式读写的文件/内存流 |
| `IDataObject` | 剪贴板与拖拽的**数据载体** |
| `IEnumFORMATETC` | 遍历一份数据支持的格式 |
| `IDropTarget` / `IDropSource` / `IDragSourceHelper` | "把文件拖进窗口""拖动时显示图标"这些老动作 |
| `IDispatch` | COM 的迟到绑定（自动化、脚本都靠它） |

## 同一接口，各平台落地

到这里，要说这套 COM 最有意思的地方了：这些接口**不是被"空转模拟"出来的**，而是被拿来当了一层**可移植抽象**。

举个最直观的例子——`IDataObject`。上面写着"剪贴板 / 拖拽的数据载体"，但在不同平台，同一份接口落到完全不同的机制上：

| 平台 | `IDataObject` 的落地 |
|---|---|
| Windows | OLE 剪贴板 |
| Linux | MIME + 剪贴板管理器 |
| macOS | `NSPasteboard` |
| iOS | `UIPasteboard` / UTI |
| Android | `ClipboardManager` |

上层 SOUI / 业务代码**只知道 `IDataObject` 一个接口**；至于底层是 MIME、是 NSPasteboard、还是 ClipboardManager，全被关在各平台各自的代理实现里（Linux 的 `SMimeData`、macOS 的 `SNsDataObjectProxy`、iOS 的 `SUIDataObjectProxy`……）。

这意味着它**不是"为了跑旧代码而倒退地仿真 Windows"**，而是借 COM 的接口约定，把平台能力抽象成一份统一声明。要多支持一个平台，只需在那个平台落地几个实现类，上层一行不改。

## 结果：一份代码，三平台，不因编译器而分裂

有了这套地基，上层代码能**严格按 MSVC 风格写 COM**：

```cpp
// 一个普通 COM 对象，引用计数、QI、IID 全由 SUnkImpl 撑起来
class MyStream : public SUnkImpl<IStream> { ... };

// 调用点写法与 Windows 上别无二致
pStream->QueryInterface(__suidof(IDataObject), (void**)&pObj);
```

在 Windows 走系统的 COM/OLE 运行时，在 macOS / Linux 上由 swinx 提供同一套 API 面。**上层代码不写一行 `#ifdef` 去迁就编译器差异。**

## 说到底

这套东西没造什么科幻，本质是：**把微软藏在编译器里的约定，用几层小头文件重新立起来**。好消息是 COM 本身并不玄——它就是一个"谁都会背"的接口约定。真去拆开，剩下的只是一堆被包装成魔法的线程细节。

真正的产出不是一个单独的接口，而是**一整套可用、可扩展的 COM 地基**。地基通用而完整，具体接口按需向上叠加——`IStream`、`IDataObject`、`IDropTarget / IDropSource`、`IDispatch`……无需为编译器或操作系统妥协，一套 Windows 代码在 macOS / Linux 上就能照常使用 COM——这层"COM 还能用"的能力，是它替所有跑在其上的框架埋好的一张底牌。

---

**SOUI4 项目主页：**

- Gitee：<https://gitee.com/setoutsoft/soui4>
- GitHub：<https://github.com/soui4/soui>