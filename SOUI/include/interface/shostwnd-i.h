#ifndef __SHOSTWND_I__H__
#define __SHOSTWND_I__H__
#include <interface/SNativeWnd-i.h>
#include <interface/SNcPainter-i.h>
#include <interface/smsgloop-i.h>

SNSBEGIN

typedef struct IWindow IWindow;
typedef struct IApplication IApplication;
typedef struct IHostPresenter IHostPresenter;
typedef struct IHostWnd IHostWnd;

typedef struct _EventHandlerInfo
{
    FunCallback fun;
    void *ctx;
} EventHandlerInfo;

#undef INTERFACE
#define INTERFACE IHostWnd
DECLARE_INTERFACE_(IHostWnd, INativeWnd)
{
#include "SNativeWndApi.h"
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 创建窗口
     * @param hWndParent 父窗口
     * @param dwStyle style
     * @param dwExStyle exStyle
     * @param x
     * @param y
     * @param nWidth
     * @param nHeight
     * @return HWND
     */
    STDMETHOD_(HWND, CreateEx)
    (THIS_ HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight) PURE;

    /**
     * @brief 创建窗口
     * @param hWndParent
     * @param x
     * @param y
     * @param nWidth
     * @param nHeight
     * @return HWND
     */
    STDMETHOD_(HWND, Create)
    (THIS_ HWND hWndParent, int x DEF_VAL(0), int y DEF_VAL(0), int nWidth DEF_VAL(0), int nHeight DEF_VAL(0)) PURE;

    /**
     * @brief 设置窗口布局资源ID
     * @param pszLayoutId 布局资源ID
     * @return
     */
    STDMETHOD_(void, SetLayoutId)(THIS_ LPCTSTR pszLayoutId) PURE;

    /**
     * @brief 从XML初始化窗口
     * @param pNode XML数据
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode * pNode) PURE;

    /**
     * @brief 获取Dui Root
     * @return Dui Root
     */
    STDMETHOD_(IWindow *, GetIRoot)(THIS) PURE;

    /**
     * @brief 查询窗口的半透明标志
     * @return TRUE-窗口半透明
     */
    STDMETHOD_(BOOL, IsTranslucent)(CTHIS) SCONST PURE;

    /**
     * @brief 获取窗口的上屏对象
     * @return IHostPresenter* - 上屏对象
     */
    STDMETHOD_(IHostPresenter *, GetPresenter)(THIS) PURE;

    /**
     * @brief 设置窗口上屏对象
     * @param pPresenter 上屏对象
     * @return
     */
    STDMETHOD_(void, SetPresenter)(THIS_ IHostPresenter * pPresenter) PURE;

    /**
     * @brief 获取窗口所属的msgloop对象
     * @param
     * @return
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) PURE;

    /**
     * @brief 根据ID查找子窗口
     * @param nId int--子窗口ID
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) PURE;

    /**
     * @brief 根据Name查找子窗口
     * @param pszName LPCWSTR--子窗口Name
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) PURE;

    /**
     * @brief 根据Name查找子窗口
     * @param pszName LPCSTR--子窗口Name(utf8)
     * @return
     */
    STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) PURE;

    /**
     * @brief 获取非客户区的绘制对象
     * @return INcPainter *
     */
    STDMETHOD_(INcPainter *, GetNcPainter)(THIS) PURE;

    /**
     * @brief 设置事件处理对象
     * @param fun 事件处理对象
     * @param ctx 事件处理对象Context
     * @return
     */
    STDMETHOD_(void, SetEventHandler)(THIS_ FunCallback fun, void *ctx) PURE;

    /**
     * @brief 获取事件处理对象
     * @return EventHandlerInfo*
     */
    STDMETHOD_(EventHandlerInfo *, GetEventHandler)(THIS) PURE;

    /**
     * @brief 动画显示/隐藏窗口
     * @param dwTime
     * @param dwFlags
     * @return
     * @remark 参考API AnimateWindow
     */
    STDMETHOD_(BOOL, AnimateHostWindow)(THIS_ DWORD dwTime, DWORD dwFlags) PURE;

    /**
     * @brief 让窗口支持DragDrop
     * @return
     */
    STDMETHOD_(void, EnableDragDrop)(THIS) PURE;

    /**
     * @brief 显示或者隐藏HostWnd
     * @param uShowCmd 和ShowWindow参数相同
     * @param bWaitAniDone 当窗口有配置进出动画时，等待动画完成标志
     * @return
     */
    STDMETHOD_(void, ShowHostWnd)(THIS_ int uShowCmd, BOOL bWaitAniDone) PURE;

    /**
     * @brief 是否启用布局中定义的私有UIDef对象
     * @param BOOL bEnable TRUE-启动，FALSE-关闭
     * @return void
     * @remark 当程序中需要重新从私有UIDef中获取数据时启动，启用完后关闭
     */
    STDMETHOD_(void, EnablePrivateUiDef)(THIS_ BOOL bEnable) PURE;

    /**
     * @brief 设置窗口的dpi倍数
     * @param int nScale dpi倍数,以100为基数
     * @param LPCRECT pDestRect 缩放后的窗口坐标
     * @return void
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale, LPCRECT pDestRect) PURE;
};

#undef INTERFACE
#define INTERFACE IHostDialog
DECLARE_INTERFACE_(IHostDialog, IHostWnd)
{
#include "SNativeWndApi.h"

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 创建窗口
     * @param hWndParent 父窗口
     * @param dwStyle style
     * @param dwExStyle exStyle
     * @param x
     * @param y
     * @param nWidth
     * @param nHeight
     * @return HWND
     */
    STDMETHOD_(HWND, CreateEx)
    (THIS_ HWND hWndParent, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight) PURE;

    /**
     * @brief 创建窗口
     * @param hWndParent
     * @param x
     * @param y
     * @param nWidth
     * @param nHeight
     * @return HWND
     */
    STDMETHOD_(HWND, Create)
    (THIS_ HWND hWndParent, int x DEF_VAL(0), int y DEF_VAL(0), int nWidth DEF_VAL(0), int nHeight DEF_VAL(0)) PURE;

    /**
     * @brief 设置窗口布局资源ID
     * @param pszLayoutId 布局资源ID
     * @return
     */
    STDMETHOD_(void, SetLayoutId)(THIS_ LPCTSTR pszLayoutId) PURE;

    /**
     * @brief 从XML初始化窗口
     * @param pNode XML数据
     * @return TRUE-成功
     */
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode * pNode) PURE;

    /**
     * @brief 获取Dui Root
     * @return Dui Root
     */
    STDMETHOD_(IWindow *, GetIRoot)(THIS) PURE;

    /**
     * @brief 查询窗口的半透明标志
     * @return TRUE-窗口半透明
     */
    STDMETHOD_(BOOL, IsTranslucent)(CTHIS) SCONST PURE;

    /**
     * @brief 获取窗口的上屏对象
     * @return IHostPresenter* - 上屏对象
     */
    STDMETHOD_(IHostPresenter *, GetPresenter)(THIS) PURE;

    /**
     * @brief 设置窗口上屏对象
     * @param pPresenter 上屏对象
     * @return
     */
    STDMETHOD_(void, SetPresenter)(THIS_ IHostPresenter * pPresenter) PURE;

    /**
     * @brief 获取窗口所属的msgloop对象
     * @param
     * @return
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS) PURE;

    /**
     * @brief 根据ID查找子窗口
     * @param nId int--子窗口ID
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) PURE;

    /**
     * @brief 根据Name查找子窗口
     * @param pszName LPCWSTR--子窗口Name
     * @return IWindow*--匹配窗口
     * @remark 采用广度优先算法搜索匹配子窗口
     */
    STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) PURE;

    /**
     * @brief 根据Name查找子窗口
     * @param pszName LPCSTR--子窗口Name(utf8)
     * @return
     */
    STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) PURE;

    /**
     * @brief 获取非客户区的绘制对象
     * @return INcPainter *
     */
    STDMETHOD_(INcPainter *, GetNcPainter)(THIS) PURE;

    /**
     * @brief 设置事件处理对象
     * @param fun 事件处理对象
     * @param ctx 事件处理对象Context
     * @return
     */
    STDMETHOD_(void, SetEventHandler)(THIS_ FunCallback fun, void *ctx) PURE;

    /**
     * @brief 获取事件处理对象
     * @return EventHandlerInfo*
     */
    STDMETHOD_(EventHandlerInfo *, GetEventHandler)(THIS) PURE;

    /**
     * @brief 动画显示/隐藏窗口
     * @param dwTime
     * @param dwFlags
     * @return
     * @remark 参考API AnimateWindow
     */
    STDMETHOD_(BOOL, AnimateHostWindow)(THIS_ DWORD dwTime, DWORD dwFlags) PURE;

    /**
     * @brief 让窗口支持DragDrop
     * @return
     */
    STDMETHOD_(void, EnableDragDrop)(THIS) PURE;

    /**
     * @brief 显示或者隐藏HostWnd
     * @param uShowCmd 和ShowWindow参数相同
     * @param bWaitAniDone 当窗口有配置进出动画时，等待动画完成标志
     * @return
     */
    STDMETHOD_(void, ShowHostWnd)(THIS_ int uShowCmd, BOOL bWaitAniDone) PURE;

    /**
     * @brief 是否启用布局中定义的私有UIDef对象
     * @param BOOL bEnable TRUE-启动，FALSE-关闭
     * @return void
     * @remark 当程序中需要重新从私有UIDef中获取数据时启动，启用完后关闭
     */
    STDMETHOD_(void, EnablePrivateUiDef)(THIS_ BOOL bEnable) PURE;

    /**
     * @brief 设置窗口的dpi倍数
     * @param int nScale dpi倍数,以100为基数
     * @param LPCRECT pDestRect 缩放后的窗口坐标
     * @return void
     */
    STDMETHOD_(void, SetScale)(THIS_ int nScale, LPCRECT pDestRect) PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 启动一个模式窗口
     * @param hParent 窗口的Owner
     * @return
     */
    STDMETHOD_(INT_PTR, DoModal)(THIS_ HWND hParent /*=NULL*/) PURE;

    /**
     * @brief 退出当前模式窗口
     * @param nResult DoModal的返回值
     * @return
     */
    STDMETHOD_(void, EndDialog)(THIS_ INT_PTR nResult) PURE;
};

SNSEND

#endif // __SHOSTWND_I__H__