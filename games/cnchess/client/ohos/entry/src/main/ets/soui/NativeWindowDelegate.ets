//
// NativeWindowDelegate.ets
//
// INativeWindow 接口的公共实现委托类，对应 Android 端 NativeWindowDelegate.java。
//
// 职责：
//   - 将窗口状态操作逻辑（nativeShow/nativeMove/nativeSetSize/nativeSetPosition/
//     nativeEnable/nativeIsVisible/nativeIsEnabled/nativeDestroy/nativeInvalidate）
//     集中在一处，让 SouiSurface 和 NativeEditView 共享同一套行为
//   - 简化 INativeWindow 实现类的代码冗余
//
// OHOS 与 Android 的差异：
//   - Android 中 View 有 parent SouiAbsLayout 概念，delegate 需要 resolveTarget
//     区分操作 Surface 自身还是操作容器（SouiWindow）
//   - OHOS 中窗口直接是 INativeWindow（Surface 或 Edit），没有"容器 + Surface"分层，
//     delegate 直接操作 INativeWindow 自身即可
//
// 使用方式：
//   - SouiSurface 构造时创建 mDelegate = new NativeWindowDelegate(this)
//   - nativeShow/nativeMove 等方法委托给 mDelegate
//   - 自定义行为（如 SouiSurface 的 dirty rect）可覆盖不委托的方法

import { INativeWindow } from './INativeWindow';
import { SouiPlatformBridge } from './SouiPlatformBridge';

//
// INativeWindow 的公共实现委托。包装一个 INativeWindow 实例，
// 提供默认的窗口状态操作实现。SouiSurface / NativeEditView 可使用此类
// 减少代码重复，对齐 Android NativeWindowDelegate 的设计。
export class NativeWindowDelegate {
  // 被委托的 INativeWindow 实例（SouiSurface 或 NativeEditView） 
  private readonly mTarget: INativeWindow;

  constructor(target: INativeWindow) {
    if (target === null || target === undefined) {
      throw new Error('NativeWindowDelegate: target must not be null');
    }
    this.mTarget = target;
  }

  //
// 销毁窗口：清理 C++ 侧引用并从 SouiPlatformBridge 注销。
// 对齐 Android NativeWindowDelegate.nativeDestroy（从父容器移除）。
  nativeDestroy(): void {
    // SouiSurface / NativeEditView 自身已实现 nativeDestroy，
    // 这里只做清理映射的逻辑兜底（实际实现由具体类负责）
    const hwnd = this.mTarget.nativeGetHwnd();
    if (hwnd === 0) return;
    // 清理捕获/焦点（由 destroyNative 内部处理）
    SouiPlatformBridge.instance().destroyNative(hwnd);
  }

  //
// 请求 Surface 重绘。
// 对齐 Android NativeWindowDelegate.nativeInvalidate → View.invalidate(l,t,r,b)。
// 注意：SouiSurface 有 dirty rect 实现，NativeEditView 是 no-op（TextInput 自绘）。
// 实际实现由具体 INativeWindow 类提供。
  nativeInvalidate(left: number, top: number, right: number, bottom: number): void {
    this.mTarget.nativeInvalidate(left, top, right, bottom);
  }

  //
// 显示/隐藏窗口。
// 对齐 Android NativeWindowDelegate.nativeShow → View.setVisibility。
// OHOS 中通过 @Observed visible 属性触发 UI 更新。
  nativeShow(cmdShow: number): boolean {
    // 委托给具体类（SouiSurface / NativeEditView 已实现 visible 属性更新）
    return this.mTarget.nativeShow(cmdShow);
  }

  //
// 移动并调整窗口大小。
// 对齐 Android NativeWindowDelegate.nativeMove → SouiAbsLayout.updateChildFrame。
// OHOS 中通过 @Observed x/y/width/height 属性触发 UI 更新。
  nativeMove(x: number, y: number, width: number, height: number): boolean {
    return this.mTarget.nativeMove(x, y, width, height);
  }

  //
// 设置窗口大小。
// 对齐 Android NativeWindowDelegate.nativeSetSize → SouiAbsLayout.setChildSize。
  nativeSetSize(cx: number, cy: number): boolean {
    return this.mTarget.nativeSetSize(cx, cy);
  }

  //
// 设置窗口位置。
// 对齐 Android NativeWindowDelegate.nativeSetPosition → SouiAbsLayout.setChildPosition。
  nativeSetPosition(x: number, y: number): boolean {
    return this.mTarget.nativeSetPosition(x, y);
  }

  // 窗口是否可见。 
  nativeIsVisible(): boolean {
    return this.mTarget.nativeIsVisible();
  }

  // 设置窗口 enabled 状态。 
  nativeEnable(enabled: boolean): boolean {
    return this.mTarget.nativeEnable(enabled);
  }

  // 窗口是否 enabled。 
  nativeIsEnabled(): boolean {
    return this.mTarget.nativeIsEnabled();
  }

  // 获取窗口信息（保留占位，对应 Win32 GetWindow）。 
  nativeGetWindow(hWnd: number, code: number): number {
    return this.mTarget.nativeGetWindow(hWnd, code);
  }
}
