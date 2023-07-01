//!添加引用
/*!
 */
STDMETHOD_(long, AddRef)(THIS) PURE;

//!释放引用
/*!
 */
STDMETHOD_(long, Release)(THIS) PURE;

//!释放对象
/*!
 */
STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
//////////////////////////////////////////////////////////////////////////

/**
 * @brief 创建一个Native窗口
 * @param lpWindowName 窗口名
 * @param dwStyle 窗口style
 * @param dwExStyle 窗口exStyle
 * @param x 窗口显示位置X
 * @param y 窗口显示位置Y
 * @param nWidth 窗口显示宽度
 * @param nHeight 窗口显示高度
 * @param hWndParent 父窗口
 * @param nID 窗口ID
 * @param lpParam 附加参数
 * @return 窗口HWND
 */
STDMETHOD_(HWND, CreateNative)
(THIS_ LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID, LPVOID lpParam) PURE;

/**
 * @brief 获取窗口HWND
 * @return 窗口HWND
 */
STDMETHOD_(HWND, GetHwnd)(THIS) PURE;

/**
 * @brief 用SNativeWnd对象子类化一个HWND
 * @param hWnd 目标HWND
 * @return TRUE-成功
 */
STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) PURE;

/**
 * @brief 反子类化
 * @param bForce 为真是，如果当前窗口的wndproc不是SNativeWnd对象的窗口过程也执行反子类化
 * @return 执行反子类化的窗口HWND
 */
STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce /*= FALSE*/) PURE;

/**
 * @brief 获取当前的窗口消息对象
 * @return 窗口消息对象指针
 */
STDMETHOD_(const MSG *, GetCurrentMessage)(CTHIS) SCONST PURE;

/**
 * @brief 获取窗口ID
 */
STDMETHOD_(int, GetDlgCtrlID)(CTHIS) SCONST PURE;

/**
 * @brief 获取窗口Style
 */
STDMETHOD_(DWORD, GetStyle)(CTHIS) SCONST PURE;

/**
 * @brief 获取窗口exStyle
 */
STDMETHOD_(DWORD, GetExStyle)(CTHIS) SCONST PURE;

/**
 * @brief 获取窗口关联数据
 * @param nIndex 数据索引
 * @return 数据值
 */
STDMETHOD_(LONG_PTR, GetWindowLongPtr)(CTHIS_ int nIndex) SCONST PURE;

/**
 * @brief 设置窗口关联数据
 * @param nIndex 数据索引
 * @param dwNewLong 数据值
 * @return 原数据值
 */
STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) PURE;

/**
 * @brief 获取父窗口
 * @return 父窗口HWND
 */
STDMETHOD_(HWND, GetParent)(THIS) PURE;

/**
 * @brief 设置父窗口
 * @param hWndNewParent 新父窗口HWND
 * @return 原父窗口HWND
 */
STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) PURE;

/**
 * @brief 获取窗口的enable状态
 * @return TRUE-enable
 */
STDMETHOD_(BOOL, IsWindowEnabled)(CTHIS) SCONST PURE;

/**
 * @brief 修改窗口style
 * @param dwRemove 待移除标志位
 * @param dwAdd 待增加标志位
 * @param nFlags Window positioning flags
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, ModifyStyle)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/) PURE;

/**
 * @brief 修改窗口exStyle
 * @param dwRemove 待移除标志位
 * @param dwAdd 待增加标志位
 * @param nFlags Window positioning flags
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, ModifyStyleEx)(THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags /*=0*/) PURE;

/**
 * @brief 设置窗口的Pos
 * @param hWndInsertAfter zorder位置
 * @param x
 * @param y
 * @param cx
 * @param cy
 * @param nFlags 标志位
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, SetWindowPos)
(THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) PURE;

/**
 * @brief 窗口居中
 * @param hWndCenter 相对居中窗口
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter /*= NULL*/) PURE;

/**
 * @brief 销毁窗口
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, DestroyWindow)(THIS) PURE;

/**
 * @brief 检查是否为有效HWND
 * @return
 */
STDMETHOD_(BOOL, IsWindow)(CTHIS) SCONST PURE;

/**
 * @brief 刷新窗口
 * @param bErase 清除窗口内容
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase /*= TRUE*/) PURE;

/**
 * @brief 刷新窗口指定区域
 * @param lpRect 待刷新区域
 * @param bErase 清除窗口内容
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase /* = TRUE*/) PURE;

/**
 * @brief 获取窗口位置
 * @param[out] lpRect 窗口位置
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, GetWindowRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

/**
 * @brief 获取窗口客户区位置
 * @param[out] lpRect 客户区位置
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, GetClientRect)(CTHIS_ LPRECT lpRect) SCONST PURE;

/**
 * @brief 将相对窗口的坐标转换为屏幕坐标
 * @param[in][out] lpPoint 待转换坐标
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, ClientToScreen)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

/**
 * @brief 将相对窗口的矩形坐标转换为屏幕坐标
 * @param[in][out] lpRect 待转换坐标
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, ClientToScreen2)(CTHIS_ LPRECT lpRect) SCONST PURE;

/**
 * @brief 将屏幕坐标转换为窗口坐标
 * @param[in][out] lpPoint 待转换坐标
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, ScreenToClient)(CTHIS_ LPPOINT lpPoint) SCONST PURE;

/**
 * @brief 将屏幕矩形坐标转换为窗口坐标
 * @param[in][out] lpRect 待转换坐标
 * @return TRUE-SUCCEED
 */
STDMETHOD_(BOOL, ScreenToClient2)(CTHIS_ LPRECT lpRect) SCONST PURE;

/**
 * @brief 将一组窗口坐标转换为相对于另一个窗口的坐标
 * @param hWndTo 目标窗口
 * @param[in][out] lpPoint 坐标数组
 * @param nCount 数组长度
 * @return TRUE-SUCCEED
 */
STDMETHOD_(int, MapWindowPoints)(CTHIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST PURE;

/**
 * @brief 将一个窗口矩形映射为相对于另一个窗口的矩形
 * @param hWndTo 目标窗口
 * @param[in][out] lpRect 待转换矩形
 * @return TRUE-SUCCEED
 */
STDMETHOD_(int, MapWindowRect)(CTHIS_ HWND hWndTo, LPRECT lpRect) SCONST PURE;

/**
 * @brief 启动一人定时器
 * @param nIDEvent 定时器ID
 * @param nElapse 延时(ms)
 * @param lpfnTimer 定时器处理函数，为null时产生WM_TIMER消息
 * @return 定时器ID
 */
STDMETHOD_(UINT_PTR, SetTimer)
(THIS_ UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) /*= NULL*/) PURE;

/**
 * @brief 删除一个定时器
 * @param nIDEvent 定时器ID
 * @return
 */
STDMETHOD_(BOOL, KillTimer)(THIS_ UINT_PTR nIDEvent) PURE;

/**
 * @brief 获取窗口的客户区DC
 * @return 客户区DC
 */
STDMETHOD_(HDC, GetDC)(THIS) PURE;

/**
 * @brief 获取窗口DC
 * @return 窗口DC
 */
STDMETHOD_(HDC, GetWindowDC)(THIS) PURE;

/**
 * @brief 释放DC
 * @param hDC DC
 * @return
 */
STDMETHOD_(int, ReleaseDC)(THIS_ HDC hDC) PURE;

/**
 * @brief 创建一个光标
 * @param hBitmap 光标位图
 * @return
 */
STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP hBitmap) PURE;

/**
 * @brief 隐藏光标
 * @return
 */
STDMETHOD_(BOOL, HideCaret)(THIS) PURE;

/**
 * @brief 显示光标
 * @return
 */
STDMETHOD_(BOOL, ShowCaret)(THIS) PURE;

/**
 * @brief 获取当前有鼠标捕获的HWND
 * @return 鼠标捕获的HWND
 */
STDMETHOD_(HWND, GetCapture)(THIS) PURE;

/**
 * @brief 将当前窗口设置为捕获鼠标输入
 * @return 前一个HWND
 */
STDMETHOD_(HWND, SetCapture)(THIS) PURE;

/**
 * @brief 退出鼠标捕获
 * @return
 */
STDMETHOD_(BOOL, ReleaseCapture)(THIS) PURE;

/**
 * @brief 让窗口成为接收键盘输入的焦点窗口
 * @return 前一个焦点窗口
 */
STDMETHOD_(HWND, SetFocus)(THIS) PURE;

/**
 * @brief 同步执行一个消息到窗口处理过程
 * @param message msg id
 * @param wParam
 * @param lParam
 * @return 消息处理返回值
 */
STDMETHOD_(LRESULT, SendMessage)
(THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

/**
 * @brief 异步执行一个消息到窗口处理过程
 * @param message msg id
 * @param wParam
 * @param lParam
 * @return 消息处理返回值
 */
STDMETHOD_(BOOL, PostMessage)
(THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

STDMETHOD_(BOOL, SendNotifyMessage)
(THIS_ UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) PURE;

/**
 * @brief 设置窗口标题
 * @param lpszString 标题文本
 * @return
 */
STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) PURE;

/**
 * @brief 获取窗口标题文本
 * @param lpszStringBuf 缓冲区
 * @param nMaxCount 缓冲区长度
 * @return lpszStringBuf为null时返回数据长度
 */
STDMETHOD_(int, GetWindowText)(CTHIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST PURE;

/**
 * @brief 查询是否为最小化状态
 * @return TRUE-最小化
 */
STDMETHOD_(BOOL, IsIconic)(CTHIS) SCONST PURE;

/**
 * @brief 查询是否为最大化状态
 * @return TRUE-最大化
 */
STDMETHOD_(BOOL, IsZoomed)(CTHIS) SCONST PURE;

/**
 * @brief 查询窗口是否可见
 * @return TRUE-可见
 */
STDMETHOD_(BOOL, IsWindowVisible)(CTHIS) SCONST PURE;

/**
 * @brief 移动窗口位置
 * @param x
 * @param y
 * @param nWidth
 * @param nHeight
 * @param bRepaint 刷新窗口标志
 * @return
 */
STDMETHOD_(BOOL, MoveWindow)
(THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint /*= TRUE*/) PURE;

/**
 * @brief 移动窗口位置
 * @param lpRect 目标位置
 * @param bRepaint 刷新标志
 * @return
 */
STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint /*= TRUE*/) PURE;

/**
 * @brief 显示窗口
 * @param nCmdShow 显示标志
 * @return
 */
STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) PURE;

/**
 * @brief 设置窗口异形区域
 * @param hRgn 异形区域
 * @param bRedraw 刷新标志
 * @return
 */
STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw /*=TRUE*/) PURE;

/**
 * @brief 设置窗口的分层属性
 * @param crKey 透明色
 * @param bAlpha 透明度
 * @param dwFlags 标志位
 * @return
 */
STDMETHOD_(BOOL, SetLayeredWindowAttributes)
(THIS_ COLORREF crKey, BYTE bAlpha, DWORD dwFlags) PURE;

/**
 * @brief 更新分层窗口
 * @param hdcDst
 * @param pptDst
 * @param psize
 * @param hdcSrc
 * @param pptSrc
 * @param crKey
 * @param pblend
 * @param dwFlags
 * @return
 */
STDMETHOD_(BOOL, UpdateLayeredWindow)
(THIS_ HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags) PURE;

/**
 * @brief 设置窗口消息处理函数
 * @param fun 消息处理函数
 * @param ctx 消息处理函数Context
 * @return
 */
STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) PURE;

/**
 * @brief 获取窗口消息处理对象
 * @return 消息处理对象
 */
STDMETHOD_(MsgHandlerInfo *, GetMsgHandler)(THIS) PURE;
