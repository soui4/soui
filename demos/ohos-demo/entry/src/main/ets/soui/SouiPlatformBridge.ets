//
// SouiPlatformBridge.ets
//
// 全局单例桥接类，对应 Android 端 SouiPlatformBridge.java。
//
// 负责：
//   1. 生命周期：init / uninit —— 加载 libsoui4ohos.so 并 N-API 注册
//   2. 窗口管理：createWindow —— 按 className 创建 INativeWindow（SouiSurface / NativeEditView）
//   3. Native 句柄管理：createNative / destroyNative —— 委托 C++ surf_nativeCreate/Destroy
//   4. 窗口状态：destroyView / showView / moveWindow / setWindowSize / setWindowPosEx / enableWindow
//   5. 重绘请求：invalidateRect —— 查找 INativeWindow 并调用 nativeInvalidate
//   6. 定时器：setTimer / killTimer —— 委托 ArkTS setTimeout/clearTimeout
//   7. 软键盘：showSoftKeyboard —— 调输入法服务
//   8. 路径：getTempPath / getSpecialFolderPath —— 调 Context.cacheDir / filesDir
//   9. HWND ↔ INativeWindow 映射：hwndAsNativeWindow / mWindowMap
//  10. 消息调度：scheduleMessageProcessing —— setTimeout(0) 触发 C++ 消息排空
//  11. 字符串槽：string slot 机制（与 Android 端 slot 完全一致）
//  12. 输入法：IME 文本通过 slot 跨层传递
//  13. 捕获 / 焦点：setCapture / releaseCapture / getCapture / setFocus / getFocus
//
// N-API 模块访问：
//   直接 import libsoui4ohos.so 的类型化模块（Index.d.ts），不再使用
//   globalThis 注入 / any 断言，满足 ArkTS 严格模式约束。

import { INativeWindow } from './INativeWindow';
import { SouiSurface } from './SouiSurface';
import { NativeEditView } from './NativeEditView';
import { common } from '@kit.AbilityKit';
import { window } from '@kit.ArkUI';
import { inputMethod } from '@kit.IMEKit';
import pasteboard from '@ohos.pasteboard';
import display from '@ohos.display';
import media from '@ohos.multimedia.media';
import soui4ohos from 'libsoui4ohos.so';
import { hilog } from '@kit.PerformanceAnalysisKit';
// ArkTS 多模输入 KeyCode 枚举（@kit.InputKit），仅用于 IME 事件转发：
// HarmonyOS 的 KeyCode 值范围 2000+（不是 Android 的 7-67 系列），
// C++ convertKeyCode 表按此枚举映射到 Win32 VK_*。
import { KeyCode } from '@kit.InputKit';
import { AudioPlayer } from './AudioPlayer';
import { audio } from '@kit.AudioKit';

// Windows WS_* 风格常量 —— 值与 Win32 完全一致（与 Android SouiPlatformBridge 对齐） 
export const WS_OVERLAPPED   = 0x00000000;
export const WS_POPUP        = 0x80000000;
export const WS_CHILD        = 0x40000000;
export const WS_MINIMIZE     = 0x20000000;
export const WS_VISIBLE      = 0x10000000;
export const WS_DISABLED     = 0x08000000;
export const WS_CLIPSIBLINGS = 0x04000000;
export const WS_CLIPCHILDREN = 0x02000000;
export const WS_BORDER       = 0x00800000;
export const WS_CAPTION      = 0x00C00000; // BORDER | DLGFRAME
export const WS_SYSMENU      = 0x00080000;
export const WS_THICKFRAME   = 0x00040000;

export const HWND_DESKTOP = 0;
export const HWND_MESSAGE = -3;

// CSIDL_* 常量 —— 与 Win32 SHGetSpecialFolderPath 对齐 
export const CSIDL_PERSONAL      = 0x0005;
export const CSIDL_APPDATA       = 0x001a;
export const CSIDL_LOCAL_APPDATA = 0x001c;
export const CSIDL_MYMUSIC       = 0x000d;
export const CSIDL_MYVIDEO       = 0x000e;
export const CSIDL_MYDOCUMENTS   = 0x0005;

// 定时器管理 
type TimerRunnable = () => void;

//
// 定时器条目。
// SOUI 的 timer 是 repeat timer（Win32 SetTimer 语义：按固定间隔重复触发，
// 直到 KillTimer），因此用 setInterval 固定节奏重复；回调内 KillTimer 后
// 通过 active 标志 + 注册表身份校验保证不再触发（clearInterval 已取消）。
interface TimerEntry {
  timerId: number;
  interval: number;
  active: boolean;
  // setInterval 重复定时器句柄，用于取消 
  handle: number;
}

// SouiScreen 组件的最小接口（避免 SouiPlatformBridge ↔ SouiScreen 循环依赖） 
interface SouiScreenRef {
  layoutName?: string;
  windows?: INativeWindow[];
  addWindow?(window: INativeWindow): void;
  removeWindow?(hwnd: number): boolean;
}

// 消息/定时器投递器（对应 Android mBridgeHandler） 
interface BridgeHandler {
  postTask(f: TimerRunnable, delayMs?: number): number;
  clearTask(id: number): void;
  postInterval(f: TimerRunnable, intervalMs: number): number;
  clearIntervalTask(id: number): void;
}

const DOMAIN = 0x5351;
const TAG = 'soui-bridge';

// BridgeHandler 默认实现：基于 setTimeout/clearTimeout + setInterval/clearInterval 
class DefaultBridgeHandler implements BridgeHandler {
  postTask(f: TimerRunnable, delayMs?: number): number {
    return setTimeout(f, delayMs ?? 0);
  }
  clearTask(id: number): void {
    clearTimeout(id);
  }
  postInterval(f: TimerRunnable, intervalMs: number): number {
    return setInterval(f, intervalMs);
  }
  clearIntervalTask(id: number): void {
    clearInterval(id);
  }
}

export class SouiPlatformBridge {
  private static sInstance: SouiPlatformBridge | null = null;
  public static instance(): SouiPlatformBridge {
    if (!SouiPlatformBridge.sInstance) SouiPlatformBridge.sInstance = new SouiPlatformBridge();
    return SouiPlatformBridge.sInstance;
  }

  private audioPlayer:AudioPlayer= new AudioPlayer();
  private mContext: common.UIAbilityContext | null = null;
  // HWND → INativeWindow 映射（对应 Android mViewMap，用于 findViewByHandle fallback） 
  private mWindowMap: Map<number, INativeWindow> = new Map();
  // 子 HWND → 父 HWND 映射（供 getWindow 遍历父子/兄弟关系） 
  private mWndParent: Map<number, number> = new Map();
  // screenId → SouiScreen 实例 
  private mScreenMap: Map<number, SouiScreenRef> = new Map();
  // 主 Surface HWND → SouiScreen 实例（用于 createWindow 时查找挂载容器） 
  private mMainHwndToScreen: Map<number, SouiScreenRef> = new Map();

  private readonly mBridgeHandler: BridgeHandler = new DefaultBridgeHandler();
  // hWnd → (timerId → TimerEntry)，与 Win32 SetTimer 语义一致 
  private mBridgeTimerMap: Map<number, Map<number, TimerEntry>> = new Map();
  private mNextTimerId = 1;

  // 当前捕获窗口（SetCapture 语义） 
  private mCaptureHwnd = 0;
  // 当前焦点窗口（SetFocus 语义） 
  private mCurrentFocusHwnd = 0;

  // ===== IME 状态跟踪 =====
  // SouiSurface Canvas 是否已通过 imc.attach 绑定 IME（自绘组件路径） 
  private mImeAttached: boolean = false;
  // 当前 IME 文本接收目标 HWND（用于 on('insertText') 回调转发到 C++） 
  private mImeTargetHwnd: number = 0;
  // IME 回调是否已注册（全局 imc 实例上只需注册一次） 
  private mImeCallbacksRegistered: boolean = false;

  // ===== 软键盘高度监听（keyboardHeightChange → WM_KEYBOARD_HEIGHT）=====
  // 主窗口引用（注册 keyboardHeightChange 回调） 
  private mKeyWindow: window.Window | null = null;
  // 上一次通知 C++ 的键盘高度（物理像素），防重复通知 
  private mKeyboardHeightPx: number = 0;
  // 轮询兜底定时器句柄（on('keyboardHeightChange') 不可用时启用） 
  private mPollTimerId: number = -1;

  private constructor() {}

  // ============================= 像素单位换算（物理 px ↔ 逻辑 vp） ============================= 

  //
// 像素密度（vp→px 倍率，如 3.0）。
// 约定：C++/SOUI/swinx 侧一律使用物理像素（px）；
//       ArkTS/ArkUI 布局与事件一律使用逻辑像素（vp）。
// 本桥接层是两种坐标系的唯一换算边界：
//   - ArkTS → C++（尺寸通知、触摸坐标、setScreenSize）×density
//   - C++ → ArkTS（createWindow / moveWindow 等窗口几何）÷density
  private mDensity: number = 0;

  public getDensity(): number {
    if (this.mDensity > 0) return this.mDensity;
    try {
      const d = display.getDefaultDisplaySync().densityPixels;
      this.mDensity = (d !== undefined && d > 0) ? d : 1;
    } catch (e) {
      console.error('getDensity failed, fallback to 1:', e);
      this.mDensity = 1;
    }
    return this.mDensity;
  }

  // 物理像素 → 逻辑像素（用于 C++ 传来的窗口几何设置 ArkUI 布局） 
  public px2vp(px: number): number { return px / this.getDensity(); }

  // 逻辑像素 → 物理像素（用于 ArkTS 侧尺寸/触摸坐标回传 C++） 
  public vp2px(vp: number): number { return vp * this.getDensity(); }

  // ============================= 生命周期 ============================= 

  // 在 UIAbility.onCreate 调用 
  public init(ctx: common.UIAbilityContext): void {
    this.mContext = ctx;
    console.log('SouiPlatformBridge.init: native API registered');
    soui4ohos.registerPlatformAPI(this, ctx);
  }

  // 在 UIAbility.onDestroy 调用 
  public uninit(): void {
    soui4ohos.unregisterPlatformAPI();
    // 停止键盘高度轮询（兜底方案）
    if (this.mPollTimerId !== -1) {
      this.mBridgeHandler.clearIntervalTask(this.mPollTimerId);
      this.mPollTimerId = -1;
    }
    // 解绑软键盘高度监听
    if (this.mKeyWindow !== null) {
      try {
        this.mKeyWindow.off('keyboardHeightChange');
      } catch (e) {
        console.warn('uninit: off keyboardHeightChange failed:', e);
      }
      this.mKeyWindow = null;
    }
    this.mKeyboardHeightPx = 0;
    this.mWindowMap.clear();
    this.mWndParent.clear();
    this.mScreenMap.clear();
    this.mCaptureHwnd = 0;
    this.mCurrentFocusHwnd = 0;
    this.mContext = null;
  }

  // ============================= SOUI 启动/关闭 ============================= 

  public startSouiApp(): boolean {
    if (!this.mContext) return false;
    // 传入 resourceManager，C++ 侧用 OH_ResourceManager_InitNativeResourceManager
    // 创建原生 NativeResourceManager，SOUI 直接读取 rawfile 中的 uires / soui_sys_res。
    return soui4ohos.initSouiApp(this.mContext.resourceManager, this.mContext.filesDir) === 1;
  }

  public stopSouiApp(): void {
    soui4ohos.uninitSouiApp();
  }

  // SouiScreen 注册（onMounted 调用） 
  public registerScreen(screenId: number, screen: SouiScreenRef, nativeHwnd: number): number {
    this.mScreenMap.set(screenId, screen);
    const layout = screen.layoutName ?? '';
    const mainHwnd = soui4ohos.screenStartup(screenId, nativeHwnd, layout);
    this.ensureKeyboardHeightListener();
    console.log(`SouiPlatformBridge: screen[${screenId}] startup, hwnd=${nativeHwnd}, main=${mainHwnd}, layout=${layout}`);
    return mainHwnd;
  }
  public unregisterScreen(screenId: number): void {
    soui4ohos.screenShutdown(screenId);
    this.mScreenMap.delete(screenId);
  }

  public getScreen(screenId: number): SouiScreenRef | null { return this.mScreenMap.get(screenId) ?? null; }

  //
// 主 Surface HWND → SouiScreen 映射注册（aboutToAppear 调用）。
// SouiPlatformBridge.createWindow 通过父 HWND 反查 SouiScreen，
// 把新建的子窗口添加到对应 Screen 的 windows 数组。
  public registerMainScreen(mainHwnd: number, screen: SouiScreenRef): void {
    this.mMainHwndToScreen.set(mainHwnd, screen);
  }
  public unregisterMainScreen(mainHwnd: number): void {
    this.mMainHwndToScreen.delete(mainHwnd);
  }
  //
// 沿父链查找最近的 SouiScreen。
// createWindow 的 hParent 可能是主窗口 HWND 或其子窗口 HWND，
// 需要逐级向上找到承载它的 SouiScreen。
// 当前实现：直接在 mMainHwndToScreen 查找；未命中时遍历所有已注册 Screen。
  public findScreenByHwnd(hwnd: number): SouiScreenRef | null {
    if (hwnd === 0) {
      // HWND_DESKTOP：返回第一个已注册 Screen
      for (const screen of this.mMainHwndToScreen.values()) return screen;
      return null;
    }
    const direct = this.mMainHwndToScreen.get(hwnd);
    if (direct) return direct;
    // 未命中：可能是子窗口 HWND，遍历所有 Screen 用 windows 数组反查
    for (const screen of this.mMainHwndToScreen.values()) {
      if (typeof screen.removeWindow === 'function') {
        const arr: INativeWindow[] = screen.windows ?? [];
        if (arr.some(w => w.nativeGetHwnd() === hwnd)) return screen;
      }
    }
    // 兜底：返回第一个 Screen（顶层窗口 fallback）
    for (const screen of this.mMainHwndToScreen.values()) return screen;
    return null;
  }

  // ============================= Native 句柄管理（对应 Android createNative / destroyNative）============================= 

  // 创建 native 引用：调用 C++ surf_nativeCreate，返回 HWND 
  public createNative(wnd: INativeWindow): number {
    const nativeId = soui4ohos.surf_nativeCreate(wnd);
    if (nativeId !== 0) {
      this.mWindowMap.set(nativeId, wnd);
    }
    return nativeId;
  }

  // 销毁 native 引用：清理捕获/焦点/定时器后调用 C++ surf_nativeDestroy 
  public destroyNative(nativeId: number): void {
    if (nativeId === 0) return;
    if (nativeId === this.mCaptureHwnd) this.mCaptureHwnd = 0;
    if (nativeId === this.mCurrentFocusHwnd) {
      this.mCurrentFocusHwnd = 0;
    }
    this.killWindowTimers(nativeId);
    soui4ohos.surf_nativeDestroy(nativeId);
    this.mWindowMap.delete(nativeId);
  }

  // ============================= 窗口工厂（对应 Android createWindow）============================= 

  //
// 创建窗口（C++ 层 N-API 调用入口）。语义与 Win32 CreateWindow 一致。
// hParent        父窗口 HWND（HWND_DESKTOP=0 表示顶层）
// screenId       屏幕 ID
// className      窗口类名（空/"SouiSurface" → 普通 Surface；"Edit" → NativeEditView）
// title          窗口标题
// style          WS_* 风格位
// exStyle        WS_EX_* 扩展风格位
// x/y            初始位置
// w/h            初始尺寸（≤0 为 WRAP_CONTENT / MATCH）
// hMenuOrCtrlId  控件 ID（WS_CHILD 时有效）
// 新窗口 HWND，失败返回 0
//
// 单位约定：x/y/w/h 均为物理像素（C++ 侧坐标系），
// 此处统一换算为逻辑像素（vp）后再设置 ArkUI 布局。
  public createWindow(hParent: number, _screenId: number, className: string, _title: string,
                      style: number, _exStyle: number, x: number, y: number, w: number, h: number,
                      hMenuOrCtrlId: number): number
  {
    const isChild = (style & WS_CHILD) !== 0;
    const visible = (style & WS_VISIBLE) !== 0;
    const disabled = (style & WS_DISABLED) !== 0;
    let window: INativeWindow | null = null;

    // 物理 px → 逻辑 vp（ArkUI 布局单位）
    const xVp = this.px2vp(x);
    const yVp = this.px2vp(y);
    const wVp = w > 0 ? this.px2vp(w) : w;
    const hVp = h > 0 ? this.px2vp(h) : h;

    if (className === 'Edit') {
      window = new NativeEditView(hMenuOrCtrlId);
    } else {
      // 普通渲染 Surface：顶层 / 子窗口
      window = new SouiSurface(wVp, hVp, isChild);
    }

    if (!window) return 0;

    // 初始位置/可见性/启用状态（在 build 前设置即可生效）
    if (x !== 0 || y !== 0) window.nativeSetPosition(xVp, yVp);
    if (w > 0 && h > 0) window.nativeSetSize(wVp, hVp);
    if (!visible) window.nativeShow(0);
    if (disabled) window.nativeEnable(false);

    const hwnd = window.nativeGetHwnd();
    if (hwnd === 0) return 0;
    this.mWindowMap.set(hwnd, window);
    this.mWndParent.set(hwnd, hParent);

    // 把新建的子窗口添加到对应 SouiScreen 的 windows 数组（对齐 Android addView）
    // 主窗口（hParent === 0）不需要 addWindow，因为 screenStartup 已加载布局创建主窗口，
    // 而主 Surface 已在 aboutToAppear 时初始化添加
    if (hParent !== 0 || isChild) {
      const screen = this.findScreenByHwnd(hParent);
      if (screen && typeof screen.addWindow === 'function') {
        screen.addWindow(window);
      }
    } else {
      // 顶层窗口：也要添加到 Screen（如果存在），保证 ForEach 渲染。
      // 注意：不能再用 screen !== findScreenByHwnd(hwnd) 判断 —— 新建窗口的 hwnd
      // 不在任何 screen.windows 中，findScreenByHwnd(hwnd) 会走兜底返回第一个 Screen，
      // 与 screen（findScreenByHwnd(0)）恒等，条件恒为 false，导致 maindlg 的
      // Surface 永远不被 addWindow → 无 Canvas → 不执行 render。
      const screen = this.findScreenByHwnd(0);
      if (screen && typeof screen.addWindow === 'function') {
        // 仅按 HWND 查重，避免重复添加主 Surface（主 Surface 已在 aboutToAppear 时加入）
        const arr: INativeWindow[] = screen.windows ?? [];
        if (!arr.some(s => s.nativeGetHwnd() === hwnd)) {
          screen.addWindow(window);
        }
      }
    }

    console.log(`SouiPlatformBridge.createWindow: class=${className} hwnd=${hwnd} parent=${hParent} style=0x${style.toString(16)} isChild=${isChild} x=${x} y=${y} w=${w} h=${h}`);
    return hwnd;
  }

  // ============================= 窗口状态操作（对应 Android destroyView / showView / moveWindow 等）============================= 

  // 按 HWND 查找 INativeWindow。 
  public findViewByHandle(hwnd: number): INativeWindow | null {
    return this.mWindowMap.get(hwnd) ?? null;
  }

  // 销毁窗口（对应 Android destroyView）。 
  public destroyView(hwnd: number): boolean {
    const w = this.mWindowMap.get(hwnd);
    if (!w) return false;
    // 从 SouiScreen 的 windows 数组移除（对齐 Android removeView）
    // 注意：removeWindow 内部会调用 nativeDestroy，这里只做 Map 清理
    const screen = this.findScreenByHwnd(hwnd);
    if (screen && typeof screen.removeWindow === 'function') {
      screen.removeWindow(hwnd);
    } else {
      // 兜底：直接销毁（如 message-only 窗口）
      w.nativeDestroy();
    }
    this.mWindowMap.delete(hwnd);
    this.mWndParent.delete(hwnd);
    return true;
  }

  //
// 遍历窗口层级（对应 Win32 GetWindow / Android nativeGetWindow）。
// 由 C++ OhosPlatformAPI::getWindow 经由 N-API 调用。
//
// hwnd 目标窗口 HWND（GW_CHILD/GW_CHILDLAST 时为父窗口；其他时为当前窗口）
// code GW_* 常量：
//             0=GW_HWNDFIRST  第一个兄弟
//             1=GW_HWNDLAST   最后一个兄弟
//             2=GW_HWNDNEXT   下一个兄弟
//             3=GW_HWNDPREV   上一个兄弟
//             4=GW_OWNER      所有者（近似=父窗口）
//             5=GW_CHILD      第一个子窗口
//             6=GW_CHILDLAST  最后一个子窗口
// 匹配的 HWND，未找到返回 0
  public getWindow(hwnd: number, code: number): number {
    const GW_HWNDFIRST = 0;
    const GW_HWNDLAST  = 1;
    const GW_HWNDNEXT  = 2;
    const GW_HWNDPREV  = 3;
    const GW_OWNER     = 4;
    const GW_CHILD     = 5;
    const GW_CHILDLAST = 6;
    if (hwnd === 0) return 0;

    // 收集指定父窗口的所有子 HWND，保持 Map 插入顺序（=创建顺序，与 Win32 Z-Order 首末近似）。
    const childrenOf = (parent: number): number[] => {
      const arr: number[] = [];
      this.mWndParent.forEach((p: number, child: number) => {
        if (p === parent) arr.push(child);
      });
      return arr;
    };

    switch (code) {
      case GW_CHILD: {
        const kids = childrenOf(hwnd);
        return kids.length > 0 ? kids[0] : 0;
      }
      case GW_CHILDLAST: {
        const kids = childrenOf(hwnd);
        return kids.length > 0 ? kids[kids.length - 1] : 0;
      }
      case GW_OWNER: {
        // GW_OWNER 语义与 parent 不完全一致，但 SOUI 场景下 parent 是合理近似。
        return this.mWndParent.get(hwnd) ?? 0;
      }
      default: { // 兄弟导航：HWNDFIRST/HWNDLAST/HWNDNEXT/HWNDPREV
        const parent = this.mWndParent.get(hwnd) ?? 0;
        const siblings = childrenOf(parent);
        const idx = siblings.indexOf(hwnd);
        if (idx < 0) return 0;
        switch (code) {
          case GW_HWNDFIRST: return siblings.length > 0 ? siblings[0] : 0;
          case GW_HWNDLAST:  return siblings.length > 0 ? siblings[siblings.length - 1] : 0;
          case GW_HWNDNEXT:  return idx + 1 < siblings.length ? siblings[idx + 1] : 0;
          case GW_HWNDPREV:  return idx > 0 ? siblings[idx - 1] : 0;
          default: return 0;
        }
      }
    }
  }

  // 显示/隐藏窗口（对应 Android showView）。 
  public showView(hwnd: number, cmdShow: number): boolean {
    const w = this.mWindowMap.get(hwnd);
    if (!w) return false;
    return w.nativeShow(cmdShow);
  }

  //
// 移动并调整窗口大小（对应 Android moveWindow）。
// x/y/w/h 为物理像素（C++ 坐标系），÷density 换算为 vp 后设置布局。
  public moveWindow(hwnd: number, x: number, y: number, w: number, h: number): boolean {
    const wnd = this.mWindowMap.get(hwnd);
    if (!wnd) return false;
    const xVp = this.px2vp(x), yVp = this.px2vp(y);
    return wnd.nativeMove(xVp, yVp,
      w > 0 ? this.px2vp(w) : w, h > 0 ? this.px2vp(h) : h);
  }

  //
// 设置窗口大小（对应 Android setWindowSize）。
// cx/cy 为物理像素，÷density 换算为 vp 后设置布局。
  public setWindowSize(hwnd: number, cx: number, cy: number): boolean {
    const wnd = this.mWindowMap.get(hwnd);
    if (!wnd) return false;
    const cxVp = this.px2vp(cx), cyVp = this.px2vp(cy);
    return wnd.nativeSetSize(cxVp, cyVp);
  }

  //
// 设置窗口位置（对应 Android setWindowPosEx）。
// x/y 为物理像素，÷density 换算为 vp 后设置布局。
  public setWindowPosEx(hwnd: number, x: number, y: number): boolean {
    const wnd = this.mWindowMap.get(hwnd);
    if (!wnd) return false;
    const xVp = this.px2vp(x), yVp = this.px2vp(y);
    return wnd.nativeSetPosition(xVp, yVp);
  }

  // 设置窗口位置和大小（对应 Win32 SetWindowPos；hWndInsertAfter/flags 在 ArkUI 不使用）。x/y/cx/cy 为物理像素。 
  public setWindowPos(hwnd: number, _hWndInsertAfter: number, x: number, y: number,
                      cx: number, cy: number, _flags: number): boolean {
    return this.moveWindow(hwnd, x, y, cx, cy);
  }

  // 启用/禁用窗口（对应 Android enableWindow）。 
  public enableWindow(hwnd: number, enabled: boolean): boolean {
    const wnd = this.mWindowMap.get(hwnd);
    if (!wnd) return false;
    return wnd.nativeEnable(enabled);
  }

  // 检查窗口是否可见（对应 Android isWindowVisible）。 
  public isWindowVisible(hwnd: number): boolean {
    const wnd = this.mWindowMap.get(hwnd);
    if (!wnd) return false;
    return wnd.nativeIsVisible();
  }

  // 检查窗口是否启用（对应 Android isWindowEnabled）。 
  public isWindowEnabled(hwnd: number): boolean {
    const wnd = this.mWindowMap.get(hwnd);
    if (!wnd) return false;
    return wnd.nativeIsEnabled();
  }

  //
// 请求重绘指定 HWND（对应 Win32 InvalidateRect / Android requestInvalidate）。
// 由 C++ OhosPlatformAPI::invalidateRect 经由 N-API 调用。
//
// 注意：left/top/right/bottom 为物理像素。渲染缓冲区（ImageData）使用物理
// 分辨率（见 SouiSurface.onAreaChanged：area(vp)×density 后分配），dirty rect
// 与物理坐标系一致，putImageData 的 dirty 参数亦为像素单位，因此无需换算。
  public invalidateRect(hwnd: number, left: number, top: number,
                        right: number, bottom: number): boolean {
    const wnd = this.mWindowMap.get(hwnd);
    if (!wnd) return false;
    wnd.nativeInvalidate(left, top, right, bottom);
    return true;
  }

  // ============================= 定时器（对应 Android setTimer/killTimer）============================= 

  public setTimer(hWnd: number, timerId: number, uElapseMs: number): number {
    // 对齐 Android SouiPlatformBridge.setTimer：uElapse<=0 按 1ms 处理
    if (uElapseMs <= 0) uElapseMs = 1;
    // hWnd==0 或 timerId==0：系统自动分配（自增，永不返回 0）
    if (timerId === 0) timerId = this.mNextTimerId++;
    // 同 (hWnd, timerId) 已有旧定时器 → 先取消再建（Win32 SetTimer 替换语义）
    let perHwnd = this.mBridgeTimerMap.get(hWnd);
    if (perHwnd) {
      const oldEntry = perHwnd.get(timerId);
      if (oldEntry) this.cancelTimerEntry(oldEntry);
    }
    const entry: TimerEntry = {
      timerId: timerId,
      interval: uElapseMs,
      active: true,
      handle: 0
    };
    if (!perHwnd) {
      perHwnd = new Map<number, TimerEntry>();
      this.mBridgeTimerMap.set(hWnd, perHwnd);
    }
    perHwnd.set(timerId, entry);
    this.scheduleTimerTick(hWnd, entry);
    return timerId;
  }

  public killTimer(hWnd: number, timerId: number): boolean {
    const perHwnd = this.mBridgeTimerMap.get(hWnd);
    if (!perHwnd) return false;
    const entry = perHwnd.get(timerId);
    if (!entry) return false;
    this.cancelTimerEntry(entry);
    perHwnd.delete(timerId);
    return true;
  }

  public killWindowTimers(hWnd: number): void {
    const perHwnd = this.mBridgeTimerMap.get(hWnd);
    if (!perHwnd) return;
    perHwnd.forEach((entry) => this.cancelTimerEntry(entry));
    perHwnd.clear();
    this.mBridgeTimerMap.delete(hWnd);
  }

  // 启动重复定时器（setInterval 固定节奏 repeat 语义，对应 Win32 SetTimer）。 
  private scheduleTimerTick(hWnd: number, entry: TimerEntry): void {
    entry.handle = this.mBridgeHandler.postInterval(() => this.runTimerTick(hWnd, entry), entry.interval);
  }

  //
// 单次触发：setInterval 每到间隔自动重复调用。
// 1) 校验 entry 仍 active 且仍是注册表中的当前条目（防替换/注销后残火）；
// 2) 同步回调 C++（NAPI → onTimerExpired → SendMessage 同线程直投）。
// 回调内部 KillTimer（如 kPulseTimer 动画结束时自毁）会走 cancelTimerEntry
// → clearInterval，之后的间隔触发自然停止。
  private runTimerTick(hWnd: number, entry: TimerEntry): void {
    if (!entry.active) return;
    {
      const perHwnd = this.mBridgeTimerMap.get(hWnd);
      if (!perHwnd || perHwnd.get(entry.timerId) !== entry) return;
    }
    soui4ohos.onTimerExpired(hWnd, entry.timerId);
  }

  // KillTimer：置 inactive + clearInterval，取消尚未发生的后续触发。 
  private cancelTimerEntry(entry: TimerEntry): void {
    entry.active = false;
    this.mBridgeHandler.clearIntervalTask(entry.handle);
  }

  // ============================= 捕获 / 焦点 / IME ============================= 

  // Win32 SetCapture 语义：将所有输入事件重定向到指定 HWND。 
  public setCapture(hwnd: number): number {
    const old = this.mCaptureHwnd;
    this.mCaptureHwnd = hwnd;
    return old;
  }
  // Win32 ReleaseCapture 语义。 
  public releaseCapture(): boolean {
    if (this.mCaptureHwnd === 0) return false;
    this.mCaptureHwnd = 0;
    return true;
  }
  public getCapture(): number { return this.mCaptureHwnd; }

  //
// Win32 SetFocus 语义：将输入焦点切到指定 HWND，并同步 IME 状态。
// 对齐 Android SouiPlatformBridge.setFocus + applyFocusAndIme：
//   - hwnd == 0：清空焦点，确保 IME 收起
//   - hwnd 为 NativeEditView：弹 IME
//   - hwnd 为 SouiSurface：收 IME（SOUI 自己处理输入或不需要 IME）
  public setFocus(hwnd: number): number {
    const old = this.mCurrentFocusHwnd;
    if (old === hwnd) return old;
    this.mCurrentFocusHwnd = hwnd;

    // 1) 通知旧焦点窗口 WM_KILLFOCUS（对齐 Android applyFocusAndIme 旧 view clearFocus）
    if (old !== 0) {
      const oldW = this.mWindowMap.get(old);
      if (oldW) oldW.nativeSendMessage(0x0008 // WM_KILLFOCUS 
, 0, 0);
    }

    // 2) 通知新焦点窗口 WM_SETFOCUS + IME 显隐（异步切到 UI 线程）
    // ArkTS setTimeout(0) 等价 Android mBridgeHandler.post
    setTimeout(() => this.applyFocusAndIme(hwnd), 0);
    return old;
  }

  //
// 把焦点 + IME 状态同步到当前 mCurrentFocusHwnd。必须在 UI 线程调用。
// 对齐 Android applyFocusAndIme：通知 C++ notifyFocusGained + 弹/收 IME。
//
// IME 策略（对齐 Android 注释意图：SouiBaseSurface 也弹 IME）：
//   - NativeEditView：先 requestFocus 聚焦 TextInput → showSoftKeyboard 内再次 requestFocus
//     （双重保险）→ 系统自动绑定 IME + 弹出键盘
//   - SouiSurface（非容器）：requestFocus → Canvas 获焦 + imc.attach 绑定自绘组件 IME
//   - 容器 SouiSurface / hwnd==0：收起 IME
// C++ 侧若焦点控件不需要 IME（如按钮），可调 showSoftKeyboard(hwnd, false) 收起。
  private applyFocusAndIme(newHwnd: number): void {
    if (newHwnd === 0) {
      this.showSoftKeyboard(0, false);
      return;
    }
    const newW = this.mWindowMap.get(newHwnd);
    if (!newW) return;
    soui4ohos.notifyFocusGained(newHwnd);
  }
  public getFocus(): number { return this.mCurrentFocusHwnd; }

  // 通知 C++ 焦点获得（由 SouiSurface / NativeEditView 的 onFocus 回调触发）。 
  public notifyFocusGained(hwnd: number): void {
    soui4ohos.notifyFocusGained(hwnd);
  }

  //
// 软键盘显隐（对齐 Android SouiPlatformBridge.applyFocusAndIme IME 策略）。
//
// 参数对齐 C++ OhosPlatformAPI::showSoftKeyboard(HWND hWnd, BOOL bShow)：
// C++ 侧通过 N-API 以 int64 传入 hWnd（number），此处接收 hwnd 后用 mWindowMap
// 解析为 INativeWindow 实例（NativeEditView / SouiSurface）。
// 早期版本参数为 object，但 N-API 跨层传递的是 number 句柄，
// 导致 instanceof 判断全部失效、软键盘无法打开——已修正为 hwnd: number。
//
// HarmonyOS IME 两种绑定路径：
//   1. TextInput 组件（NativeEditView）：requestFocus → 系统自动绑定 IME + 弹出键盘
//   2. 自绘组件（SouiSurface Canvas）：imc.attach(true, textConfig) → 绑定 IME 到
//      Canvas，配合 on('insertText') 回调接收输入法文本 → 转发到 C++ sendImeString
//
// 任意 hwnd 对应的窗口都能打开软键盘（不限于 NativeEditView）：
//   - bShow=true + NativeEditView：requestFocus 聚焦 TextInput
//   - bShow=true + SouiSurface：requestFocus 聚焦 Canvas + imc.attach 绑定 IME
//   - bShow=false：detach（SouiSurface 路径）+ hideTextInput（兜底）
//
// hwnd  当前焦点窗口句柄（C++ HWND，0 表示无焦点窗口）
// bShow true=弹出软键盘, false=收起
  public async showSoftKeyboard(hwnd: number, bShow: boolean): Promise<boolean> {
    try {
      hilog.info(DOMAIN,TAG,"showSoftKeyboard, hwnd="+hwnd+", bShow="+bShow);
      const imc = inputMethod.getController();
      if (bShow) {
        // 先清理旧的 IME attach 绑定（避免 attach 冲突）
        if (this.mImeAttached) {
          await imc.detach().catch(() => {});
          this.mImeAttached = false;
          this.mImeTargetHwnd = 0;
        }
        // 通过 HWND 解析出 ArkTS 层 INativeWindow 实例
        // （C++ 只能传 number 句柄，不能直接传 ArkTS 对象）
        const focusView = this.mWindowMap.get(hwnd) ?? null;
        if (focusView instanceof NativeEditView) {
          // NativeEditView：requestFocus 聚焦 TextInput → 系统自动绑定 IME + 弹出键盘
          focusView.requestFocus();
        } else if (focusView instanceof SouiSurface) {
          // SouiSurface（Canvas 自绘）：requestFocus 聚焦 Canvas + imc.attach 绑定 IME
          const surface = focusView as SouiSurface;
          surface.requestFocus();
          // windowId=0：单窗口 App 使用当前前台窗口，无需获取系统 Window ID
          const textConfig: inputMethod.TextConfig = {
            inputAttribute: {
              textInputType: inputMethod.TextInputType.TEXT,
              enterKeyType: inputMethod.EnterKeyType.GO,
            },
            cursorInfo: { left: 0, top: 0, width: 1, height: 40 },
            selection: { start: 0, end: 0 },
            windowId: 0,
          };
          await imc.attach(true, textConfig).catch((e: Error) => {
            console.error('imc.attach failed:', e);
          });
          this.mImeAttached = true;
          this.mImeTargetHwnd = surface.nativeGetHwnd();
          this.ensureImeCallbacks(imc);
        } else {
          hilog.warn(DOMAIN,TAG,"showSoftKeyboard: hwnd "+hwnd+" 未在 mWindowMap 中找到对应 INativeWindow");
        }
      } else {
        // 收起 IME：先 detach 自绘组件绑定，再 hideTextInput 兜底
        if (this.mImeAttached) {
          await imc.detach().catch(() => {});
          this.mImeAttached = false;
          this.mImeTargetHwnd = 0;
        }
        await imc.hideTextInput().catch(() => {});
      }
      return true;
    } catch (e) {
      console.error('showSoftKeyboard error:', e);
      return false;
    }
  }

  //
// 注册 IME 文本回调（全局 imc 实例上只需注册一次）。
// on('insertText')：输入法插入文本 → 转发到 C++ sendImeString
// on('deleteLeft')：退格键 → 转发 VK_BACK 按键事件到 C++
  private ensureImeCallbacks(imc: inputMethod.InputMethodController): void {
    if (this.mImeCallbacksRegistered) return;
    this.mImeCallbacksRegistered = true;

    imc.on('insertText', (text: string) => {
      // 日志确认 insertText 触发（requestFocus 改用 .id() 后应能收到）
      hilog.info(DOMAIN,TAG,"insertText: text='"+text+"', targetHwnd="+this.mImeTargetHwnd);
      if (this.mImeTargetHwnd !== 0) {
        this.sendImeStringToCxx(this.mImeTargetHwnd, text);
      }
    });

    imc.on('deleteLeft', (length: number) => {
      hilog.info(DOMAIN,TAG,"deleteLeft: length="+length+", targetHwnd="+this.mImeTargetHwnd);
      if (this.mImeTargetHwnd !== 0) {
        // 退格：用 ArkTS KeyCode.KEYCODE_DEL (=2055) 作为键码下发，
        // 统一走 C++ convertKeyCode 查表 → VK_BACK(0x08)。
        // 之前硬编码 0x08 是 Win32 VK_BACK 值（不是合法 ArkTS KeyCode），
        // 在"原来版本"的 Android 查表中命中 case 8 → return '1'(0x31)，
        // 导致退格变输入字符'1'，这就是用户报告的 BUG。
        // 每个字符删除都发 DOWN + UP 完整按键对，避免 SOUI 内部按键状态不一致。
        const now = Date.now();
        for (let i = 0; i < length; i++) {
          const ts = now + i * 2;
          soui4ohos.surf_onKeyEvent(
            this.mImeTargetHwnd, KeyCode.KEYCODE_DEL, 0, 0, 0, 0, 0, 0, ts
          );
          soui4ohos.surf_onKeyEvent(
            this.mImeTargetHwnd, KeyCode.KEYCODE_DEL, 1, 0, 0, 0, 0, 0, ts + 1
          );
        }
      }
    });
  }

  //
// 播放 WAV 音频文件，语义对齐 Win32 PlaySound / Android SouiPlatformBridge.playSound。
// 使用 AVPlayer（对应 Android MediaPlayer）：
//   - 空路径或 SND_PURGE → 停止当前播放
//   - 新播放前先停止释放旧实例
// pszSound 音频文件绝对路径（沙盒内）
// fdwSound Win32 SND_* 标志
// true 播放成功启动
  public playSound(pszSound: string, fdwSound: number): boolean {
    this.audioPlayer.playSound(pszSound,fdwSound);
    return true;
  }

  // ============================= 路径（对应 Path API）============================= 

  // getCacheDir() —— 对应 Android SouiPlatformBridge.getTempPath 
  public getTempPath(): string | null {
    return this.mContext?.cacheDir ?? null;
  }

  // CSIDL → ArkTS 沙盒目录映射，与 Android SouiPlatformBridge.getSpecialFolderPath 对齐 
  public getSpecialFolderPath(nFolder: number, fCreate: boolean): string | null {
    if (!this.mContext) return null;
    switch (nFolder) {
      case CSIDL_LOCAL_APPDATA:
        return this.mContext.cacheDir;
      case CSIDL_PERSONAL:
      case CSIDL_MYDOCUMENTS:
      case CSIDL_APPDATA:
        return this.mContext.filesDir;
      case CSIDL_MYMUSIC:
        return this.mContext.filesDir + '/Music';
      case CSIDL_MYVIDEO:
        return this.mContext.filesDir + '/Movies';
      default:
        return this.mContext.filesDir;
    }
  }

  // ============================= 消息调度（scheduleMessageProcessing）============================= 

  public scheduleMessageProcessing(): void {
    this.mBridgeHandler.postTask(() => {
      soui4ohos.processPendingMessages();
      let idleCount = 0;
      for (let i = 0; i < 8; i++) {
        if (soui4ohos.processIdle(idleCount++) === 0) break;
      }
    }, 0);
  }

  // ============================= HWND → NativeWindow ============================= 

  public hwndAsNativeWindow(hwnd: number): INativeWindow | null {
    return this.mWindowMap.get(hwnd) ?? null;
  }

  // ============================= 剪贴板（Clipboard API 接入）============================= 
  public getClipboardSysText(): string {
    try {
      const sysBoard = pasteboard.getSystemPasteboard();
      const data = sysBoard.getDataSync();
      if (!data) return '';
      return data.getPrimaryText() ?? '';
    } catch (e) { return ''; }
  }
  public setClipboardSysText(text: string): void {
    try {
      const sysBoard = pasteboard.getSystemPasteboard();
      sysBoard.setData(pasteboard.createData(pasteboard.MIMETYPE_TEXT_PLAIN, text));
    } catch (e) { console.error('setClipboardSysText err:', e); }
  }

  // ============================= String slots ============================= 
  public allocStringSlot(): number {
    return soui4ohos.allocStringSlot();
  }
  public freeStringSlot(slotId: number): void {
    soui4ohos.freeStringSlot(slotId);
  }
  public readCxxStringSlot(slotId: number): string {
    return soui4ohos.readCxxStringSlot(slotId);
  }
  public writeCxxStringSlot(slotId: number, str: string): void {
    soui4ohos.writeCxxStringSlot(slotId, str);
  }

  // ============================= IME → C++ ============================= 
  public sendImeStringToCxx(hwnd: number, str: string): void {
    soui4ohos.sendImeString(hwnd, str);
  }

  // ============================= 屏幕尺寸变化 → C++ MoveWindow ============================= 

  //
// 通知 C++ 屏幕尺寸（对应 Android SouiScreen.syncScreenSize）。
// w/h 必须为物理像素（调用方 SouiScreen.onAreaChange 已把 area(vp)×density），
// 与 C++ 侧物理像素坐标系保持一致。
  public setScreenSize(screenId: number, hMainWnd: number, w: number, h: number): void {
    soui4ohos.setScreenSize(screenId, hMainWnd, w, h);
  }

  // ============================= 屏幕信息（供 C++ getScreenWidth/Height/getDpi 调用）============================= 

  //
// 获取屏幕物理尺寸。返回 (w << 32) | h 打包的数值（int64 安全，w*h 均远小于 2^31）。
// C++ OhosPlatformAPI::getScreenWidth/getScreenHeight 经 N-API 调用本方法，
// 此前 ArkTS 侧未实现导致 C++ 走 1280x720 兜底值。
  public getScreenSize(_screenId: number): number {
    let w = 0;
    let h = 0;
    try {
      const d = display.getDefaultDisplaySync();
      // Display.width/height 即屏幕物理像素（px），无需换算
      w = Math.round(d.width);
      h = Math.round(d.height);
    } catch (e) {
      console.error('getScreenSize failed:', e);
    }
    if (w <= 0 || h <= 0) {
      // 兜底：按 1280x720 vp 折算物理像素
      const density = this.getDensity();
      w = Math.round(1280 * density);
      h = Math.round(720 * density);
    }
    return w * 4294967296 + h;
  }

  // 获取屏幕 DPI：density × 160（对齐 Android xhdpi 换算法）。 
  public getDpi(): number {
    return Math.round(this.getDensity() * 160);
  }

  // ============================= 键盘高度监听（keyboardHeightChange → WM_KEYBOARD_HEIGHT）============================= 

  //
// 确保键盘高度监听器已注册。
// 通过 getLastWindow() 获取主窗口后，订阅 keyboardHeightChange 事件，
// 键盘弹出/收起时实时获取高度（px，物理像素）并通知 C++。
//
// 与 Android 端 WindowInsetsAnimation.Callback.onProgress 对齐：
//   - Android：ime().bottom → SouiPlatformBridge.nativeSetKeyboardHeight
//   - OHOS：keyboardHeightChange 回调直接给出物理像素高度 → notifyKeyboardHeight(px)
//
// 注意：keyboardHeightChange 回调的高度单位为 px（物理像素），无需 vp2px 换算。
  public ensureKeyboardHeightListener(): void {
    if (this.mKeyWindow !== null) return; // 已注册，直接返回
    if (!this.mContext) return;
    try {
      window.getLastWindow(this.mContext).then((mainWin: window.Window) => {
        this.mKeyWindow = mainWin;
        // 订阅 keyboardHeightChange 事件（回调高度单位为 px）
        try {
          mainWin.on('keyboardHeightChange', (keyboardHeight: number) => {
            const hPx: number = Math.round(keyboardHeight);
            if (hPx !== this.mKeyboardHeightPx) {
              this.mKeyboardHeightPx = hPx;
              this.notifyKeyboardHeight(hPx);
              hilog.info(DOMAIN, TAG, 'keyboardHeightChange: height PX=' + hPx);
            }
          });
        } catch (e) {
          console.warn('ensureKeyboardHeightListener: on keyboardHeightChange failed:', e);
          // 兜底：keyboardHeightChange 不可用时轮询 getWindowAvoidArea
          this.ensureKeyboardHeightListenerPolling(mainWin);
        }
      }).catch((e: Error) => {
        console.warn('ensureKeyboardHeightListener: getLastWindow failed:', e);
      });
    } catch (e) {
      console.warn('ensureKeyboardHeightListener error:', e);
    }
  }

  //
// 从 AvoidArea 的四个 Rect 中取出最大高度值作为键盘高度。
// 软键盘通常出现在 bottomRect，但兼容横竖屏/折叠屏时可能在 left/right/top。
  private maxAvoidAreaHeight(area: window.AvoidArea | null | undefined): number {
    let h = 0;
    if (!area) return h;
    if (area.topRect) { h = Math.max(h, area.topRect.height); }
    if (area.bottomRect) { h = Math.max(h, area.bottomRect.height); }
    if (area.leftRect) { h = Math.max(h, area.leftRect.height); }
    if (area.rightRect) { h = Math.max(h, area.rightRect.height); }
    return h;
  }

  //
// 兜底轮询方案：当 on('keyboardHeightChange') 不可用时，
// 启动 100ms 间隔的 setInterval，主动 getWindowAvoidArea 并比对高度变化。
  private ensureKeyboardHeightListenerPolling(mainWin: window.Window): void {
    if (this.mPollTimerId !== -1) return;
    hilog.info(DOMAIN, TAG, 'ensureKeyboardHeightListenerPolling: fallback to 100ms interval');
    this.mPollTimerId = this.mBridgeHandler.postInterval(() => {
      if (!this.mKeyWindow) {
        if (this.mPollTimerId !== -1) {
          this.mBridgeHandler.clearIntervalTask(this.mPollTimerId);
          this.mPollTimerId = -1;
        }
        return;
      }
      try {
        const area: window.AvoidArea = mainWin.getWindowAvoidArea(window.AvoidAreaType.TYPE_KEYBOARD);
        const h: number = this.maxAvoidAreaHeight(area);
        const hPx: number = Math.round(this.vp2px(h));
        if (hPx !== this.mKeyboardHeightPx) {
          this.mKeyboardHeightPx = hPx;
          this.notifyKeyboardHeight(hPx);
        }
      } catch (e) {
        // 静默忽略单次轮询异常
      }
    }, 100);
  }

  // ============================= 键盘高度 → C++ WM_KEYBOARD_HEIGHT ============================= 
  public notifyKeyboardHeight(heightPx: number): void {
    soui4ohos.setKeyboardHeight(heightPx);
  }
}
