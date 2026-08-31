package com.soui.android;

import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;

/**
 * INativeWindow 接口的公共实现委托类。将窗口状态操作逻辑集中在一处，
 * 让不同基类的 Surface View（SouiSurface、NativeEditView 等）共享同一套行为。
 */
public final class NativeWindowDelegate {

    /** 真正承载 INativeWindow 接口的 Surface View（HWND 真身，绘像素用）。 */
    private final View mSurface;

    /** 计算实际执行操作的目标 View：parent 是 SouiAbsLayout 则操作容器，否则操作 Surface 自身。 */
    private View resolveTarget() {
        final ViewParent p = mSurface.getParent();
        if (p instanceof SouiAbsLayout) {
            // 正常成对出现：操作容器（=SouiWindow）
            return (View) p;
        }
        // Fallback：Surface 自身
        Log.e("nativewnd","error, not valid target");
        return mSurface;
    }

    /** 与 resolveTarget 类似，但允许 orphan（message-only 窗口），此时返回 null。 */
    private View resolveTargetAllowOrphan() {
        final ViewParent p = mSurface.getParent();
        if (p == null) {
            // 无父视图：典型 message-only 占位。返回 null 让 nativeDestroy/nativeShow 等
            // 方法可以直接跳过（"不可见 0x0 占位"本身就不需要视觉树操作）。
            return null;
        }
        if (p instanceof SouiAbsLayout) {
            return (View) p;
        }
        // 其他非标准父容器：把 Surface 自身当作操作目标（如 nativeSetSize/Show）
        return mSurface;
    }

    public NativeWindowDelegate(View surface) {
        if (surface == null) throw new NullPointerException("Surface View must not be null");
        mSurface = surface;
    }

    /** 销毁窗口：从父容器移除整个容器，连带移除所有子窗口。 */
    public void nativeDestroy() {
        final View t = resolveTargetAllowOrphan();
        if (t == null) return; // message-only / 未挂载：无需移除视图，直接返回。
        final ViewParent p = t.getParent();
        if (p instanceof SouiAbsLayout) {
            ((SouiAbsLayout) p).removeView(t);
            if(p instanceof SouiScreen){
                SouiScreen screen = (SouiScreen) p;
                if(screen.getChildCount() == 0){

                }
            }
        } else if (p != null) {
            // 非标准父容器（message-only 不挂父，这里一般不会走到），防御性移除
            if (p instanceof ViewGroup) {
                ((ViewGroup) p).removeView(t);
            }
        }
        // 其它情况（p == null）：message-only 占位 / 已被外部移除，无事可做。
    }

    /** 请求 Surface 重绘。 */
    public void nativeInvalidate(int left, int top, int right, int bottom) {
        mSurface.invalidate(left, top, right, bottom);
    }

    /** 显示/隐藏窗口。 */
    public boolean nativeShow(int cmdShow) {
        final View t = resolveTarget();
        t.setVisibility(cmdShow == 0 ? View.GONE : View.VISIBLE);
        return true;
    }

    /** 移动并调整窗口大小。 */
    public boolean nativeMove(int x, int y, int width, int height) {
        final View t = resolveTarget();
        final ViewParent p = t.getParent();
        if (p instanceof SouiAbsLayout) {
            SouiAbsLayout layout = (SouiAbsLayout) p;
            layout.updateChildFrame(t, x, y, width, height);
            return true;
        } else {
            Log.e("nativedegegate","invalid parent");
            return false;
        }
    }

    /** 设置窗口大小。 */
    public boolean nativeSetSize(int cx, int cy) {
        final View t = resolveTarget();
        final ViewParent p = t.getParent();
        if (p instanceof SouiAbsLayout) {
            ((SouiAbsLayout) p).setChildSize(t, cx, cy);
            return true;
        }else{
            Log.e("nativedegegate","invalid parent");
            return false;
        }
    }

    /** 设置窗口位置。 */
    public boolean nativeSetPosition(int x, int y) {
        final View t = resolveTarget();
        final ViewParent p = t.getParent();
        if (p instanceof SouiAbsLayout) {
            ((SouiAbsLayout) p).setChildPosition(t, x, y);
            return true;
        } else if (p != null) {
            t.setTranslationX(x);
            t.setTranslationY(y);
            return true;
        }
        return false;
    }

    /** 窗口是否可见。 */
    public boolean nativeIsVisible() {
        return resolveTarget().getVisibility() == View.VISIBLE;
    }

    /** 设置窗口 enabled 状态。 */
    public boolean nativeEnable(boolean enabled) {
        final View t = resolveTarget();
        t.setEnabled(enabled);
        return true;
    }

    /** 窗口是否 enabled。 */
    public boolean nativeIsEnabled() {
        return resolveTarget().isEnabled();
    }

    /** 获取窗口信息（对应 Win32 GetWindow）。 */
    public long nativeGetWindow(long hWnd, int code) {
		return 0;
/* hjx, crash
        final int GW_HWNDFIRST = 0;
        final int GW_HWNDLAST  = 1;
        final int GW_HWNDNEXT  = 2;
        final int GW_HWNDPREV  = 3;
        final int GW_OWNER     = 4;
        final int GW_CHILD     = 5;
        final int GW_CHILDLAST = 6;
        final View self = resolveTarget();
        if (self == null) return 0;
        final ViewParent vp = self.getParent();
        final ViewGroup parent = (vp instanceof ViewGroup) ? (ViewGroup) vp : null;
        switch (code) {
            case GW_CHILD: {
                if (self instanceof ViewGroup) {
                    ViewGroup vg = (ViewGroup) self;
                    if (vg.getChildCount() > 0) return hwndOfView(vg.getChildAt(0));
                }
        return 0;
    }
            case GW_CHILDLAST: {
                if (self instanceof ViewGroup) {
                    ViewGroup vg = (ViewGroup) self;
                    final int n = vg.getChildCount();
                    if (n > 0) return hwndOfView(vg.getChildAt(n - 1));
                }
                return 0;
            }
            case GW_OWNER:
                return (parent != null) ? hwndOfView((View) parent) : 0;
            default: {
                if (parent == null) return 0;
                final int count = parent.getChildCount();
                int myIdx = -1;
                for (int i = 0; i < count; i++) {
                    if (parent.getChildAt(i) == self) { myIdx = i; break; }
                }
                if (myIdx < 0) return 0;
                switch (code) {
                    case GW_HWNDFIRST: return count > 0 ? hwndOfView(parent.getChildAt(0)) : 0;
                    case GW_HWNDLAST:  return count > 0 ? hwndOfView(parent.getChildAt(count - 1)) : 0;
                    case GW_HWNDNEXT:  return (myIdx + 1 < count) ? hwndOfView(parent.getChildAt(myIdx + 1)) : 0;
                    case GW_HWNDPREV:  return (myIdx > 0) ? hwndOfView(parent.getChildAt(myIdx - 1)) : 0;
                    default: return 0;
                }
            }
        }
*/
    }

    /**
     * View → HWND 句柄转换。若 View 实现了 INativeWindow 接口则直接调用 nativeGetHwnd()。
     * SOUI 窗口视图（SouiBaseSurface / NativeEditView 等）一律是 INativeWindow 实现，
     * 因此该调用为 O(1) 直接访问；其他非 SOUI 视图返回 0 作为兜底。
     */
    private long hwndOfView(View v) {
        if (v instanceof INativeWindow) return ((INativeWindow) v).nativeGetHwnd();
        return 0;
    }
}
