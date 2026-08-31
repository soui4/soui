declare namespace soui4ohos {
  // 平台 API 注册
  function registerPlatformAPI(bridge: object, ctx: object): number;
  function unregisterPlatformAPI(): void;
  // 定时器
  function onTimerExpired(hWnd: number, timerId: number): void;
  // 自绘 Surface：创建/销毁/渲染/事件
  function surf_nativeCreate(nativeWnd: object): number;
  function surf_nativeDestroy(nativeId: number): void;
  /** 渲染并返回本次实际重绘的更新区域 [l, t, r, b]（物理像素）；空矩形表示无需上屏 */
  function surf_render(nativeId: number, pixelBuffer: Uint8ClampedArray | Uint8Array,
                       width: number, height: number): number[];
  function surf_onSizeChanged(nativeId: number, width: number, height: number): void;
  function surf_onMotionEvent(nativeId: number, action: number, x: number, y: number,
                              pointerId: number, buttonState: number,
                              vscroll: number, hscroll: number,
                              metaState: number, timestamp: number): void;
  function surf_onKeyEvent(nativeId: number, keyCode: number, action: number,
                            metaState: number, repeatCount: number, scanCode: number,
                            unicodeChar: number, flags: number, timestamp: number): boolean;
  // HWND → ArkTS NativeWindow
  function nativeViewFromHwnd(hwnd: number): object | undefined;
  // Screen 生命周期
  function screenStartup(screenId: number, screenHwnd: number, layout: string): number;
  function screenShutdown(screenId: number): void;
  // Edit 事件通知
  function nativeNotifyEditEvent(hEdit: number, code: number, wparam: number, lparam: number): void;
  // 字符串槽
  function readCxxStringSlot(slotId: number): string;
  function writeCxxStringSlot(slotId: number, str: string): void;
  // 焦点 / 屏幕 / 键盘
  function notifyFocusGained(hwnd: number): void;
  function setScreenSize(screenId: number, hMainWnd: number, w: number, h: number): void;
  function moveWindowNative(hwnd: number, x: number, y: number, w: number, h: number): void;
  function initSouiApp(rawfileMgr: object, appFilesDir: string): number;
  function uninitSouiApp(): void;
  function processPendingMessages(): void;
  function processIdle(idleCount: number): number;
  function setKeyboardHeight(h: number): void;
  function allocStringSlot(): number;
  function freeStringSlot(slotId: number): void;
  function sendImeString(hwnd: number, str: string): void;
}

export default soui4ohos;
