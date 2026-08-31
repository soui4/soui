//
// SouiAbsLayout.ets
//
// 绝对布局引擎，对应 Android 端 SouiAbsLayout.java。
//
// 职责：
//   - 为子窗口提供绝对定位引擎：通过 (x, y, width, height) LayoutParams 定位
//   - 提供 updateChildFrame / setChildPosition / setChildSize 等方法
//   - SouiScreen 可使用此类集中管理子窗口位置和尺寸
//
// OHOS 与 Android 的差异：
//   - Android 中 SouiAbsLayout 继承 ViewGroup，通过 onMeasure/onLayout 实际测量和布局子 View
//   - OHOS 中没有等价的 ViewGroup，子窗口通过 @Observed 属性 + .position() 修饰器定位
//   - 因此本类作为"逻辑布局引擎"，不直接渲染，而是更新 INativeWindow 的响应式属性
//
// 布局参数 LayoutParams：
//   - x, y：相对父容器左上角像素偏移
//   - width, height：尺寸（0 或负数表示 MATCH_PARENT / WRAP_CONTENT）
//
// 使用方式：
//   - SouiScreen 持有一个 SouiAbsLayout 实例
//   - createWindow 创建子窗口后，调用 updateChildFrame 设置初始位置和尺寸
//   - nativeMove / nativeSetSize / nativeSetPosition 委托给本类

import { INativeWindow } from './INativeWindow';

//
// 子窗口布局参数。对应 Android SouiAbsLayout.LayoutParams。
// (x, y) 相对父容器左上角像素偏移，width/height 遵循 Android 语义。
export class LayoutParams {
  // MATCH_PARENT 等价值：表示填满父容器 
  public static readonly MATCH_PARENT: number = -1;
  // WRAP_CONTENT 等价值：表示按内容自适应 
  public static readonly WRAP_CONTENT: number = -2;

  x: number = 0;
  y: number = 0;
  width: number = LayoutParams.WRAP_CONTENT;
  height: number = LayoutParams.WRAP_CONTENT;

  constructor();
  constructor(width: number, height: number);
  constructor(x: number, y: number, width: number, height: number);
  constructor(x?: number, y?: number, width?: number, height?: number) {
    if (x !== undefined && y !== undefined && width !== undefined && height !== undefined) {
      this.x = x;
      this.y = y;
      this.width = width;
      this.height = height;
    } else if (x !== undefined && y !== undefined) {
      // (width, height) 签名：x=width, y=height
      this.width = x;
      this.height = y;
    }
  }
}

//
// 绝对布局引擎。对应 Android SouiAbsLayout。
// 管理子窗口的 LayoutParams，通过更新 INativeWindow 的响应式属性实现定位。
export class SouiAbsLayout {
  // 子窗口 → LayoutParams 映射 
  private mChildParams: Map<number, LayoutParams> = new Map();

  //
// 获取或创建子窗口的 LayoutParams。
// child 子窗口
// 现有或新建的 LayoutParams
  public obtainChildLayoutParams(child: INativeWindow): LayoutParams {
    const hwnd = child.nativeGetHwnd();
    let lp = this.mChildParams.get(hwnd);
    if (!lp) {
      lp = new LayoutParams();
      this.mChildParams.set(hwnd, lp);
    }
    return lp;
  }

  //
// 更新子窗口的位置和尺寸（对应 Android updateChildFrame）。
// 通过调用 child.nativeMove 更新 @Observed 属性，触发 UI 重新布局。
// child 子窗口
// x/y 新位置
// width/height 新尺寸
  public updateChildFrame(child: INativeWindow, x: number, y: number,
                          width: number, height: number): void {
    const lp = this.obtainChildLayoutParams(child);
    lp.x = x;
    lp.y = y;
    lp.width = width;
    lp.height = height;
    // 通过 nativeMove 更新 INativeWindow 的响应式属性（@Observed 自动触发 UI 重绘）
    child.nativeMove(x, y, width, height);
  }

  //
// 设置子窗口位置（对应 Android setChildPosition(x, y)）。
// child 子窗口
// x/y 新位置
  public setChildPosition(child: INativeWindow, x: number, y: number): void {
    const lp = this.obtainChildLayoutParams(child);
    lp.x = x;
    lp.y = y;
    child.nativeSetPosition(x, y);
  }

  //
// 设置子窗口尺寸（对应 Android setChildSize）。
// child 子窗口
// width/height 新尺寸
  public setChildSize(child: INativeWindow, width: number, height: number): void {
    const lp = this.obtainChildLayoutParams(child);
    lp.width = width;
    lp.height = height;
    child.nativeSetSize(width, height);
  }

  //
// 移除子窗口的 LayoutParams（窗口销毁时调用）。
// child 子窗口
  public removeChild(child: INativeWindow): void {
    this.mChildParams.delete(child.nativeGetHwnd());
  }

  //
// 清空所有子窗口的 LayoutParams。
  public clear(): void {
    this.mChildParams.clear();
  }

  //
// 获取子窗口的 LayoutParams（只读）。
// child 子窗口
// LayoutParams 或 undefined
  public getChildLayoutParams(child: INativeWindow): LayoutParams | undefined {
    return this.mChildParams.get(child.nativeGetHwnd());
  }
}
