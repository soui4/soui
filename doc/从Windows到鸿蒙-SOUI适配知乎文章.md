# 从 Windows 到鸿蒙：一个 C++ DirectUI 框架是怎么吃下五个平台的

> 当"自主可控"从口号变成刚需，一个老问题再次摆在国内 C++ 客户端团队面前：我们的界面代码，能不能也跑在鸿蒙上？

## 一、为什么是现在

鸿蒙（HarmonyOS / OHOS）的本体语言是 ArkTS，UI 体系是 ArkUI。这与 Qt 的 C++、Flutter 的 Dart 都不在同一个世界。对于大量沉淀了十年以上 C++ 界面逻辑的项目——棋牌、工业控制、专业工具软件——"重新用 ArkTS 写一遍"几乎等于重写整个产品。

跨平台 UI 框架是唯一的出路。但现实是：主流方案里，Flutter 有社区驱动的 OHOS 分支、Qt 有商业授权版本，而**纯 C++ 的 DirectUI 框架，在鸿蒙上几乎是一片空白**。

SOUI 决定填上这块空白。

## 二、SOUI 是什么，以及它的"全平台版图"

SOUI 是国内起步很早（2014 年前后）的开源 C++ DirectUI 框架，核心思想是"用 XML 描述界面、用 C++ 写逻辑、自绘渲染不依赖系统主题"。在补齐鸿蒙之前，它已经跑在四个平台上：

- **Windows**：原生 GDI/Skia 后端，根基
- **macOS / iOS**：通过 `swinx` 平台层对接 UIKit 与 Core Graphics（Objective-C++）
- **Android**：通过 JNI 桥接，Bitmap + Skia 渲染

鸿蒙是第五块拼图。补齐之后，一份 SWinx C++ 业务代码可以构建出 **Windows / macOS / iOS / Android / OHOS** 五个端。据公开信息，在同类开源 C++ UI 框架中，SOUI 是**率先完成鸿蒙适配、形成五端统一代码库**的框架。

这不只是"能跑"，而是"用同一套抽象、同一套消息循环、同一套脏矩形机制"跑起来。

## 三、适配鸿蒙到底难在哪

鸿蒙和 C++ 之间隔着三道坎：

**1. 语言鸿沟。** ArkTS 是带 GC 的类型化语言，C++ 不能直接持有 ArkTS 对象。双方只能通过一个叫 **N-API** 的 C 接口对话。

**2. 线程铁律。** `napi_env` 只在 JS 线程上有效。任何从工作线程直接调用 N-API 的行为都会立刻崩溃——这不是偶发 bug，是设计使然。我们在接入定时器时，就踩过这个真实的坑（见下文）。

**3. 没有 HDC。** 传统 Windows 思维里"拿到一个设备上下文 HDC 就能画"，鸿蒙没有这种东西。渲染必须走 `PixelMap`（位图对象），自己管理离屏缓冲。

## 四、架构：三层桥接

SOUI for OHOS 的落地代码在 `soui-ohos-lib`，采用与 Android 同构的三层结构：

```
ArkTS / ArkUI 层      原生容器组件，承载 Surface、转发输入
        ↕  N-API
C++ 桥接层            soui-ohos-lib：把 ArkTS 方法导出为 C 函数
        ↕
SWinx 渲染层          与 Windows/macOS/iOS 同源的 SWinx 抽象
```

关键是：**上层的 SWinx 渲染层与 Windows 端几乎一致**。平台差异被压缩在桥接层，业务代码一行不用改。

## 五、三个最值得讲的设计

### 1. HWND = napi_ref 指针，零查表

在 SOUI 里，窗口句柄 `HWND` 是贯穿全局的标识。在鸿蒙上，它不能直接是 ArkTS 对象（GC 会回收），也不能是字符串 ID（每次回调都要查表，慢且易错）。

做法：创建窗口时，用 `napi_create_reference` 把 ArkTS 窗口对象"固持"成一个 `napi_ref`，再把 `ref` 的指针地址当作 `HWND` 返回给 C++。回调时 `napi_get_reference_value` 取回对象即可。**没有哈希表、没有字符串比对，一次指针解引用搞定。**

### 2. 渲染：PixelMap 离屏 + 脏矩形回传

鸿蒙每帧全屏提交成本不低。SOUI 的做法是：

- 在 `PixelMap` 上开一块离屏缓冲，SOUI 把界面画上去；
- 渲染完成后，C++ 侧 `GetUpdateRgn / GetRgnBox` 算出真正的更新矩形 `rcUpdate`；
- 通过 `surf_render` 把这个 `[left, top, right, bottom]` 回传给 ArkTS；
- ArkUI 只 flush 这一小块区域。

更新矩形由 C++ 侧作为**单一数据源**产出，TS 层不再自己维护脏矩形列表。这避免了"两边各记一份脏区导致对不齐、画面撕裂"的经典问题。

### 3. 跨线程定时器：先判线程，再决定是否跨线程

这是实打实修过的线上崩溃。

`setTimer` 大多发生在主线程，但 SOUI 内部有工作线程也会申请定时器。原来的实现一律走跨线程投递，主线程场景平白多了一份 `BridgeArg` 构造与结果回传的开销；更糟的是，如果调用线程直接持有了 N-API 环境又不小心跨线程用，会崩。

重构后的逻辑非常清晰：

```cpp
DWORD_PTR OhosPlatformAPI::setTimer(...) {
    if (isJsThread()) {
        // 主线程快路径：直接 callBridge 调用 ArkTS 的 setTimer
        return callBridge(...);
    }
    // 工作线程才走跨线程：napi_threadsafe_function 投递回 JS 线程
    return invokeBridge(...);
}
```

主线程零跨线程开销，工作线程靠 `napi_threadsafe_function` 安全地把调用"送回"JS 线程执行。同时，本地 `m_timerEntries` 的记录/擦除都放在调用**之后**加锁，避免持锁等待 JS 线程引发 `onTimerExpired` 死锁。

## 六、统一的不只是渲染

为了让五端行为一致，SOUI 在鸿蒙上还对齐了一组"平台无关"的语义：

- **SetCapture**：鼠标/触摸捕获用统一的 `SetCapture`，屏蔽 ArkTS 事件模型差异；
- **KeyCode 映射**：鸿蒙键码 2000+ 映射回 `VK_*` 体系，键盘逻辑跨端通用；
- **screenId 激活栈**：对应鸿蒙的多 Ability / 多窗口，SOUI 的 `screenId` 激活栈原样复用；
- **String Slot / 剪贴板 / 键盘高度**：统一封装，业务无感。

输入、剪贴板、生命周期——这些在 Android/iOS 上各自为政的东西，在 SOUI 里被收敛成同一组接口。

## 七、实战：中国象棋跑在鸿蒙上

`soui4` 仓库里的 `cnchess`（中国象棋）是最好的验证样本。它的客户端 `games/cnchess/client` 是**同一份 C++ 代码**，通过入口差异覆盖五端：

- 桌面走 `int main → _tWinMain`；
- iOS 走 `int main → swinx_ios_entry(...)`；
- Android / OHOS 因 JNI / N-API 入口必须落在桥接模块内，才有单独的 `android_entry.cc` / `ohos_entry.cc`。

鸿蒙端用 `soui-ohos-lib` 接入后，联网对战通过 libwebsockets 跑通（其间还因 `module.json5` 缺 `ohos.permission.INTERNET` 权限踩过 EPERM 的坑——又一个鸿蒙特有细节）。最终，这盘棋在手机上用 ArkUI 容器、SOUI 自绘、脏矩形局部刷新，流畅运行。

## 八、这意味着什么

把鸿蒙纳入版图后，SOUI 成为**首个以五端统一 C++ 代码库支持 HarmonyOS 的开源 UI 框架**。对存量 C++ 客户端团队，它的价值很直接：

- 已有 Windows / Android 产品，几乎零改动能长出鸿蒙版；
- 不绑定单一商业授权，代码自主可控、可审计；
- 渲染、消息、输入全自绘，不依赖系统主题，多端视觉一致。

在"国产化替代"的语境下，这或许比技术本身更重要：你的核心界面资产，终于不用为了上鸿蒙而推倒重来。

## 九、写在最后

SOUI for OHOS 不是把框架"移植"到一个新平台，而是把鸿蒙**收敛**进了既有的跨平台抽象里。这正是 SOUI 十年来一以贯之的思路——用一层薄而严谨的桥接，让业务代码忘记自己身在何处。

五端已齐。下一步，是把这套机制在更多真机、更多形态（折叠屏、车机、穿戴）上磨稳。

> 技术细节与完整架构对比，见仓库 `doc/SOUI-for-Android-适配方案技术文档.md`。
>
> SOUI 已在 Gitee 与 GitHub 开源：
> - Gitee：https://gitee.com/setoutsoft/soui4
> - GitHub：https://github.com/soui4/soui
>
> 欢迎 star、提 issue，一起把鸿蒙适配磨得更稳。
