-keep public class com.soui.android.** { *; }
-keep public class com.soui.android.SouiPlatformBridge { *; }
-keep public class com.soui.android.SouiNativeHandle { *; }
-keep public class com.soui.android.SouiBaseSurface { *; }
-keep public class com.soui.android.NativeEditView { *; }
-keepclasseswithmembernames class * {
    native <methods>;
}
