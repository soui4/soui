/**
 * 原生 EditText 控件实现。实现 INativeWindow，通过 C++ 侧注册的窗口类 "Edit" 创建。
 * 处理 Win32 风格的编辑控件消息（WM_SETTEXT/EM_GETSEL 等）。
 */
package com.soui.android;

import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.EditText;

import java.lang.reflect.Method;
import java.util.function.Supplier;

@SuppressWarnings({"ViewConstructor", "unused"})
public class NativeEditView extends EditText implements INativeWindow {
    /** 反射缓存 TextView 的隐藏方法（canUndo/undo/forgetUndoRedoHistory）。 */
    private static final Method sTextView_canUndo;
    private static final Method sTextView_undo;
    private static final Method sTextView_forgetUndoRedo;
    static {
        Method canUndo = null, undo = null, forget = null;
        try {
            @SuppressWarnings("JavaReflectionMemberAccess")
            Class<?> tc = Class.forName("android.widget.TextView");
            try { canUndo = tc.getDeclaredMethod("canUndo"); canUndo.setAccessible(true); } catch (Throwable ignored) {}
            try { undo   = tc.getDeclaredMethod("undo");   undo  .setAccessible(true); } catch (Throwable ignored) {}
            try {
                forget = tc.getDeclaredMethod("forgetUndoRedoHistory");
                forget.setAccessible(true);
            } catch (Throwable ignored) {}
        } catch (Throwable ignored) {
            // Class.forName 理论不可能失败，保留兜底
        }
        sTextView_canUndo            = canUndo;
        sTextView_undo               = undo;
        sTextView_forgetUndoRedo     = forget;
    }
    /** 能否 undo：先反射 canUndo() → 否返回 false；不抛 NoSuchMethodError。 */
    private boolean canUndoReflect() {
        if (sTextView_canUndo != null) {
            try { return Boolean.TRUE.equals(sTextView_canUndo.invoke(this)); }
            catch (Throwable ignored) { /* IllegalAccess / InvocationTarget 等 */ }
        }
        return false;
    }
    /** 执行撤销操作。 */
    private void undoReflect() {
        if (sTextView_undo != null) {
            try { sTextView_undo.invoke(this); return; } catch (Throwable ignored) {}
        }
        // TextView 公开 API 1+：onTextContextMenuItem(id) — 多数 ROM 对 undo/redo 有实现。
        try { onTextContextMenuItem(android.R.id.undo); } catch (Throwable ignored) {}
    }
    /** 清空撤销缓冲区。 */
    private void forgetUndoRedoHistory() {
        if (sTextView_forgetUndoRedo != null) {
            try { sTextView_forgetUndoRedo.invoke(this); return; } catch (Throwable ignored) {}
        }
        // 没有公开清空 undo 栈的可靠 API；保持 no-op。
    }

    private final NativeWindowDelegate mDelegate = new NativeWindowDelegate(this);
    private long nativeId = 0;
    /** 文本变化监听器，用于反射 EN_CHANGE/EN_UPDATE 到 C++。 */
    private TextWatcher mChangeWatcher;
    /** C++ 内部文本修改嵌套计数器。 */
    private int mInNativeTextOp = 0;
    /** 文本修改期间是否变脏，用于合并 EN_CHANGE 通知。 */
    private boolean mNativeTextDirty = false;
    /** 字符数上限（EM_LIMITTEXT），0 表示不限制。 */
    private int mMaxChars = 0;
    /** 修改标记（EM_GETMODIFY/EM_SETMODIFY）。 */
    private boolean mModified = false;

    public NativeEditView(String initialText) {
        super(SouiPlatformBridge.getInstance().getContext());
        if (initialText != null) {
            setText(initialText);
        }
        nativeId = SouiPlatformBridge.getInstance().createNative(this);
        // 默认给 EditText 聚焦能力，确保 IME 可弹起；实际 (x,y,w,h) 会由 NativeMove/SetSize 再覆盖。
        setFocusable(true);
        setFocusableInTouchMode(true);
        // 获焦变化同步：Java 原生焦点（用户直接点击 EditText 不走 SouiBaseSurface）
        // 必须同步到 bridge 焦点 + C++ SWinx 焦点 + 投 WM_SETFOCUS/WM_KILLFOCUS
        setOnFocusChangeListener((v, hasFocus) -> {
            if (hasFocus && nativeId != 0L) {
                final SouiPlatformBridge bridge = SouiPlatformBridge.getInstance();
                if (bridge.getFocus() != nativeId) {
                    bridge.setFocus(nativeId);
                }
                SouiPlatformBridge.notifyFocusGained(nativeId);
            }
            // EN_SETFOCUS / EN_KILLFOCUS 反射
            notifyEditEvent(hasFocus ? EN_SETFOCUS : EN_KILLFOCUS);
        });
        // TextWatcher → EN_CHANGE / EN_UPDATE / EN_MAXTEXT
        mChangeWatcher = new TextWatcher() {
            @Override public void beforeTextChanged(CharSequence s, int st, int cnt, int after) { /* no-op */ }
            @Override public void onTextChanged(CharSequence s, int st, int before, int cnt) {
                // EM_LIMITTEXT 上限：超过 → EN_MAXTEXT + 回滚
                if (mMaxChars > 0 && s.length() > mMaxChars) {
                    notifyEditEvent(EN_MAXTEXT);
                }
                // 外部 IME/按键输入 → 直接反射 EN_UPDATE；
                // 嵌套文本操作内部 → 不逐条发，避免抖动
                if (mInNativeTextOp == 0) {
                    notifyEditEvent(EN_UPDATE);
                }
            }
            @Override public void afterTextChanged(Editable s) {
                if (mMaxChars > 0 && s.length() > mMaxChars) {
                    // 超长时裁剪：嵌套一层计数，裁剪本身再触发 afterTextChanged 不重复发
                    ++mInNativeTextOp;
                    try { s.delete(mMaxChars, s.length()); }
                    finally { --mInNativeTextOp; }
                }
                mModified = true;
                if (mInNativeTextOp == 0) {
                    // 纯用户输入/IME → 直接反射 EN_CHANGE
                    notifyEditEvent(EN_CHANGE);
                } else {
                    // 内部文本 op → 标脏，等最外层 inTextOp 退出时合并 1 条
                    mNativeTextDirty = true;
                }
            }
        };
        addTextChangedListener(mChangeWatcher);
    }

    /** 选区变化时通知 C++ 层。 */
    @Override
    protected void onSelectionChanged(int selStart, int selEnd) {
        super.onSelectionChanged(selStart, selEnd);
        notifyEditEvent(EN_SELCHANGE);
    }

    /** 向上反射 EN_* 通知到 C++ 父窗口。 */
    private static native void nativeNotifyEditEvent(long hEdit, int code, long wparam, long lparam);

    /** 向上反射一条 EN_* 通知到 C++ 父窗口。 */
    private void notifyEditEvent(int code) {
        if (nativeId == 0L) return;
        final int ctrlId = (getId() == View.NO_ID) ? 0 : (getId() & 0x7FFFFFFF);
        final long wp = (ctrlId & 0xFFFFL) | (((long)code << 16) & 0xFFFF0000L);
        nativeNotifyEditEvent(nativeId, code, wp, nativeId);
    }

    /** 进入 C++ 内部文本修改保护区域，嵌套计数并合并 EN_CHANGE 通知。 */
    private <T> T inTextOp(Supplier<T> block) {
        ++mInNativeTextOp;
        boolean prevDirty = mNativeTextDirty;
        mNativeTextDirty = false;
        try {
            return block.get();
        } finally {
            final boolean dirtyNow = mNativeTextDirty;
            mNativeTextDirty = prevDirty || dirtyNow;
            --mInNativeTextOp;
            if (mInNativeTextOp == 0 && dirtyNow) {
                notifyEditEvent(EN_CHANGE);
            }
        }
    }
    /** 进入 C++ 内部文本修改保护区域（无返回值版本）。 */
    private void inTextOp(Runnable block) {
        ++mInNativeTextOp;
        boolean prevDirty = mNativeTextDirty;
        mNativeTextDirty = false;
        try {
            block.run();
        } finally {
            final boolean dirtyNow = mNativeTextDirty;
            mNativeTextDirty = prevDirty || dirtyNow;
            --mInNativeTextOp;
            if (mInNativeTextOp == 0 && dirtyNow) {
                notifyEditEvent(EN_CHANGE);
            }
        }
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        if (nativeId != 0) {
            SouiPlatformBridge.getInstance().destroyNative(nativeId);
            nativeId = 0;
        }
    }

    @Override
    public long nativeGetHwnd() {
        return nativeId;
    }

    @Override public void nativeDestroy()                              { mDelegate.nativeDestroy(); }
    @Override public void nativeInvalidate(int l, int t, int r, int b) { mDelegate.nativeInvalidate(l,t,r,b); }
    @Override public boolean nativeShow(int cmdShow)                   { return mDelegate.nativeShow(cmdShow); }
    @Override public boolean nativeMove(int x, int y, int w, int h)    { return mDelegate.nativeMove(x,y,w,h); }
    @Override public boolean nativeSetSize(int cx, int cy)             { return mDelegate.nativeSetSize(cx,cy); }
    @Override public boolean nativeSetPosition(int x, int y)           { return mDelegate.nativeSetPosition(x,y); }
    @Override public boolean nativeIsVisible()                         { return mDelegate.nativeIsVisible(); }
    @Override public boolean nativeEnable(boolean enabled)             { return mDelegate.nativeEnable(enabled); }
    @Override public boolean nativeIsEnabled()                         { return mDelegate.nativeIsEnabled(); }
    @Override public long nativeGetWindow(long hwnd,int code)          { return mDelegate.nativeGetWindow(hwnd,code); }
    @Override
    public View asView() {
        return this;
    }

    // =====================================================================
    //  Win32 消息实现：nativeSendMessage / nativeSendMessageStr / nativeSendMessageStrOut
    //
    //  消息号硬编码值与 swinx/include/winuser.h 保持一致；错配则只影响新消息路由，
    //  不会 crash（default 返回 0 / null）。
    // =====================================================================
    private static final int WM_GETTEXT          = 0x000D;
    private static final int WM_GETTEXTLENGTH    = 0x000E;
    private static final int WM_SETTEXT          = 0x000C;
    private static final int EM_GETSEL           = 0x00B0;
    private static final int EM_SETSEL           = 0x00B1;
    private static final int EM_GETLINECOUNT     = 0x00BA;
    private static final int EM_REPLACESEL       = 0x00C2;
    private static final int EM_GETMODIFY        = 0x00B8;
    private static final int EM_SETMODIFY        = 0x00B9;
    private static final int EM_UNDO             = 0x00C7;
    private static final int EM_CANUNDO          = 0x00CE;
    private static final int EM_EMPTYUNDOBUFFER  = 0x00CD;
    private static final int EM_LIMITTEXT        = 0x00C5;
    private static final int EM_SETREADONLY      = 0x00CF;
    private static final int EM_SETMARGINS       = 0x00D3;
    private static final int EM_GETMARGINS       = 0x00D4;
    private static final int EM_SETPASSWORDCHAR  = 0x00CC;
    private static final int EM_GETPASSWORDCHAR  = 0x00D2;
    private static final int EM_GETLINE          = 0x00C4;
    private static final int EM_LINEINDEX        = 0x00BB;
    private static final int EM_LINELENGTH       = 0x00C1;
    private static final int EM_LINEFROMCHAR     = 0x00C9;
    private static final int EM_SCROLLCARET      = 0x00B7;
    private static final int EM_GETLIMITTEXT     = 0x00D5;

    // ---- StringSlot 编码常量（2026-07 SSOT；与 C++ AndroidPlatformAPI.h 严格对齐）----
    /** slot id 范围约定 ∈ [1..65535]，C++ 侧循环复用；Java 解码始终用 16-bit 掩码。 */
    private static final long SLOT_WORD_MASK = 0xFFFFL;
    /** EM_GETLINE：lParam = (maxChars << 16) | slotId（maxChars ∈ 高 48-bit；slotId ∈ 低 16-bit）。 */
    private static final int  EM_GETLINE_SLOT_SHIFT = 16;

    // EN_* 通知码（反射给父窗口的 WM_COMMAND HIWORD(wParam)）
    private static final int EN_SETFOCUS   = 0x0100;
    private static final int EN_KILLFOCUS  = 0x0200;
    private static final int EN_CHANGE     = 0x0300;
    private static final int EN_UPDATE     = 0x0400;
    private static final int EN_MAXTEXT    = 0x0501;
    @SuppressWarnings("unused")
    private static final int EN_HSCROLL    = 0x0601;
    @SuppressWarnings("unused")
    private static final int EN_VSCROLL    = 0x0602;
    /** 注：Win32 下 EN_SELCHANGE 通过 WM_NOTIFY 发送；这里通过 WM_COMMAND 反射以
     *  复用现有 C++ 父窗口 WM_COMMAND 分派；HIWORD(wParam) == EN_SELCHANGE 即可识别。 */
    private static final int EN_SELCHANGE  = 0x0701;

    @Override
    public long nativeSendMessage(final int msg, final long wp, final long lp) {
        return UiThreadUtils.runOnUiSync(() -> handleMessageImpl(msg, wp, lp));
    }

    /** nativeSendMessage 的实际消息实现主体：保证已在 Android 主线程中执行。 */
    private long handleMessageImpl(int msg, long wp, long lp) {
        switch (msg) {
            case WM_GETTEXTLENGTH:
                return length();
            case EM_GETSEL: {
                final int s = Math.max(0, getSelectionStart());
                final int e = Math.max(0, getSelectionEnd());
                // 打包为 jlong 返回：低 32 bit = start, 高 32 bit = end
                // C++ sendMessage EM_GETSEL 特判再解包回 *(DWORD*)wParam=start, *(DWORD*)lParam=end
                return ((long)s & 0xFFFFFFFFL) | (((long)e & 0xFFFFFFFFL) << 32);
            }
            case EM_SETSEL: {
                final int s = (int)wp;
                final int e = (int)lp;
                final int len = length();
                final int start = Math.max(0, Math.min(s, len));
                final int end   = Math.max(0, Math.min(e, len));
                if (start <= end) setSelection(start, end);
                else              setSelection(end, start);
                return 1;
            }
            case EM_GETLINECOUNT:
                return getLineCount();
            case EM_GETMODIFY:
                return mModified ? 1 : 0;
            case EM_SETMODIFY:
                mModified = (wp != 0);
                return 0;
            // =========================================
            // EM_CANUNDO / EM_UNDO / EM_EMPTYUNDOBUFFER
            //   【BUG 修复】旧代码 super.isUndoAvailable() → NoSuchMethodError 崩
            //   新实现：反射缓存 canUndo/undo/forgetUndoRedoHistory，全路径 catch Throwable
            // =========================================
            case EM_CANUNDO:
                return canUndoReflect() ? 1 : 0;
            case EM_UNDO:
                if (canUndoReflect()) {
                    inTextOp(this::undoReflect);
                    return 1;
                }
                return 0;
            case EM_EMPTYUNDOBUFFER:
                inTextOp(this::forgetUndoRedoHistory);
                mModified = false;
                return 1;
            case EM_LIMITTEXT: {
                mMaxChars = (int)wp;
                if (mMaxChars > 0 && length() > mMaxChars) {
                    inTextOp(() -> getText().delete(mMaxChars, length()));
                }
                return 0;
            }
            case EM_GETLIMITTEXT:
                return mMaxChars;
            case EM_SETREADONLY:
                setFocusableInTouchMode(wp == 0);
                setCursorVisible(wp == 0);
                setEnabled(wp == 0 || hasFocusable());
                // ES_READONLY：只允许通过代码修改，用户不可编辑
                setKeyListener(wp == 0 ? getKeyListener() : null);
                return 0;
            case EM_SETPASSWORDCHAR:
                if (wp == 0) {
                    setTransformationMethod(android.text.method.SingleLineTransformationMethod.getInstance());
                } else {
                    setTransformationMethod(android.text.method.PasswordTransformationMethod.getInstance());
                }
                return 0;
            case EM_GETPASSWORDCHAR:
                final Object tm = getTransformationMethod();
                if (tm instanceof android.text.method.PasswordTransformationMethod) return '*';
                return 0;
            case EM_SETMARGINS: {
                final int flags = (int)wp;
                final int lo = (short)(lp & 0xFFFF);
                final int hi = (short)((lp >> 16) & 0xFFFF);
                int pl = getPaddingLeft(), pr = getPaddingRight();
                if ((flags & 1/*EC_LEFTMARGIN*/) != 0)  pl = lo;
                if ((flags & 2/*EC_RIGHTMARGIN*/) != 0) pr = hi;
                setPadding(pl, getPaddingTop(), pr, getPaddingBottom());
                return 0;
            }
            case EM_GETMARGINS: {
                int lo = getPaddingLeft() & 0xFFFF;
                int hi = getPaddingRight() & 0xFFFF;
                return (lo & 0xFFFFL) | ((hi & 0xFFFFL) << 16);
            }
            case EM_LINEINDEX: {
                // wp=-1 表示当前行；其它是 0 基行号
                int line = (int)wp;
                if (line == -1) line = getLayout() == null ? 0 : getLayout().getLineForOffset(getSelectionStart());
                if (getLayout() == null) return line == 0 ? 0 : -1;
                final int lineCount = getLayout().getLineCount();
                if (line < 0) line = 0;
                if (line >= lineCount) return -1;
                return getLayout().getLineStart(line);
            }
            case EM_LINELENGTH: {
                // wp = 字符索引（=EM_LINELENGTH 参数是"指定字符索引所在行的字节长度"）
                int idx = (int)wp;
                if (idx < 0) idx = getSelectionStart();
                if (getLayout() == null) return length();
                final int line = getLayout().getLineForOffset(Math.min(idx, length()));
                return getLayout().getLineEnd(line) - getLayout().getLineStart(line);
            }
            case EM_LINEFROMCHAR: {
                int idx = (int)lp;
                if (idx == -1) idx = getSelectionStart();
                if (getLayout() == null) return 0;
                return getLayout().getLineForOffset(Math.min(idx, length()));
            }
            case EM_SCROLLCARET:
                if (hasFocus()) {
                    android.text.Layout ly = getLayout();
                    if (ly != null) {
                        final int off = Math.max(0, Math.min(getSelectionStart(), length()));
                        try {
                            int lineY = ly.getLineTop(ly.getLineForOffset(off));
                            if (lineY < getScrollY() || lineY > getScrollY() + getHeight() - getPaddingTop() - getPaddingBottom()) {
                                scrollTo(getScrollX(), lineY - 2);
                            }
                        } catch (Throwable ignored) { /* no-op */ }
                    }
                }
                return 1;
            // =================================================================
            //  字符串入参消息：lParam = 正整数 slotId（Android_Edit WndProc 已 AutoStringSlot 编码）。
            //  不再有 bit63 / isSlot 分支，直接解码 slotId。
            // =================================================================
            case WM_SETTEXT: {
                // wParam: unused；lParam: (LPARAM)inSlotId
                final int slotId = (int) lp;
                final CharSequence text = SouiPlatformBridge.nativeReadCxxStringSlot(slotId);
                inTextOp(() -> setText(text == null ? "" : text));
                mModified = true;
                return 1;
            }
            case EM_REPLACESEL: {
                // wParam: BOOL fCanUndo（保留语义）；lParam: (LPARAM)inSlotId
                final int slotId = (int) lp;
                final String insert = SouiPlatformBridge.nativeReadCxxStringSlot(slotId);
                final CharSequence cur = getText() == null ? "" : getText();
                final int ss = Math.max(0, getSelectionStart());
                final int ee = Math.max(0, getSelectionEnd());
                final int a = Math.min(ss, ee), b = Math.max(ss, ee);
                final CharSequence next = new StringBuilder()
                        .append(cur, 0, a)
                        .append(insert == null ? "" : insert)
                        .append(cur, b, cur.length());
                final int newSel = a + (insert == null ? 0 : insert.length());
                inTextOp(() -> {
                    setText(next);
                    if (newSel <= length()) setSelection(newSel, newSel);
                });
                mModified = true;
                return 0;
            }
            // =================================================================
            //  字符串出参消息：lParam 已编码 outSlot（WM_GETTEXT 直接 slot；
            //  EM_GETLINE 按 "maxChars<<16 | WORD(slotId)" 打包）；
            //  处理完直接 nativeWriteCxxStringSlot，AutoStringSlot 析构自动释放。
            // =================================================================
            case WM_GETTEXT: {
                // wParam: cchTextMax（buffer 大小，含 '\0' 终止位）
                // lParam: (LPARAM)outSlotId（正整数；∈[1..65535]）
                final int maxChars = (int) wp;
                final int slotId = (int) lp;
                final CharSequence s = getText();
                String result = (s == null) ? "" : s.toString();
                if (maxChars > 0) {
                    final int end = Math.min(result.length(), Math.max(0, maxChars - 1));
                    result = result.substring(0, Math.max(0, end));
                }
                SouiPlatformBridge.nativeWriteCxxStringSlot(slotId, result);
                return result.length();
            }
            case EM_GETLINE: {
                // 对齐 Win32：
                //   wParam = lineNo（行号，0 基）
                //   lParam = (maxChars << 16) | (slotId & 0xFFFF)
                //            (高 48-bit = buf 容量；低 16-bit = outSlotId ∈[1..65535])
                final int lineNo = (int) wp;
                final long l = lp;
                final int slotId   = (int) (l & SLOT_WORD_MASK);
                final int maxChars = (int) (l >>> EM_GETLINE_SLOT_SHIFT);
                final android.text.Layout layout = getLayout();
                String lineStr = "";
                if (layout != null) {
                    final int cnt = layout.getLineCount();
                    final int ln = Math.max(0, Math.min(lineNo, Math.max(0, cnt - 1)));
                    final int so = layout.getLineStart(ln);
                    final int eo = layout.getLineVisibleEnd(ln);
                    final CharSequence txt = getText() == null ? "" : getText();
                    final int end = (maxChars <= 0) ? eo : Math.min(eo, so + maxChars);
                    if (so < txt.length()) {
                        lineStr = txt.subSequence(so, Math.min(end, txt.length())).toString();
                    }
                }
                SouiPlatformBridge.nativeWriteCxxStringSlot(slotId, lineStr);
                return lineStr.length();
            }
            default:
                return 0;
        }
    }

    // -----------------------------------------------------------------
    //  注：WM_SETTEXT / EM_REPLACESEL / WM_GETTEXT / EM_GETLINE 等带字符串的消息
    //  已统一走 nativeSendMessage(int, long, long)，字符串通过 C++ slot 交换：
    //    • 入参字符串：C++ 侧 WriteString(slotId, s)，本侧 case 分支里调
    //                 SouiPlatformBridge.nativeReadCxxStringSlot(slotId) 读取；
    //    • 出参字符串：C++ 侧先分配 returnSlot，本侧调用
    //                 SouiPlatformBridge.nativeWriteCxxStringSlot(returnSlot, result) 写回。
    //  详见 AndroidPlatformAPI.h 顶部 SSOT 参数编码约定。
    // -----------------------------------------------------------------
}
