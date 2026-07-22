package com.soui.android;

public class SouiNativeHandle {
    public static native long NativeCreate(INativeWindow wnd);
    public static native void NativeDestroy(long nativeId);
}
