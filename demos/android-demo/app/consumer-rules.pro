# If this library defines custom components that require keeping reflections/native methods,
# keep them here. This demo's consumer rules are identical to proguard-rules.pro by default.
-keep class com.soui.** { *; }
-keepclassmembers class com.soui.** { native <methods>; }
