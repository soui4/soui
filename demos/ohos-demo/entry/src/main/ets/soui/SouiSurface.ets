//
// SouiSurface.ets
//
// SOUI 主渲染 Surface，对应 Android 端 SouiBaseSurface.java + SouiSurface.java。
//
// 设计：
//   - @Observed 类，视觉属性（x/y/width/height/visible/enabled）变化自动触发 UI 更新
//   - 基于 ArkUI Canvas 自绘组件实现，可与原生 ArkUI 组件无缝混合排版
//   - 实现 INativeWindow 接口，HWND = N-API 全局引用（napi_ref）指针值
//   - 在 onAreaChange 中分配像素缓冲区并通知 C++ 尺寸变化
//   - 在 nativeInvalidate 中调度局部或整体重绘：surf_render → 同步 PixelMap → drawImage
//   - 在 onTouch 中将 ArkUI 触摸事件转发到 C++（含多指 POINTER_DOWN/UP）
//
// 渲染流程：
//   1. ArkTS 分配 Uint8ClampedArray（ImageData.data）作为像素缓冲区
//   2. C++ 通过 N-API 拿到底层指针，包装成 HBITMAP + HDC
//   3. C++ SendMessage(WM_PAINT) → SOUI 窗口过程渲染到 HDC → 像素缓冲区
//   4. C++ 做 BGRA→RGBA 通道交换（Cairo ARGB32 vs ArkUI RGBA_8888）
//   5. ArkTS 把缓冲区同步到 PixelMap，再 ctx.drawImage 上屏（支持 dirty rect 局部刷新）
//      （putImageData 走软件路径全屏 ~40ms，drawImage(PixelMap) 走 GPU 纹理 <1ms）
//
// 事件流程：
//   ArkUI TouchEvent → SouiSurfaceView.onTouch → surf_onMotionEvent →
//   C++ SouiSurfaceProxy::onMotionEvent → 转换为 WM_LBUTTONDOWN/UP/MOUSEMOVE →
//   swinx 消息循环 → SOUI 窗口过程

import { INativeWindow } from './INativeWindow';
import { LengthMetricsUnit } from '@kit.ArkUI';
import { image } from '@kit.ImageKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { displaySync } from '@kit.ArkGraphics2D';
import { SouiPlatformBridge } from './SouiPlatformBridge';
import soui4ohos from 'libsoui4ohos.so';

// ArkUI TouchType 数值（与 C++ ARKUI_TOUCH_* 一致） 
const TOUCH_DOWN = 0;
const TOUCH_UP = 1;
const TOUCH_MOVE = 2;
const TOUCH_CANCEL = 3;

const DOMAIN = 0x5350;
const TAG = 'SouiSurface';

//
// SOUI 渲染 Surface 模型。@Observed 装饰使视觉属性变化可被 ArkUI 观察。
// 配合 SouiSurfaceView @Component + @ObjectLink 实现响应式 UI 更新。
//
// 对应 Android 端 SouiBaseSurface + SouiSurface 的全部职责。
@Observed
export class SouiSurface implements INativeWindow {
  // C++ 侧 HWND（= napi_ref 指针值，由 surf_nativeCreate 返回） 
  private mNativeId: number = 0;

  //
// Canvas 绘图上下文（onReady 后可用）。
//
// 单位模式设为 LengthMetricsUnit.PX（API 12+）：
//   - createImageData 的宽高参数以物理像素（px）解释 —— 传物理尺寸即分配
//     恰好物理分辨率的缓冲区，不再被当作 vp 放大 density² 倍内存
//   - drawImage 的 dx/dy/dw/dh 参数也以 px 解释 —— dirty rect（C++ 侧
//     InvalidateRect 物理像素）可直接透传，无需换算
// 与 C++/SOUI 侧的物理像素坐标系完全一致。
  mCtx: CanvasRenderingContext2D =
    new CanvasRenderingContext2D(new RenderingContextSettings(true), LengthMetricsUnit.PX);
  // 离屏像素缓冲区（RGBA_8888），物理分辨率（px） 
  mImageData: ImageData | null = null;
  //
// GPU 友好的上屏载体 PixelMap（与 mImageData 同尺寸，RGBA_8888）。
// C++ 把 SOUI 内容写入 mImageData.data 后，writePixelsSync/writeBufferToPixelsSync
// 同步到本 PixelMap，再由 mCtx.drawImage 上屏。drawImage 走 GPU 纹理路径
// （单帧 <1ms），替代 putImageData 的软件路径（全屏 ~40ms）。
  mPixelMap: image.PixelMap | null = null;
  // 当前缓冲区像素宽高（物理像素，vp 尺寸 ×density；用于复用缓冲区，避免抖动） 
  mWidth: number = 0;
  mHeight: number = 0;
  // Canvas 是否已就绪（onReady 触发后置 true） 
  mCanvasReady: boolean = false;

  // ===== 视觉属性（@Observed 使变化触发 UI 更新，对齐 Android View 的 setX/setLayoutParams）=====
  // 窗口位置（对应 Android View.setTranslationX/Y 或 LayoutParams.x/y） 
  x: number = 0;
  y: number = 0;
  // 窗口尺寸（0 表示填满父容器，对应 Android LayoutParams.MATCH_PARENT） 
  width: number = 0;
  height: number = 0;
  // 可见性 / 启用状态（对应 Android View.setVisibility/setEnabled） 
  visible: boolean = true;
  enabled: boolean = true;
  // 是否为子窗口（影响布局策略：'100%' vs 精确尺寸） 
  isChild: boolean = false;
  //
// 是否为纯容器（对应 Android SouiScreen 的 SouiBaseSurface：只做窗口容器，不渲染 bitmap）。
// 容器仍然注册 native 代理（screenStartup 需要其 HWND 作为屏幕窗口），
// 但 skip 一切 surf_render，避免屏幕容器产生多余的渲染层。
  isContainer: boolean = false;

  //
// 焦点键（供 ArkUI focusControl.requestFocus 定位 Canvas）。
// 由 SouiSurfaceView 通过 .id() 绑定到 Canvas（requestFocus 接受 .id() 值，非 .key()）；
// showSoftKeyboard 通过该键驱动 Canvas 获焦，配合 imc.attach 绑定自绘组件 IME。
  readonly focusKey: string;

  // 渲染调度标志：避免同一帧内重复排队 vsync 回调 
  private mRenderScheduled: boolean = false;
  //
// displaySync 实例：将渲染回调绑定到系统 vsync 信号。
// 替代 setTimeout(0) 驱动方式——setTimeout 不与 vsync 对齐，主线程饱和时
// 回调被推迟，导致动画中间帧丢失（只显示末帧）。
// displaySync 在每个 vsync 周期触发 frame 回调，保证渲染与合成同步。
  private mDisplaySync: displaySync.DisplaySync | null = null;
  // displaySync 是否已初始化（避免重复 create/start） 
  private mDisplaySyncInited: boolean = false;
  // 渲染序号（诊断用，统计 doRender 调用频率/时序） 
  private mRenderSeq: number = 0;
  // nativeInvalidate 调用序号（诊断用，看动画期间 invalidate 到达频率） 
  private mInvalidateSeq: number = 0;
  // 上一次触摸事件中已知的 pointer id 集合，用于检测 POINTER_DOWN/UP 
  private mPrevTouchIds: Set<number> = new Set<number>();

  constructor(initW: number = 0, initH: number = 0, isChild: boolean = false,
              isContainer: boolean = false) {
    this.width = initW;
    this.height = initH;
    this.isChild = isChild;
    this.isContainer = isContainer;
    // 委托 SouiPlatformBridge 创建 native 引用（对应 Android SouiBaseSurface.init）
    this.mNativeId = SouiPlatformBridge.instance().createNative(this);
    // 焦点键唯一：用 HWND 拼接，保证同屏多 Surface 不冲突
    this.focusKey = `soui-surface-${this.mNativeId}`;
  }

  // ============ 焦点控制（供 SouiPlatformBridge.showSoftKeyboard 调用）============ 

  //
// 请求 ArkUI 焦点到本控件 Canvas（配合 imc.attach 绑定自绘组件 IME）。
// 必须在 UI 线程调用（由 showSoftKeyboard 经 setTimeout(0) 保证）。
  public requestFocus(): void {
    try {
      focusControl.requestFocus(this.focusKey);
    } catch (e) {
      console.error('SouiSurface.requestFocus failed:', e);
    }
  }

  // ============================= INativeWindow ============================= 

  nativeGetHwnd(): number { return this.mNativeId; }

  nativeDestroy(): void {
    if (this.mDisplaySync) {
      this.mDisplaySync.stop();
      this.mDisplaySync = null;
      this.mDisplaySyncInited = false;
    }
    if (this.mNativeId !== 0) {
      SouiPlatformBridge.instance().destroyNative(this.mNativeId);
      this.mNativeId = 0;
    }
    if (this.mPixelMap) {
      this.mPixelMap.release();
      this.mPixelMap = null;
    }
    this.mImageData = null;
    this.mCanvasReady = false;
  }

  //
// 请求局部重绘（由 C++ 层通过 N-API 回调调用）。
// 对齐 Android SouiBaseSurface.requestInvalidate + View.invalidate(l,t,r,b)。
// dirty rect 不在 TS 层保存：C++ 侧 InvalidateRect → GetUpdateRgn 统一维护，
// doRender 时 surf_render 返回本次实际重绘区域，直接用它局部上屏。
  nativeInvalidate(left: number, top: number, right: number, bottom: number): void {
    this.mInvalidateSeq++;
    //hilog.info(DOMAIN, TAG,`nativeInvalidate#${this.mInvalidateSeq} rect=[${left},${top},${right},${bottom}] scheduled=${this.mRenderScheduled} t=${Date.now()}`);
    this.scheduleRender();
  }

  nativeShow(cmdShow: number): boolean {
    this.visible = (cmdShow !== 0);
    // 隐藏→显示：ArkUI 不会自动触发 Canvas 重绘，主动调度一次渲染
    // （Win32 ShowWindow(SW_SHOW) 会触发 WM_PAINT；这里等价补偿）
    if (this.visible) this.scheduleRender();
    return true;
  }

  //
// 移动并调整窗口大小。
// 对齐 Android NativeWindowDelegate.nativeMove → SouiAbsLayout.updateChildFrame。
// @Observed 使 x/y/width/height 变化自动触发 SouiSurfaceView 重新布局。
  nativeMove(x: number, y: number, width: number, height: number): boolean {
    this.x = x;
    this.y = y;
    if (width > 0) this.width = width;
    if (height > 0) this.height = height;
    // 同步 swinx wndObj->rc（物理像素）。
    // ArkTS moveWindow/nativeMove 只更新 ArkUI .position 属性，不通知 C++ swinx，
    // 导致 swinx wndObj->rc 位置停留在旧值（如 SHostWnd Create 后的 (-50,-50)），
    // 后续 ClientToScreen 用错误的 rc.left/top 计算 combobox 下拉屏幕坐标。
    // 通过 moveWindowNative 触发 WM_MOVE/WM_SIZE 同步 swinx rc。
    // 不会循环：C++ SetWindowPos → ArkTS setWindowPosEx/setWindowSize → nativeSetPosition/nativeSetSize
    // 只更新 ArkTS 属性，不再回调 C++。
    if (this.mNativeId !== 0) {
      const bridge = SouiPlatformBridge.instance();
      const xPx = Math.round(bridge.vp2px(x));
      const yPx = Math.round(bridge.vp2px(y));
      const wPx = (width > 0) ? Math.round(bridge.vp2px(width)) : 0;
      const hPx = (height > 0) ? Math.round(bridge.vp2px(height)) : 0;
      soui4ohos.moveWindowNative(this.mNativeId, xPx, yPx, wPx, hPx);
    }
    // 尺寸从 0 变为有效值（如 tooltip 首次 SetWindowPos）→ 调度渲染
    if (this.width > 0 && this.height > 0 && this.visible) this.scheduleRender();
    return true;
  }

  nativeSetSize(cx: number, cy: number): boolean {
    if (cx > 0) this.width = cx;
    if (cy > 0) this.height = cy;
    if (this.width > 0 && this.height > 0 && this.visible) this.scheduleRender();
    return true;
  }

  nativeSetPosition(x: number, y: number): boolean {
    this.x = x;
    this.y = y;
    return true;
  }

  nativeIsVisible(): boolean { return this.visible; }

  nativeEnable(enabled: boolean): boolean {
    this.enabled = enabled;
    return true;
  }

  nativeIsEnabled(): boolean { return this.enabled; }

  nativeGetWindow(_hWnd: number, _code: number): number {
    // SOUI 自绘 Surface 不处理原生控件 GetWindow，默认 return 0
    return 0;
  }

  nativeSendMessage(_msg: number, _wParam: number, _lParam: number): number {
    // SOUI 自绘 Surface 不处理原生控件消息，默认 return 0
    return 0;
  }

  asView(): object { return this; }

  // ============================= 渲染调度 ============================= 

  //
// 调度一次渲染任务（去重：同一帧多次请求合并为一次）。
// 对应 Android View.invalidate() → 调度 onDraw()。
//
// 渲染守卫（对齐 Android"只有可见且有内容的窗口才绘制"）：
//   - isContainer：屏幕容器不渲染 bitmap（渲染只发生在 maindlg / popup / tooltip 等真窗口上）
//   - !visible：隐藏窗口不渲染（ShowWindow 时 nativeShow 会补偿触发）
//   - width/height ≤ 0：尚未被 SOUI 定位的 0x0 窗口（如 tooltip host）不渲染
  scheduleRender(): void {
    if (this.mRenderScheduled) {
      //hilog.info(DOMAIN, TAG, `scheduleRender: DEDUPED (already scheduled) t=${Date.now()}`);
      return;
    }
    if (this.isContainer || !this.visible) return;
    if (this.width <= 0 || this.height <= 0) return;
    if (!this.mCanvasReady || this.mNativeId === 0) return;
    this.mRenderScheduled = true;
    this.ensureDisplaySync();
    //hilog.info(DOMAIN, TAG, `scheduleRender: QUEUED displaySync(vsync) t=${Date.now()}`);
  }

  //
// 初始化 displaySync（延迟到首次 scheduleRender，确保在 UI 线程上下文创建）。
// setExpectedFrameRateRange 设 30-60fps，on('frame') 回调在 vsync 到来时触发 doRender。
  private ensureDisplaySync(): void {
    if (this.mDisplaySyncInited) return;
    this.mDisplaySyncInited = true;
    this.mDisplaySync = displaySync.create();
    this.mDisplaySync.setExpectedFrameRateRange({ min: 30, max: 60, expected: 60 });
    this.mDisplaySync.on('frame', () => {
      if (this.mRenderScheduled) {
        this.mRenderScheduled = false;
        this.doRender();
      }
    });
    this.mDisplaySync.start();
  }

  //
// 执行一次渲染：surf_render → 同步 PixelMap → drawImage。
// 局部刷新以 surf_render 返回的更新矩形为准（C++ GetUpdateRgn 单一数据源，
// TS 层不再维护脏矩形列表）；空矩形（w/h<=0）表示本次无内容需上屏，直接跳过。
//
// 上屏载体选型（基于 refresh-test 实测）：
//   - putImageData 走软件路径，全屏 ~40ms（瓶颈）
//   - drawImage(PixelMap) 走 GPU 纹理路径，单帧 <1ms
// 故改用 PixelMap 作 GPU 友好载体：C++ 写 mImageData.data → writePixelsSync/
// writeBufferToPixelsSync 同步到 mPixelMap → drawImage 上屏。
  private doRender(): void {
    if (this.isContainer) return;
    if (!this.mCanvasReady || this.mNativeId === 0) return;
    if (this.mWidth <= 0 || this.mHeight <= 0) return;
    if (!this.mImageData || !this.mPixelMap) return;

    this.mRenderSeq++;
    const seq = this.mRenderSeq;
    const t0 = Date.now();

    // C++ 把 SOUI 内容写入 ImageData.data（Uint8ClampedArray），
    // 并返回本次实际重绘的更新区域 [l, t, r, b]（物理像素，客户区坐标）
    // 注意：BGRA→RGBA 通道交换已在 C++ 侧临时屏蔽（#if 0），颜色 B/R 会互换
    const rc: number[] = soui4ohos.surf_render(
      this.mNativeId, this.mImageData.data, this.mWidth, this.mHeight) as number[];
    const t1 = Date.now();

    // 同步渲染结果到 PixelMap 并 drawImage 上屏（仅更新矩形覆盖的子区）
    let syncMs = 0;
    const dx = (rc && rc.length >= 4) ? Math.max(0, Math.floor(rc[0])) : 0;
    const dy = (rc && rc.length >= 4) ? Math.max(0, Math.floor(rc[1])) : 0;
    const dw = (rc && rc.length >= 4)
      ? Math.max(0, Math.min(this.mWidth, Math.floor(rc[2])) - dx) : 0;
    const dh = (rc && rc.length >= 4)
      ? Math.max(0, Math.min(this.mHeight, Math.floor(rc[3])) - dy) : 0;

    if (dw > 0 && dh > 0) {
      // 从 mImageData.data（整缓冲区，行步幅 mWidth*4）中取更新子区写入 PixelMap 同位置。
      // PX 单位上下文：drawImage 的 dx/dy/dw/dh 按物理像素解释，与 C++ 返回的
      // 更新矩形同坐标系，直接透传无需换算。
      // 注意：目标位置 (dx,dy) 必须等于更新矩形原点——若写死 (0,0) 会把
      // 源缓冲区中 (dx,dy) 起的局部内容错误地画到窗口左上角，造成刷新错位。
      const area: image.PositionArea = {
        pixels: this.mImageData.data.buffer,
        offset: (dy * this.mWidth + dx) * 4,
        stride: this.mWidth * 4,
        region: { x: dx, y: dy, size: { width: dw, height: dh } }
      };
      this.mPixelMap.writePixelsSync(area);
      // 先 clearRect 清除旧像素，再 drawImage。
      // drawImage 默认 source-over 混合，若新帧含半透明像素（抗锯齿/透明背景），
      // 旧内容会透过来造成文字叠加。clearRect 使目标变透明，drawImage 后等价于直接替换。
      this.mCtx.clearRect(dx, dy, dw, dh);
      this.mCtx.drawImage(this.mPixelMap, dx, dy, dw, dh, dx, dy, dw, dh);
    }
    const t2 = Date.now();
    syncMs = t2 - t1;

    // 上屏耗时分解：surf=C++ 渲染, sync=PixelMap同步+drawImage, total=doRender 总时长
    //hilog.info(DOMAIN, TAG,`doRender#${seq} surf=${t1 - t0}ms sync+draw=${syncMs}ms total=${t2 - t0}ms rect=[${dx},${dy},${dw},${dh}] t=${t0}`);
  }

  // ============================= 内部方法（供 SouiSurfaceView 调用）============================= 

  //
// 创建或复用 ImageData（尺寸匹配时复用避免内存抖动，对齐 Android createOffscreenBitmap）。
// 入参为物理像素（px）。在 PX 单位上下文下，createImageData 的参数即按 px 解释，
// 分配出的缓冲区恰好为物理分辨率（mImageData.width/height === width/height）。
  allocateImageData(width: number, height: number): void {
    if (width <= 0 || height <= 0) return;
    if (this.mImageData
        && this.mImageData.width === width
        && this.mImageData.height === height
        && this.mPixelMap) {
      return;
    }
    this.mImageData = this.mCtx.createImageData(width, height);
    // 同步构建 GPU 友好的 PixelMap（RGBA_8888，与 ImageData 同尺寸）
    const opts: image.InitializationOptions = {
      editable: true,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      size: { width, height }
    };
    if (this.mPixelMap) {
      this.mPixelMap.release();
    }
    this.mPixelMap = image.createPixelMapSync(this.mImageData.data.buffer, opts);
  }

  // Canvas onReady 回调（由 SouiSurfaceView 触发）。 
  onCanvasReady(): void {
    this.mCanvasReady = true;
    if (this.mWidth > 0 && this.mHeight > 0 && !this.mImageData && !this.isContainer) {
      this.allocateImageData(this.mWidth, this.mHeight);
    }
    this.scheduleRender();
  }

  //
// Canvas onAreaChange 回调（由 SouiSurfaceView 触发）。
// 对齐 Android SouiSurface.onSizeChanged：
//   1. createOffscreenBitmap(w,h) → allocateImageData
//   2. nativeOnSizeChanged(nativeId, w, h) → surf_onSizeChanged
//   3. 旧 dirty rect 失效（尺寸变化导致旧坐标无意义），清空后触发全量重绘
//
// 单位约定：入参 w/h 为逻辑像素（vp，ArkUI onAreaChange 单位）。
// 本方法把缓冲区尺寸与通知 C++ 的尺寸统一为物理像素（px）：
//   - 优先取 Canvas 实际后备尺寸（PX 单位上下文下 ctx.width/height 即物理像素），
//     彻底避免 vp↔px 换算精度导致分配比实际少像素
//   - Canvas 未就绪（ctx 尺寸为 0）时按 w×density 向上取整估算，保证 ≥ 后备尺寸
//   - C++ 侧（surf_onSizeChanged / surf_render / InvalidateRect dirty）均为物理像素
// 渲染链路（surf_render 写入 → putImageData 上屏）全程物理坐标系，无需再换算。
  onAreaChanged(w: number, h: number): void {
    if (w <= 0 || h <= 0) return;
    // 物理像素尺寸：ctx.width/height 为 PX 单位下 Canvas 后备像素尺寸
    let pw = Math.round(this.mCtx.width);
    let ph = Math.round(this.mCtx.height);
    if (pw <= 0 || ph <= 0) {
      // Canvas 尚未绑定（onReady 前）：vp × density 向上取整，保证不差像素
      const density = SouiPlatformBridge.instance().getDensity();
      pw = Math.ceil(w * density);
      ph = Math.ceil(h * density);
    }
    if (pw === this.mWidth && ph === this.mHeight) return;
    this.mWidth = pw;
    this.mHeight = ph;
    // 1) 分配新尺寸的像素缓冲区（物理分辨率；对齐 Android createOffscreenBitmap；容器不分配）
    if (!this.isContainer) this.allocateImageData(pw, ph);
    // 2) 无需手动置脏：surf_onSizeChanged → C++ SetWindowPos(WM_SIZE) →
    //    InvalidateRect(整窗)，doRender 时 surf_render 自然返回全量更新矩形
    // 3) 通知 C++ 层尺寸变化（物理像素，对应 Android nativeOnSizeChanged → C++ OnSizeChanged）
    if (this.mNativeId !== 0) {
      soui4ohos.surf_onSizeChanged(this.mNativeId, pw, ph);
    }
    // 4) 调度渲染（dirty 队列非空 → doRender 时全量 drawImage；容器/隐藏/0 尺寸由守卫拦下）
    this.scheduleRender();
  }

  //
// 触摸事件处理（由 SouiSurfaceView.onTouch 转发）。
// 对齐 Android SouiBaseSurface.dispatchTouchEventToNative：
//   - DOWN/UP/CANCEL：仅处理主指针，按下时自动请求焦点
//   - MOVE：逐指针转发
//   - 支持多指 POINTER_DOWN/UP（通过 touches 数量变化检测）
//   - 支持 SetCapture 重定向：捕获窗口优先接收事件
//
// ArkUI TouchEvent → Android MotionEvent action 映射：
//   - type=Down, 无历史指针                  → ACTION_DOWN(0)
//   - type=Down, 已有历史指针                 → ACTION_POINTER_DOWN(5)，新指针
//   - type=Up,   剩余指针(剔除离开者)=0       → ACTION_UP(1)
//   - type=Up,   剩余指针(剔除离开者)>0       → ACTION_POINTER_UP(6)，离开指针
//   - type=Move                             → ACTION_MOVE(2)，逐指针
//   - type=Cancel                           → ACTION_CANCEL(3)
//
// 注意：ArkUI Up 事件的 touches 仍包含即将离开的触点（真机验证），
// 判定"最后一个指针离开"必须从 touches 中剔除离开指针后再数剩余。
//
// 指针定位对齐 Android event.getActionIndex()：POINTER_DOWN/UP 的触点坐标
// 取 changedTouches（本次变化的触点），而不是 touches[0]（剩余/全部触点）。
  onTouchEvent(event: TouchEvent): void {
    if (this.mNativeId === 0) return;

    const eventType = event.type as number;
    const timestamp = Date.now();
    const touches = event.touches;
    if (!touches || touches.length === 0) return;
    // changedTouches：本次事件中发生变化的触点（对齐 Android getActionIndex 定位的指针）
    const changed = event.changedTouches ?? [];

    const bridge = SouiPlatformBridge.instance();
    const metaState = 0;
    const buttonState = 0;
    const vscroll = 0;
    const hscroll = 0;

    // 计算实际 action：根据 type + touches 数量变化推断 POINTER_DOWN/UP
    let action: number;
    let primaryTouch = touches[0];

    if (eventType === TOUCH_DOWN) {
      // 检测是否是首个指针 vs 后续指针
      const prevCount = this.mPrevTouchIds.size;
      const currentIds = new Set<number>(touches.map(t => (t.id ?? 0) as number));
      // 定位本次新增指针：优先 changedTouches（Down 事件中即新按下的触点），
      // 对齐 Android event.getActionIndex()；退回 touches 中首个不在历史集合的指针
      const newTouches = changed.length > 0
        ? changed
        : touches.filter(t => !this.mPrevTouchIds.has((t.id ?? 0) as number));
      if (newTouches.length > 0) {
        primaryTouch = newTouches[0];
      }
      // 更新当前指针集合
      this.mPrevTouchIds = currentIds;
      if (prevCount === 0) {
        // 首个指针按下 → ACTION_DOWN
        action = 0; // MotionEvent.ACTION_DOWN
      } else {
        // 后续指针按下 → ACTION_POINTER_DOWN
        action = 5; // MotionEvent.ACTION_POINTER_DOWN
      }
      // 按下时自动请求焦点（对齐 Android ACTION_DOWN → requestFocus）
      if (action === 0) {
        if (bridge.getFocus() !== this.mNativeId) {
          bridge.setFocus(this.mNativeId);
        }
      }
    } else if (eventType === TOUCH_UP) {
      // ArkUI Up 事件的 touches 仍包含即将离开的触点（真机日志证实：
      // 单指点按曾被误判为 POINTER_UP，右键语义错乱且左键永不抬起）。
      // 离开指针：优先 changedTouches[0]；否则用 历史集合-当前集合 差集推断。
      let departTouch: TouchObject | undefined = changed.length > 0 ? changed[0] : undefined;
      const currentIds = new Set<number>(touches.map(t => (t.id ?? 0) as number));
      if (!departTouch) {
        for (const t of touches) {
          const tid = (t.id ?? 0) as number;
          if (!this.mPrevTouchIds.has(tid) || currentIds.size === 1) {
            departTouch = t;
            break;
          }
        }
      }
      if (departTouch) {
        primaryTouch = departTouch;
      }
      // 剩余指针 = 当前集合剔除离开指针（touches 含离开触点，必须减去）
      const remaining = new Set<number>(currentIds);
      if (departTouch) {
        remaining.delete((departTouch.id ?? 0) as number);
      }
      this.mPrevTouchIds = remaining;
      if (remaining.size === 0) {
        // 最后一个指针离开 → ACTION_UP
        action = 1; // MotionEvent.ACTION_UP
        this.mPrevTouchIds.clear();
      } else {
        // 中间指针离开 → ACTION_POINTER_UP
        action = 6; // MotionEvent.ACTION_POINTER_UP
      }
    } else if (eventType === TOUCH_CANCEL) {
      action = 3; // MotionEvent.ACTION_CANCEL
      this.mPrevTouchIds.clear();
    } else if (eventType === TOUCH_MOVE) {
      action = 2; // MotionEvent.ACTION_MOVE
      // 更新指针集合（Move 期间指针 id 不变）
      const currentIds = new Set<number>(touches.map(t => (t.id ?? 0) as number));
      this.mPrevTouchIds = currentIds;
    } else {
      // 未知事件类型，忽略
      return;
    }

    // ---- SetCapture 重定向：捕获窗口优先接收事件（对齐 Android tryDispatchCapturedMotion）----
    const captureHwnd = bridge.getCapture();
    if (captureHwnd !== 0 && captureHwnd !== this.mNativeId) {
      // 有捕获窗口且不是自己：重定向事件到捕获窗口
      const capturedWnd = bridge.findViewByHandle(captureHwnd);
      if (capturedWnd && capturedWnd instanceof SouiSurface) {
        // 将事件转发给捕获窗口（坐标转换：本 Surface 局部 → 屏幕坐标 → 捕获窗口局部）
        // ArkUI TouchObject 的 x/y 是相对当前组件的局部坐标（逻辑像素 vp），
        // C++ 侧需要物理像素：统一 ×density 后转发
        // （由于无法直接拿到屏幕坐标，屏幕坐标换算在 C++ 侧处理）
        const target = capturedWnd as SouiSurface;
        if (target.mNativeId !== 0) {
          const density = bridge.getDensity();
          soui4ohos.surf_onMotionEvent(
            target.mNativeId, action, primaryTouch.x * density, primaryTouch.y * density,
            (primaryTouch.id ?? 0) as number, buttonState, vscroll, hscroll, metaState, timestamp
          );
        }
        // 捕获窗口已处理，本窗口不再处理
        this.scheduleRender();
        return;
      }
      // 捕获窗口已销毁或非 Surface：兜底释放捕获
      bridge.releaseCapture();
    }

    // ---- 分发事件到 C++ 层（触摸坐标为逻辑像素 vp，×density 转物理像素）----
    const density = bridge.getDensity();
    if (action === 2 // MOVE 
) {
      // 移动：逐指针转发（对齐 Android ACTION_MOVE 循环 pointerCount）
      for (const t of touches) {
        soui4ohos.surf_onMotionEvent(
          this.mNativeId, action, t.x * density, t.y * density, (t.id ?? 0) as number,
          buttonState, vscroll, hscroll, metaState, timestamp
        );
      }
    } else {
      // Down/Up/Cancel/PointerDown/PointerUp：只处理 primary 指针
      soui4ohos.surf_onMotionEvent(
        this.mNativeId, action, primaryTouch.x * density, primaryTouch.y * density,
        (primaryTouch.id ?? 0) as number, buttonState, vscroll, hscroll, metaState, timestamp
      );
    }

    // 触摸后立即重绘，确保 UI 反映最新状态
    this.scheduleRender();
  }

  //
// 按键事件处理（由 SouiSurfaceView.onKeyEvent 转发）。
// 对齐 Android SouiBaseSurface.dispatchKeyEvent。
//
// keyCode 直接透传 ArkTS MMI KeyCode 枚举（2000+ 段，不是 Android 的 7-67 段），
// C++ convertKeyCode 专门按此枚举查表，映射到 Win32 VK_*。
  onKeyEvent(event: KeyEvent): boolean {
    if (this.mNativeId === 0) return false;

    // ArkUI KeyEvent: type=1=Down, type=2=Up；映射到 C++ action（0=Down,1=Up,2=Multiple）
    const action = (event.type === 1) ? 0 : 1;
    const keyCode = event.keyCode;
    // API 24 ArkUI KeyEvent 无 code 字段，scanCode 置 0（keyCode 已单独传递）
    const scanCode = 0;
    const unicodeChar = 0;
    const repeatCount = 0;
    const metaState = 0;
    const flags = 0;
    const timestamp = Date.now();

    hilog.info(DOMAIN, TAG,
      'onKeyEvent: hwnd=' + this.mNativeId +
      ' keyCode=' + keyCode + '(' + (event.keyText ?? '') +
      ') action=' + action + ' type=' + event.type);

    const handled = soui4ohos.surf_onKeyEvent(
      this.mNativeId, keyCode, action, metaState,
      repeatCount, scanCode, unicodeChar, flags, timestamp
    );
    this.scheduleRender();
    return Boolean(handled);
  }
}
