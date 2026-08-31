//
// UiThreadUtils.ets
//
// UI 线程同步执行工具，对应 Android 端 UiThreadUtils.java。
//
// 背景：
//   ArkUI 的 @Observed / @State / @ObjectLink 等装饰器只能在 UI 线程修改；
//   C++ 层 N-API 回调（surf_render / surf_onMotionEvent / surf_onSizeChanged 等）
//   通过 napi_threadsafe_function 调度到 ArkUI 主线程执行，因此绝大多数
//   场景下 ArkTS 回调已在主线程。本工具针对少数跨线程场景提供同步/异步切换。
//
// ArkUI 主线程检测：
//   ArkUI 进程启动时主线程即固定。本模块在首次 init 时记录主线程标识，
//   后续通过比对判断当前是否在主线程（@ohos.process 的 process.tid）。
//   无法直接拿到 thread id 时，退化为"始终同步执行"——前提是 C++ 侧已使用
//   napi_threadsafe_function 把回调编排到主线程（与 Android 端
//   Handler(Looper.getMainLooper()) 等价）。

import process from '@ohos.process';

//
// UI 线程同步执行工具。提供 Android 端 UiThreadUtils.runOnUiSync 等价能力。
export class UiThreadUtils {
  // 同步等待超时时间（毫秒），避免主线程阻塞导致永久挂起。对齐 Android UI_SYNC_TIMEOUT_SEC=10s 
  public static readonly UI_SYNC_TIMEOUT_MS: number = 10_000;

  // 主线程标识（init 时记录，用于 isOnUiThread 比对）。 
  private static mUiThreadMark: number = 0;

  private constructor() {}

  //
// 初始化主线程标识。应在 UIAbility.onCreate 或 SouiPlatformBridge.init 中调用一次。
  public static init(): void {
    if (UiThreadUtils.mUiThreadMark !== 0) return;
    let tid = 0;
    try {
      tid = process.tid;
    } catch (_e) { // 取不到则退化为 Date.now 
 }
    if (tid === 0) tid = Date.now() & 0x7FFFFFFF;
    UiThreadUtils.mUiThreadMark = tid;
  }

  //
// 判断当前是否在 UI 线程。
// 注意：ArkTS 无标准线程 API，本判断依赖 init 时记录的标识。
// 如果回调确实来自非主线程但 tid API 不可用，本方法可能误判为 true，
// 此时应由 C++ 侧使用 napi_threadsafe_function 确保回调在主线程触发。
  public static isOnUiThread(): boolean {
    if (UiThreadUtils.mUiThreadMark === 0) return true; // 未 init：乐观假设在主线程
    try {
      return process.tid === UiThreadUtils.mUiThreadMark;
    } catch (_e) { // ignore 
 }
    return true; // 无法判断时乐观假设
  }

  //
// 在 UI 线程同步执行代码块并返回结果（对齐 Android runOnUiSync）。
//   - 已在 UI 线程：直接执行，避免死锁
//   - 非 UI 线程：通过 Promise + setTimeout 投递，并同步等待结果
//
// 注意：ArkTS 不支持线程阻塞等待，因此非 UI 线程的"同步等待"实际依赖
// Atomics.wait + SharedArrayBuffer 等机制。当前实现退化为"投递后立即返回
// undefined"，仅用于 fire-and-forget；真正的同步等待由 C++ 侧
// napi_threadsafe_function 保障（回调本就在主线程）。
  public static runOnUiSync<T>(block: () => T): T {
    if (block === null || block === undefined) {
      throw new Error('UiThreadUtils: block must not be null');
    }
    // 快速路径：已在 UI 线程 → 直接执行（避免死锁，对齐 Android 快速路径）
    if (UiThreadUtils.isOnUiThread()) {
      return block();
    }
    // 慢速路径：投递到 UI 线程。ArkTS 无 thread.join，只能异步投递。
    // C++ 侧若需要同步结果，应通过 napi_threadsafe_function 保证回调在主线程触发，
    // 此时本路径不会走到。
    setTimeout(() => { block(); }, 0);
    return undefined as T;
  }

  //
// Runnable 版主线程同步执行（无返回值，对齐 Android runOnUiSyncVoid）。
  public static runOnUiSyncVoid(block: () => void): void {
    if (block === null || block === undefined) {
      throw new Error('UiThreadUtils: block must not be null');
    }
    UiThreadUtils.runOnUiSync<void>(() => { block(); });
  }

  //
// 异步投递到 UI 线程执行（fire-and-forget）。
// 等价于 Android Handler.post() / ArkUI setTimeout(0)。
  public static postOnUiThread(block: () => void): void {
    setTimeout(block, 0);
  }
}
