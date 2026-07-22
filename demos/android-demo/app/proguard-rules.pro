# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# SOUI: 反射/JNI 入口不要被改名或剥离
-keep class com.soui.** { *; }
-keepclassmembers class com.soui.** { native <methods>; }
-keep class com.soui.demo.** { *; }
-keepclassmembers class com.soui.demo.** { native <methods>; }
