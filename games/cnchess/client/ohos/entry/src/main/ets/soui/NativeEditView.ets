//
// NativeEditView.ets
//
// 对应 Android 端 NativeEditView.java。
//
// 当 className 为 "Edit"/"EditableView"/"SouiEditView" 时，SouiPlatformBridge
// 创建一个 NativeEditView（而不是 SouiSurface）。它把 ArkUI 原生 TextInput
// 作为真正的输入控件（和 Android EditText 一样），并将输入变化用 string slot
// 机制传递给 C++ 侧。C++ SEdit::OnKillFocus/WM_GETTEXT 再从 slot 回读，
// 保证输入法、拼写检查、系统键盘行为完全使用 ArkUI 原生能力。
//
// 设计要点（与 Android 端严格对齐）：
//   1. @Observed 类，视觉属性（x/y/width/height/visible/enabled）变化触发 UI 更新
//   2. nativeId = SouiPlatformBridge.createNative(this)，HWND = nativeId
//   3. onChange → EN_UPDATE + EN_CHANGE（inTextOp 嵌套计数合并通知）
//   4. onFocus → notifyFocusGained + EN_SETFOCUS；onBlur → EN_KILLFOCUS
//   5. nativeSendMessage 实现 WM_SETTEXT/WM_GETTEXT/EM_* 等 Win32 编辑消息
//   6. 字符串入参：readCxxStringSlot(slotId)；字符串出参：writeCxxStringSlot(slotId)
//
// 响应式机制（对齐 SouiSurface）：
//   - @Observed 装饰使视觉属性变化被 NativeEditViewView @Component 观察
//   - 配合 NativeEditViewView @ObjectLink 实现 x/y/width/height/visible 变化自动重绘
//   - 事件回调通过公共方法暴露给 NativeEditViewView，由其 wire 到 TextInput

import { INativeWindow } from './INativeWindow';
import { SouiPlatformBridge } from './SouiPlatformBridge';
import { UiThreadUtils } from './UiThreadUtils';
import soui4ohos from 'libsoui4ohos.so';

// ===================== Win32 消息常量（与 swinx/include/winuser.h 对齐）=====================
const WM_GETTEXT          = 0x000D;
const WM_GETTEXTLENGTH    = 0x000E;
const WM_SETTEXT          = 0x000C;
const EM_GETSEL           = 0x00B0;
const EM_SETSEL           = 0x00B1;
const EM_GETLINECOUNT     = 0x00BA;
const EM_REPLACESEL       = 0x00C2;
const EM_GETMODIFY        = 0x00B8;
const EM_SETMODIFY        = 0x00B9;
const EM_UNDO             = 0x00C7;
const EM_CANUNDO          = 0x00CE;
const EM_EMPTYUNDOBUFFER  = 0x00CD;
const EM_LIMITTEXT        = 0x00C5;
const EM_SETREADONLY      = 0x00CF;
const EM_SETMARGINS       = 0x00D3;
const EM_GETMARGINS       = 0x00D4;
const EM_SETPASSWORDCHAR  = 0x00CC;
const EM_GETPASSWORDCHAR  = 0x00D2;
const EM_GETLINE          = 0x00C4;
const EM_LINEINDEX        = 0x00BB;
const EM_LINELENGTH       = 0x00C1;
const EM_LINEFROMCHAR     = 0x00C9;
const EM_SCROLLCARET      = 0x00B7;
const EM_GETLIMITTEXT     = 0x00D5;

// EN_* 通知码（反射给父窗口的 WM_COMMAND HIWORD(wParam)）
const EN_SETFOCUS   = 0x0100;
const EN_KILLFOCUS  = 0x0200;
const EN_CHANGE     = 0x0300;
const EN_UPDATE    = 0x0400;
const EN_MAXTEXT    = 0x0501;
const EN_SELCHANGE  = 0x0701;

// StringSlot 编码（与 C++ OhosPlatformAPI.h 严格对齐）
const SLOT_WORD_MASK = 0xFFFF;
const EM_GETLINE_SLOT_SHIFT = 16;

//
// 原生 EditText 控件实现。@Observed 装饰使视觉属性变化触发 UI 重新渲染。
// 实现 INativeWindow，通过 C++ 侧注册的窗口类 "Edit" 创建。
// 处理 Win32 风格的编辑控件消息（WM_SETTEXT/EM_GETSEL 等）。
@Observed
export class NativeEditView implements INativeWindow {
  // C++ 侧 HWND（= napi_ref 指针值，由 surf_nativeCreate 返回） 
  private mNativeId: number = 0;
  // 控件 ID（getId 对应值，用于 EN_* 通知的 wParam 编码） 
  private mCtrlId: number = 0;
  //
// 焦点键（供 ArkUI focusControl.requestFocus/clearFocus 定位 TextInput）。
// 由 NativeEditViewView 通过 .id() 绑定到 TextInput（requestFocus 接受 .id() 值，非 .key()）；
// showSoftKeyboard 通过该键驱动 IME 显隐（TextInput 获得焦点后 IME 自动弹出）。
  readonly focusKey: string;
  // TextInput 控制器（用于选区、光标控制）。供 NativeEditViewView 读取 
  mController: TextInputController = new TextInputController();
  // 当前文本内容。供 NativeEditViewView 读取 
  mText: string = '';
  // 字符数上限（EM_LIMITTEXT），0 表示不限制 
  private mMaxChars: number = 0;
  // 修改标记（EM_GETMODIFY/EM_SETMODIFY） 
  private mModified: boolean = false;
  // C++ 内部文本修改嵌套计数器，用于合并 EN_CHANGE 通知 
  mInNativeTextOp: number = 0;
  // 文本修改期间是否变脏 
  private mNativeTextDirty: boolean = false;
  // 是否只读。供 NativeEditViewView 读取以禁用 TextInput 
  mReadOnly: boolean = false;
  // 是否密码模式（非 0 表示密码字符）。供 NativeEditViewView 读取 
  mPasswordChar: number = 0;
  // 选区起止（TextInput onChange 不直接提供选区，用 caretPosition 维护） 
  private mSelectionStart: number = 0;
  private mSelectionEnd: number = 0;

  // ===== 视觉属性（@Observed 使变化触发 UI 更新，对齐 SouiSurface 与 Android View）=====
  // 窗口位置（对应 Android View.setTranslationX/Y 或 LayoutParams.x/y） 
  x: number = 0;
  y: number = 0;
  // 窗口尺寸（0 表示填满父容器，对应 Android LayoutParams.MATCH_PARENT） 
  width: number = 0;
  height: number = 0;
  // 可见性 / 启用状态（对应 Android View.setVisibility/setEnabled） 
  visible: boolean = true;
  enabled: boolean = true;
  // 透明度（ArkUI 特有，对应 Android View.setAlpha） 
  opacity: number = 1.0;
  // padding（EM_SETMARGINS 使用） 
  padLeft: number = 0;
  padRight: number = 0;

  constructor(ctrlId: number = 0) {
    this.mCtrlId = ctrlId;
    // 委托 SouiPlatformBridge 创建 native 引用（对应 Android SouiPlatformBridge.createNative）
    this.mNativeId = SouiPlatformBridge.instance().createNative(this);
    // 焦点键唯一：用 HWND 拼接，保证同屏多 Edit 不冲突
    this.focusKey = `soui-edit-${this.mNativeId}`;
  }

  // ============ 焦点/IME 控制（供 SouiPlatformBridge.showSoftKeyboard 调用）============ 

  //
// 请求 ArkUI 焦点到本控件 TextInput（IME 自动弹出）。
// 必须在 UI 线程调用（由 showSoftKeyboard 经 setTimeout(0) 保证）。
  public requestFocus(): void {
    try {
      focusControl.requestFocus(this.focusKey);
    } catch (e) {
      console.error('NativeEditView.requestFocus failed:', e);
    }
  }

  // ============ INativeWindow ============ 

  nativeGetHwnd(): number { return this.mNativeId; }

  nativeDestroy(): void {
    if (this.mNativeId !== 0) {
      SouiPlatformBridge.instance().destroyNative(this.mNativeId);
      this.mNativeId = 0;
    }
  }

  nativeInvalidate(_left: number, _top: number, _right: number, _bottom: number): void {
    // TextInput 自绘，无需手动 invalidate（ArkUI 自行管理重绘）
  }

  //
// 显示/隐藏窗口。
// @Observed 使 visible 变化自动触发 NativeEditViewView 重新渲染。
  nativeShow(cmdShow: number): boolean {
    this.visible = (cmdShow !== 0);
    return true;
  }

  //
// 移动并调整窗口大小。
// @Observed 使 x/y/width/height 变化自动触发 NativeEditViewView 重新布局。
  nativeMove(x: number, y: number, w: number, h: number): boolean {
    this.x = x;
    this.y = y;
    if (w > 0) this.width = w;
    if (h > 0) this.height = h;
    return true;
  }

  nativeSetSize(cx: number, cy: number): boolean {
    if (cx > 0) this.width = cx;
    if (cy > 0) this.height = cy;
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
    // ArkUI 无原生 GetWindow 等价物，返回 0
    return 0;
  }

  asView(): object { return this; }

  //
// 通用消息转发。字符串参数通过字符串槽交换，纯数值消息按 Win32 语义传递。
// 对应 Android NativeEditView.nativeSendMessage：UiThreadUtils.runOnUiSync(() -> handleMessageImpl)。
// C++ 层 N-API 回调可能在非 UI 线程触发，必须切到 UI 线程访问 ArkUI TextInput 状态。
  nativeSendMessage(msg: number, wParam: number, lParam: number): number {
    return UiThreadUtils.runOnUiSync(() => this.handleMessageImpl(msg, wParam, lParam));
  }

  // ============ EN_* 通知反射 ============ 

  //
// 向上反射一条 EN_* 通知到 C++ 父窗口。
// wParam = (ctrlId & 0xFFFF) | (code << 16)；lParam = hEdit
  private notifyEditEvent(code: number): void {
    if (this.mNativeId === 0) return;
    const wp = (this.mCtrlId & 0xFFFF) | ((code & 0xFFFF) << 16);
    soui4ohos.nativeNotifyEditEvent(this.mNativeId, code, wp, this.mNativeId);
  }

  // ============ inTextOp：C++ 内部文本修改保护区域 ============ 

  //
// 进入 C++ 内部文本修改保护区域：嵌套计数 + 合并 EN_CHANGE 通知。
// 在最外层 op 退出时，若有变脏，统一发一条 EN_CHANGE。
  private inTextOp(block: () => void): void {
    ++this.mInNativeTextOp;
    const prevDirty = this.mNativeTextDirty;
    this.mNativeTextDirty = false;
    try {
      block();
    } finally {
      const dirtyNow = this.mNativeTextDirty;
      this.mNativeTextDirty = prevDirty || dirtyNow;
      --this.mInNativeTextOp;
      if (this.mInNativeTextOp === 0 && dirtyNow) {
        this.notifyEditEvent(EN_CHANGE);
      }
    }
  }

  // ============ nativeSendMessage 消息实现主体 ============ 

  private handleMessageImpl(msg: number, wp: number, lp: number): number {
    switch (msg) {
      case WM_GETTEXTLENGTH:
        return this.mText.length;

      case EM_GETSEL: {
        // 打包为 64-bit 返回：低 32 bit = start, 高 32 bit = end
        // 对齐 Android: ((long)s & 0xFFFFFFFFL) | (((long)e & 0xFFFFFFFFL) << 32)
        // ArkTS number 是 IEEE 754 double，可精确表示 2^53 以内整数；
        // 但位运算 << 只用 32-bit，左移 32 位结果为 0，必须改用乘法。
        const s = Math.max(0, this.mSelectionStart) & 0xFFFFFFFF;
        const e = Math.max(0, this.mSelectionEnd) & 0xFFFFFFFF;
        return s + e * 0x100000000;
      }

      case EM_SETSEL: {
        const s = wp | 0;
        const e = lp | 0;
        const len = this.mText.length;
        const start = Math.max(0, Math.min(s, len));
        const end   = Math.max(0, Math.min(e, len));
        this.mSelectionStart = start;
        this.mSelectionEnd = end;
        // ArkUI TextInput 通过 controller 设置光标位置（选区 API 较弱，仅设置光标）
        try {
          if (start === end) {
            this.mController.caretPosition(start);
          }
        } catch (_e) { // ignore 
 }
        return 1;
      }

      case EM_GETLINECOUNT:
        // ArkUI TextInput 默认单行；多行模式返回行数=1
        return 1;

      case EM_GETMODIFY:
        return this.mModified ? 1 : 0;

      case EM_SETMODIFY:
        this.mModified = (wp !== 0);
        return 0;

      case EM_CANUNDO:
        // ArkUI 无公开 undo API，统一返回 0
        return 0;

      case EM_UNDO:
        // ArkUI 无公开 undo API，no-op
        return 0;

      case EM_EMPTYUNDOBUFFER:
        this.mModified = false;
        return 1;

      case EM_LIMITTEXT: {
        this.mMaxChars = wp | 0;
        if (this.mMaxChars > 0 && this.mText.length > this.mMaxChars) {
          const max = this.mMaxChars;
          this.inTextOp(() => { this.mText = this.mText.substring(0, max); });
        }
        return 0;
      }

      case EM_GETLIMITTEXT:
        return this.mMaxChars;

      case EM_SETREADONLY:
        this.mReadOnly = (wp !== 0);
        return 0;

      case EM_SETPASSWORDCHAR:
        this.mPasswordChar = wp | 0;
        return 0;

      case EM_GETPASSWORDCHAR:
        return this.mPasswordChar;

      case EM_SETMARGINS: {
        const flags = wp | 0;
        const lo = (lp & 0xFFFF) | 0;
        const hi = ((lp >> 16) & 0xFFFF) | 0;
        if ((flags & 1) !== 0) this.padLeft = lo;
        if ((flags & 2) !== 0) this.padRight = hi;
        return 0;
      }

      case EM_GETMARGINS: {
        const lo = this.padLeft & 0xFFFF;
        const hi = this.padRight & 0xFFFF;
        return ((lo & 0xFFFF) | ((hi & 0xFFFF) << 16)) >>> 0;
      }

      case EM_LINEINDEX:
        // 单行模式：行 0 起始偏移为 0；其它行返回 -1
        return (wp | 0) === 0 ? 0 : -1;

      case EM_LINELENGTH:
        // 单行模式：返回当前文本长度
        return this.mText.length;

      case EM_LINEFROMCHAR:
        // 单行模式：始终返回 0
        return 0;

      case EM_SCROLLCARET:
        // ArkUI TextInput 自带滚动，no-op
        return 1;

      // ============ 字符串入参消息：lParam = slotId ============
      case WM_SETTEXT: {
        const slotId = lp | 0;
        const text = SouiPlatformBridge.instance().readCxxStringSlot(slotId);
        this.inTextOp(() => { this.mText = text ?? ''; });
        this.mModified = true;
        return 1;
      }

      case EM_REPLACESEL: {
        const slotId = lp | 0;
        const insert = SouiPlatformBridge.instance().readCxxStringSlot(slotId) ?? '';
        const ss = Math.max(0, this.mSelectionStart);
        const ee = Math.max(0, this.mSelectionEnd);
        const a = Math.min(ss, ee);
        const b = Math.max(ss, ee);
        const next = this.mText.substring(0, a) + insert + this.mText.substring(b);
        const newSel = a + insert.length;
        this.inTextOp(() => {
          this.mText = next;
          this.mSelectionStart = newSel;
          this.mSelectionEnd = newSel;
        });
        this.mModified = true;
        return 0;
      }

      // ============ 字符串出参消息：lParam = outSlotId ============
      case WM_GETTEXT: {
        // wParam = cchTextMax（含 '\0'），lParam = outSlotId
        const maxChars = wp | 0;
        const slotId = lp | 0;
        let result = this.mText;
        if (maxChars > 0) {
          const end = Math.min(result.length, Math.max(0, maxChars - 1));
          result = result.substring(0, Math.max(0, end));
        }
        SouiPlatformBridge.instance().writeCxxStringSlot(slotId, result);
        return result.length;
      }

      case EM_GETLINE: {
        // wParam = lineNo；lParam = (maxChars << 16) | (slotId & 0xFFFF)
        const l = lp;
        const slotId = (l & SLOT_WORD_MASK) | 0;
        const maxChars = (l >>> EM_GETLINE_SLOT_SHIFT) | 0;
        // 单行模式：直接返回当前文本（按 maxChars 截断）
        let lineStr = this.mText;
        if (maxChars > 0) {
          lineStr = lineStr.substring(0, Math.min(lineStr.length, maxChars));
        }
        SouiPlatformBridge.instance().writeCxxStringSlot(slotId, lineStr);
        return lineStr.length;
      }

      default:
        return 0;
    }
  }

  // ============ TextInput 事件回调（由 NativeEditViewView wire 到 TextInput）============ 

  //
// TextInput.onChange 回调。处理用户输入文本变化。
// 对应 Android 端 EditText.onTextChanged → EN_UPDATE + EN_CHANGE。
  public onTextInputChange(value: string): void {
    // EM_LIMITTEXT 上限：超过 → EN_MAXTEXT + 回滚
    if (this.mMaxChars > 0 && value.length > this.mMaxChars) {
      this.notifyEditEvent(EN_MAXTEXT);
    }
    // 外部 IME/按键输入 → 反射 EN_UPDATE；
    // 嵌套文本操作内部 → 不逐条发，避免抖动
    if (this.mInNativeTextOp === 0) {
      this.notifyEditEvent(EN_UPDATE);
    }
    // 超长时裁剪
    let newText = value;
    if (this.mMaxChars > 0 && newText.length > this.mMaxChars) {
      ++this.mInNativeTextOp;
      try { newText = newText.substring(0, this.mMaxChars); }
      finally { --this.mInNativeTextOp; }
    }
    this.mText = newText;
    this.mModified = true;
    if (this.mInNativeTextOp === 0) {
      // 纯用户输入/IME → 直接反射 EN_CHANGE
      this.notifyEditEvent(EN_CHANGE);
    } else {
      // 内部文本 op → 标脏，等最外层 inTextOp 退出时合并 1 条
      this.mNativeTextDirty = true;
    }
  }

  //
// TextInput.onTextSelectionChange 回调。更新选区状态并反射 EN_SELCHANGE。
  public onTextInputSelection(selectionStart: number, selectionEnd: number): void {
    this.mSelectionStart = selectionStart;
    this.mSelectionEnd = selectionEnd;
    this.notifyEditEvent(EN_SELCHANGE);
  }

  //
// TextInput.onFocus 回调。同步焦点到 C++、反射 EN_SETFOCUS，并显式弹起软键盘。
  public onTextInputFocus(): void {
    if (this.mNativeId !== 0) {
      const bridge = SouiPlatformBridge.instance();
      if (bridge.getFocus() !== this.mNativeId) {
        bridge.setFocus(this.mNativeId);
      }
      bridge.notifyFocusGained(this.mNativeId);
      // 兜底：桥 setFocus 命中 mCurrentFocusHwnd===hwnd 快路返回时，applyFocusAndIme 不会执行，
      // 这里直接显式弹起 IME，确保不依赖桥副作用路径也能工作。
      // 注意：Demo 版 bridge.showSoftKeyboard 第一个参数是 HWND(number)，非 INativeWindow 对象。
      bridge.showSoftKeyboard(this.nativeGetHwnd(), true);
    }
    this.notifyEditEvent(EN_SETFOCUS);
  }

  //
// TextInput.onBlur 回调。反射 EN_KILLFOCUS，并显式收起软键盘。
  public onTextInputBlur(): void {
    if (this.mNativeId !== 0) {
      // 用户点击外部触发 blur：直接收起 IME（桥 setFocus→applyFocusAndIme 的 hide 路径
      // 不一定覆盖所有场景，例如用户没走 bridge.setFocus 直接点外部失焦）。
      // 注意：Demo 版 bridge.showSoftKeyboard 第一个参数是 HWND(number)，0 表示无焦点窗口。
      SouiPlatformBridge.instance().showSoftKeyboard(0, false);
    }
    this.notifyEditEvent(EN_KILLFOCUS);
  }
}
