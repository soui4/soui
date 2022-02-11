#ChangeLog file for SOUI

##4.0 使用类COM技术导致SOUI核心对象，方便C语言调用。
###3.0升级4.0注意事项
####1 原SOUI_CLASS_NAME替换为DEF_SOBJECT, 且第一个参数是它的基类
####2 事件定义使用DEF_EVENT/DEF_EVENT_EXT这两个宏来定义，使用使用方法参见系统事件
####3 内置的ResProvider使用SouiFactory来创建，不再提供全局函数创建。
####4 resbuilder重写，定R,UIRES的方法有点变化。参考demo

##3.0 增加矩阵变换及仿Android动画
##2.0 完善SOUI模板结构框架