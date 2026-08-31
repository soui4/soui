//
// SouiScreen.ets
//
// SOUI 屏幕容器（ArkUI 组件），对应 Android 端 SouiScreen.java。
//
// 职责：
//   - 作为单 UIAbility 的顶层容器（Stack），承载所有 Native 子窗口
//   - 每个 UIAbility 最多一个 SouiScreen（对应 Android 设计："Each Activity can only have one SouiScreen"）
//   - onAreaChange: 首次尺寸就绪 → SouiPlatformBridge.registerScreen → C++ 层初始化 HWND
//   - aboutToDisappear: 反向注销 → C++ ScreenShutdown + EraseScreen
//   - 所有通过 SouiPlatformBridge.createWindow 创建的 INativeWindow 都挂在其下
//
// 设计：
//   - 顶层主 Surface 使用 SouiSurface（Canvas 自绘），尺寸与屏幕一致
//   - 该主 Surface 的 HWND 即为 SouiPlatformBridge.registerScreen 的 screenHwnd 参数
//   - 后续 createWindow 创建的子窗口（popup/child/Surface/Edit）也添加到 @State windows 数组
//   - 使用 ForEach + SouiSurfaceView / NativeEditViewView 动态渲染所有子窗口
//
// 响应式机制（对齐 Android View 体系）：
//   - SouiSurface / NativeEditView 是 @Observed 类，视觉属性变化触发 UI 更新
//   - SouiSurfaceView / NativeEditViewView 是 @Component + @ObjectLink，自动响应属性变化
//   - SouiScreen 使用 @State windows 数组，添加/删除子窗口时触发 ForEach 重新渲染

import { SouiPlatformBridge } from './SouiPlatformBridge';
import { SouiSurface } from './SouiSurface';
import { SouiSurfaceView } from './SouiSurfaceView';
import { NativeEditView } from './NativeEditView';
import { NativeEditViewView } from './NativeEditViewView';
import { INativeWindow } from './INativeWindow';

@Component
export struct SouiScreen {
  @Prop layoutName: string = '';
  @Prop screenId: number = 1;
  // 所有子窗口列表（主 Surface + 子窗口 Surface + 子窗口 Edit），变化触发 ForEach 重新渲染 
  @State windows: INativeWindow[] = [];
  // 主 Surface 的 HWND（= napi_ref 指针值） 
  private mainNativeHwnd: number = 0;
  // 屏幕 area 宽高（首次就绪后启动 SOUI） 
  private areaW: number = 0;
  private areaH: number = 0;
  // 内部标志：SOUI 是否已启动（首次尺寸就绪后置 true，避免重复 startup） 
  private _souiStarted: boolean = false;

  aboutToAppear(): void {
    // 创建顶层主 Surface（全屏，作为顶级窗口挂载点）
    // 构造时即调用 surf_nativeCreate 注册到 C++ 拿到 HWND
    // isContainer=true：主 Surface 只是屏幕容器（对齐 Android SouiScreen 的
    // SouiBaseSurface 容器语义），不渲染 bitmap；真正渲染的是 screenStartup
    // 返回的 maindlg（mainHwnd），它会被 setScreenSize 全屏铺在容器之上。
    const mainSurface = new SouiSurface(// initW
 0, /*initH*/ 0, /*isChild*/ false, /*isContainer*/ true);
    this.mainNativeHwnd = mainSurface.nativeGetHwnd();
    // 加入 @State windows 数组，触发 ForEach 渲染
    this.windows = [mainSurface];
    // 注册到 SouiPlatformBridge，使 createWindow 能根据父 HWND 找到本 Screen
    SouiPlatformBridge.instance().registerMainScreen(this.mainNativeHwnd, this);
  }

  aboutToDisappear(): void {
    SouiPlatformBridge.instance().unregisterMainScreen(this.mainNativeHwnd);
    SouiPlatformBridge.instance().unregisterScreen(this.screenId);
    // 销毁所有子窗口
    for (const w of this.windows) {
      w.nativeDestroy();
    }
    this.windows = [];
    this.mainNativeHwnd = 0;
  }

  //
// 添加子窗口到容器（对应 Android SouiScreen.addView / SouiAbsLayout.addView）。
// 由 SouiPlatformBridge.createWindow 在创建子窗口后调用。
// 支持 SouiSurface 和 NativeEditView 两种类型。
  public addWindow(window: INativeWindow): void {
    this.windows.push(window);
    // 触发 @State 变化，ForEach 重新渲染
    this.windows = [...this.windows];
  }

  //
// 移除子窗口（对应 Android SouiScreen.removeView / SouiAbsLayout.removeView）。
// 由 SouiPlatformBridge.destroyView 调用。
  public removeWindow(hwnd: number): boolean {
    const idx = this.windows.findIndex(w => w.nativeGetHwnd() === hwnd);
    if (idx < 0) return false;
    this.windows[idx].nativeDestroy();
    this.windows.splice(idx, 1);
    this.windows = [...this.windows];
    return true;
  }

  build() {
    Stack({ alignContent: Alignment.TopStart }) {
      // 动态渲染所有子窗口（主 Surface + 子窗口 Surface + 子窗口 Edit）
      // 对应 Android 端 ViewGroup 的 addView/removeView 机制
      // 使用 if/else if 区分 SouiSurface 和 NativeEditView，对齐 Android
      // 同类型容器（ViewGroup）下不同子 View 类型的混合排版
      ForEach(this.windows, (w: INativeWindow) => {
        if (w instanceof SouiSurface) {
          // 渲染 Surface：基于 Canvas 自绘
          SouiSurfaceView({ surface: w as SouiSurface })
        } else if (w instanceof NativeEditView) {
          // 渲染 Edit：原生 TextInput 控件
          NativeEditViewView({ edit: w as NativeEditView })
        }
      }, (w: INativeWindow) => w.nativeGetHwnd().toString())
    }
    .width('100%')
    .height('100%')
    .onAreaChange((_old, area) => {
      // area.width/height 为逻辑像素（vp）—— 转换为物理像素（px）再传给 C++。
      // SOUI/swinx 全链路使用物理像素坐标系（含 moveWindow 的 x/y 也需要物理值，
      // 桥接层会再 ÷density 还原为 vp 设置 ArkUI 布局）。
      const bridge = SouiPlatformBridge.instance();
      const wPx = Math.round(bridge.vp2px(Math.floor(area.width as number)));
      const hPx = Math.round(bridge.vp2px(Math.floor(area.height as number)));
      if (wPx <= 0 || hPx <= 0) return;
      this.areaW = wPx;
      this.areaH = hPx;

      if (this.mainNativeHwnd === 0 || this.windows.length === 0) return;

      // 首次尺寸就绪 → 启动 SOUI（screenStartup 内部加载布局并创建主窗口）
      if (!this._souiStarted) {
        this._souiStarted = true;
        const mainHwnd = bridge.registerScreen(this.screenId, this, this.mainNativeHwnd);
        // 对齐 Android SouiScreen.syncScreenSize：
        // 1) 先把主 Surface 移到全屏。
        //    C++ 侧注册时主 Surface 窗口对象初始为 100x100（RegisterVirtualHWND），
        //    moveWindow 同步 ArkTS 布局，setScreenSize 直接 C++ MoveWindow 扩展窗口对象/HDC，
        //    否则 InvalidateRect/WM_PAINT 的 dirty 区域只有 100x100，渲染只占屏幕上角。
        bridge.setScreenSize(this.screenId, this.mainNativeHwnd, wPx, hPx);
        bridge.moveWindow(this.mainNativeHwnd, 0, 0, wPx, hPx);
        // 2) 再把 SOUI 主窗口（ScreenStartup 返回的 maindlg）移到全屏。
        //    返回 HWND 为 0 则不处理（对齐 Android 逻辑）。
        if (mainHwnd !== 0) {
          bridge.moveWindow(mainHwnd, 0, 0, wPx, hPx);
        }
      } else {
        // 后续尺寸变化：通知 C++ MoveWindow + setScreenSize
        bridge.moveWindow(this.mainNativeHwnd, 0, 0, wPx, hPx);
        const focusHwnd = bridge.getFocus();
        const targetHwnd = focusHwnd !== 0 ? focusHwnd : this.mainNativeHwnd;
        bridge.setScreenSize(this.screenId, targetHwnd, wPx, hPx);
      }
    });
  }
}
