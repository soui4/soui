//
// INativeWindow.ets
//
// 抽象接口，对应 Android 端 INativeWindow.java。
//
// 设计契约：HWND = INativeWindow 的 N-API 全局引用（napi_ref）指针值，一一对应。
//   ArkTS 侧：SouiPlatformBridge.createWindow(...) 返回一个 INativeWindow 实例，
//   该对象持有 napi_ref 指向 C++ _Window 的句柄（等价于 Android 中 jobject=GlobalRef=HWND）。
//
// C++ → ArkTS 回调查找链路：
//   OhosPlatformAPI::hwndAsNativeWindow(hwnd)
//     → SouiPlatformBridge.hwndAsNativeWindow(hwnd)
//     → mWindowMap.get(hwnd) 返回对应 INativeWindow 实例
//
// ArkTS → C++ 事件链路：
//   INativeWindow 作为 Canvas 挂载点/输入焦点接收点，
//   接收 ArkTS 的触摸/按键事件后，调用 N-API 转发送给 swinx 消息循环。

export interface INativeWindow {
  // 获取 HWND（= napi_ref 指针值，用作 mWindowMap key）。 
  nativeGetHwnd(): number;

  // 销毁窗口：从父容器移除，并通知 C++ 释放 napi_ref。 
  nativeDestroy(): void;

  //
// 请求局部（或全部）重绘。全为 0 表示整体重绘。
// 对应 Win32 InvalidateRect。
  nativeInvalidate(left: number, top: number, right: number, bottom: number): void;

  //
// 显示/隐藏窗口。
// cmdShow 0 = 隐藏，其他值 = 显示
// 操作是否成功
  nativeShow(cmdShow: number): boolean;

  // 移动并改变大小（对应 Win32 MoveWindow）。 
  nativeMove(x: number, y: number, width: number, height: number): boolean;

  // 仅改尺寸（对应 Win32 SetWindowPos 带 SWP_NOMOVE）。 
  nativeSetSize(cx: number, cy: number): boolean;

  // 仅改位置（对应 Win32 SetWindowPos 带 SWP_NOSIZE）。 
  nativeSetPosition(x: number, y: number): boolean;

  // 当前是否可见（对应 Win32 IsWindowVisible）。 
  nativeIsVisible(): boolean;

  // 设置 enabled 状态（对应 Win32 EnableWindow）。 
  nativeEnable(enabled: boolean): boolean;

  // 当前是否 enabled（对应 Win32 IsWindowEnabled）。 
  nativeIsEnabled(): boolean;

  // 获取窗口相关信息（对应 Win32 GetWindow）。 
  nativeGetWindow(hWnd: number, code: number): number;

  //
// 通用消息转发。字符串参数通过字符串槽交换，纯数值消息按 Win32 语义传递。
// msg 消息号（WM_* / EM_*）
// wParam WPARAM
// lParam LPARAM
// LRESULT
  nativeSendMessage(msg: number, wParam: number, lParam: number): number;

  // 返回对应的 ArkUI 组件（用于挂到视觉树）。 
  asView(): object;

  //
// 请求 ArkUI 焦点到本控件（用于 IME 显隐驱动）。
// NativeEditView：聚焦 TextInput → 系统自动绑定 IME 并弹出键盘。
// SouiSurface：聚焦 Canvas → 配合 imc.attach 绑定自绘组件 IME。
  requestFocus(): void;
}
