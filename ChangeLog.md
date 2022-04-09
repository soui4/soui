# ChangeLog file for SOUI

## 4.0 使用类COM技术导致SOUI核心对象，方便C语言调用。
### 3.0升级4.0注意事项
1.  原SOUI_CLASS_NAME替换为DEF_SOBJECT, 且第一个参数是它的基类
2.  事件定义使用DEF_EVENT/DEF_EVENT_EXT这两个宏来定义，使用使用方法参见系统事件
3.  内置的ResProvider使用SouiFactory来创建，不再提供全局函数创建。
4.  resbuilder重写，定R,UIRES的方法有点变化。参考demo
5.  SEventArgs替换为IEvtArgs
6. 原代码中直接使用pugi_xml，4.0中统一使用SOUI包装后的SXmlDoc, SXmlNode, SXmlAttribute。对应的接口为IXmlDoc, IXmlNode, IXmlAttribute。
7. 原代码中使用的IPen, IBrush, IBitmap, IRegion, IPath等IRenderObj对象统一增加S后缀，变为IPenS, IBrushS, IBitmapS, IRegionS, IPathS等，主要是C接口中不支持命名空间，导致可能和系统接口重名。（最新修改在C++版本中增加了兼容定义）
8. 为了兼容C接口，所有接口的方法都没有了默认参数。比如IRenderTarget.SelectObject，3.0第二个参数默认为NULL, 新版本没有默认参数，必须写全参数。(最新修改已经可以兼容默认参数）
9. XxxView的适配器接口有调整，所有方法都统一使用WINAPI调用，原SWindow*的参数替换为SItemPanel*, 原pugi::xml_node替换为SXmlNode。
10. SHostWnd不直接继承自SWindow，而是只继承SNativeWnd, 需要使用GetRoot()来获取顶层SWindow*
11. SXmlDoc对象和pugi::xml_document一个区别在于SXmlDoc不继承SXmlNode, 要获取root节点，可以使用SXmlDoc.root()方法。

## 3.0 增加矩阵变换及仿Android动画
## 2.0 完善SOUI模板结构框架