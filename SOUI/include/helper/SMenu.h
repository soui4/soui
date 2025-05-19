#ifndef __SMENU__H__
#define __SMENU__H__

#include <windows.h>
#include <sobject/Sobject.hpp>
#include <core/SNativeWnd.h>
#include <res.mgr/SSkinPool.h>
#include <helper/obj-ref-impl.hpp>
#include <interface/smenu-i.h>

SNSBEGIN

#define CX_ICON 16 // 支持的图标的宽度
#define CY_ICON 16 // 支持的图标的高度

/**
 * @class SMenuAttr
 * @brief 菜单属性类
 */
class SMenuAttr : public TObjRefImpl<SObject> {
    friend class SMenu;
    friend class SMenuODWnd;
    DEF_SOBJECT(TObjRefImpl<SObject>, L"menuattr")

  public:
    /**
     * @brief 构造函数
     */
    SMenuAttr();

    /**
     * @brief 析构函数
     */
    ~SMenuAttr();

    /**
     * @brief 设置缩放比例
     * @param scale 缩放比例
     */
    void SetScale(int scale);

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"itemSkin", m_pItemSkin, FALSE)           // 菜单项皮肤，包含2种状态：正常状态+选中状态
        ATTR_SKIN(L"iconSkin", m_pIconSkin, FALSE)           // 菜单图标
        ATTR_SKIN(L"sepSkin", m_pSepSkin, FALSE)             // 分割栏皮肤
        ATTR_SKIN(L"checkSkin", m_pCheckSkin, FALSE)         // 选中状态,包含两种状态:勾选+圈选
        ATTR_LAYOUTSIZE(L"itemHeight", m_nItemHei, FALSE)    // 菜单项高度
        ATTR_LAYOUTSIZE(L"iconMargin", m_nIconMargin, FALSE) // 图标边缘空间
        ATTR_LAYOUTSIZE(L"textMargin", m_nTextMargin, FALSE) // 文本边缘空间
        ATTR_LAYOUTSIZE(L"maxWidth", m_nMaxWidth, FALSE)     // 菜单项最大宽度
        ATTR_LAYOUTSIZE2(L"iconSize", m_szIcon, FALSE)       // 图标尺寸
        ATTR_FONT(L"font", m_dpiFont, FALSE)                 // 字体
        ATTR_COLOR(L"colorText", m_crTxtNormal, FALSE)       // 正常文本颜色
        ATTR_COLOR(L"colorTextSel", m_crTxtSel, FALSE)       // 选中文本颜色
        ATTR_COLOR(L"colorTextGray", m_crTxtGray, FALSE)     // 灰文本颜色
        ATTR_STRINGW(L"trCtx", m_strTrCtx, FALSE)            // 翻译上下文
    SOUI_ATTRS_END()

  protected:
    /**
     * @brief 获取文本边缘空间
     * @return 文本边缘空间
     */
    int GetTextMargin();

    /**
     * @brief 获取图标边缘空间
     * @return 图标边缘空间
     */
    int GetIconMargin();

    /**
     * @brief 获取图标尺寸
     * @return 图标尺寸
     */
    CSize GetIconSize();

    /**
     * @brief 获取菜单项高度
     * @return 菜单项高度
     */
    int GetItemHeight();

    /**
     * @brief 获取菜单项最大宽度
     * @return 菜单项最大宽度
     */
    int GetMaxWidth();

    /**
     * @brief 获取缩放比例
     * @return 缩放比例
     */
    int GetScale() const
    {
        return m_scale;
    }

    /**
     * @brief 获取字体指针
     * @return 字体指针
     */
    SAutoRefPtr<IFontS> GetFontPtr();

  protected:
    /**
     * @brief 初始化完成后的回调函数
     * @param xmlNode XML节点
     */
    virtual void WINAPI OnInitFinished(SXmlNode xmlNode);

    SAutoRefPtr<ISkinObj> m_pItemSkin;  // 菜单项皮肤，包含2种状态：正常状态+选中状态
    SAutoRefPtr<ISkinObj> m_pIconSkin;  // 菜单图标
    SAutoRefPtr<ISkinObj> m_pSepSkin;   // 分割栏皮肤
    SAutoRefPtr<ISkinObj> m_pCheckSkin; // 选中状态,包含两种状态:勾选+圈选
    SLayoutSize m_nItemHei;             // 菜单项高度
    SLayoutSize m_nIconMargin;          // 图标边缘空间
    SLayoutSize m_nTextMargin;          // 文本边缘空间
    COLORREF m_crTxtNormal;             // 正常文本颜色
    COLORREF m_crTxtSel;                // 选中文本颜色
    COLORREF m_crTxtGray;               // 灰文本颜色
    SLayoutSize m_szIcon[2];            // 图标尺寸
    SDpiAwareFont m_dpiFont;            // 字体
    SStringW m_strTrCtx;                // 翻译上下文
    SLayoutSize m_nMaxWidth;            // 菜单项最大宽度
    int m_scale;                        // 缩放比例
};

/**
 * @struct SMenuItemData
 * @brief 菜单项数据结构
 */
struct SMenuItemData
{
    /**
     * @brief 构造函数
     */
    SMenuItemData();

    /**
     * @brief 析构函数
     */
    ~SMenuItemData();

    int iIcon;            // 图标索引
    SStringT strText;     // 文本
    UINT vHotKey;         // 快捷键
    ULONG_PTR dwUserData; // 用户数据
    HICON hIcon;          // 图标句柄
};

/**
 * @class SOwnerDraw
 * @brief 所有者绘制模板类
 * @tparam T 派生类类型
 */
template <class T>
class SOwnerDraw {
  public:
    // Message map and handlers
    BEGIN_MSG_MAP_EX(SOwnerDraw<T>)
        MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
        MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
        MESSAGE_HANDLER(WM_COMPAREITEM, OnCompareItem)
        MESSAGE_HANDLER(WM_DELETEITEM, OnDeleteItem)
        MESSAGE_HANDLER(WM_MENUCHAR, OnMenuChar)
        ALT_MSG_MAP(1)
        MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
        MESSAGE_HANDLER(OCM_MEASUREITEM, OnMeasureItem)
        MESSAGE_HANDLER(OCM_COMPAREITEM, OnCompareItem)
        MESSAGE_HANDLER(OCM_DELETEITEM, OnDeleteItem)
    END_MSG_MAP()

    /**
     * @brief 处理WM_DRAWITEM消息
     * @param uMsg 消息ID
     * @param wParam 附加参数1
     * @param lParam 附加参数2
     * @param bHandled 消息是否被处理
     * @return 处理结果
     */
    LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);
        pT->DrawItem((LPDRAWITEMSTRUCT)lParam);
        bHandled = pT->IsMsgHandled();
        return (LRESULT)TRUE;
    }

    /**
     * @brief 处理WM_MEASUREITEM消息
     * @param uMsg 消息ID
     * @param wParam 附加参数1
     * @param lParam 附加参数2
     * @param bHandled 消息是否被处理
     * @return 处理结果
     */
    LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);
        pT->MeasureItem((LPMEASUREITEMSTRUCT)lParam);
        bHandled = pT->IsMsgHandled();
        return (LRESULT)TRUE;
    }

    /**
     * @brief 处理WM_COMPAREITEM消息
     * @param uMsg 消息ID
     * @param wParam 附加参数1
     * @param lParam 附加参数2
     * @param bHandled 消息是否被处理
     * @return 处理结果
     */
    LRESULT OnCompareItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);
        bHandled = pT->IsMsgHandled();
        return (LRESULT)pT->CompareItem((LPCOMPAREITEMSTRUCT)lParam);
    }

    /**
     * @brief 处理WM_DELETEITEM消息
     * @param uMsg 消息ID
     * @param wParam 附加参数1
     * @param lParam 附加参数2
     * @param bHandled 消息是否被处理
     * @return 处理结果
     */
    LRESULT OnDeleteItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);
        pT->DeleteItem((LPDELETEITEMSTRUCT)lParam);
        bHandled = pT->IsMsgHandled();
        return (LRESULT)TRUE;
    }

    /**
     * @brief 处理WM_MENUCHAR消息
     * @param uMsg 消息ID
     * @param wParam 附加参数1
     * @param lParam 附加参数2
     * @param bHandled 消息是否被处理
     * @return 处理结果
     */
    LRESULT OnMenuChar(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
    {
        T *pT = static_cast<T *>(this);
        pT->SetMsgHandled(TRUE);

        UINT nChar = LOWORD(wParam);
        UINT nFlags = HIWORD(wParam);
        HMENU hMenu = (HMENU)lParam;
        LRESULT lRes = pT->OnMenuChar(nChar, nFlags, hMenu);

        bHandled = pT->IsMsgHandled();
        return lRes;
    }

    // Overrideables
    /**
     * @brief 绘制菜单项
     * @param lpDrawItemStruct 绘制项结构
     */
    void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
    {
        // must be implemented
        SASSERT(FALSE);
    }

    /**
     * @brief 测量菜单项
     * @param lpMeasureItemStruct 测量项结构
     */
    void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
    {
        if (lpMeasureItemStruct->CtlType != ODT_MENU)
        {
            // return default height for a system font
            T *pT = static_cast<T *>(this);
            HWND hWnd = pT->GetDlgItem(lpMeasureItemStruct->CtlID);
            HDC dc = ::GetDC(hWnd);
            TEXTMETRIC tm = { 0 };
            GetTextMetrics(dc, &tm);

            lpMeasureItemStruct->itemHeight = tm.tmHeight;
            ::ReleaseDC(hWnd, dc);
        }
        else
            lpMeasureItemStruct->itemHeight = ::GetSystemMetrics(SM_CYMENU);
    }

    /**
     * @brief 比较菜单项
     * @param lpCompareItemStruct 比较项结构
     * @return 比较结果
     */
    int CompareItem(LPCOMPAREITEMSTRUCT /*lpCompareItemStruct*/)
    {
        // all items are equal
        return 0;
    }

    /**
     * @brief 删除菜单项
     * @param lpDeleteItemStruct 删除项结构
     */
    void DeleteItem(LPDELETEITEMSTRUCT /*lpDeleteItemStruct*/)
    {
        // default - nothing
    }
};

/**
 * @class SMenuODWnd
 * @brief 所有者绘制菜单窗口类
 */
class SMenuODWnd
    : public SNativeWnd
    , public SOwnerDraw<SMenuODWnd> {
    friend class SOwnerDraw<SMenuODWnd>;

  public:
    /**
     * @brief 构造函数
     * @param hMenuOwner 菜单所有者窗口句柄
     * @param pMenuAttr 菜单属性对象指针
     */
    SMenuODWnd(HWND hMenuOwner, SMenuAttr *pMenuAttr);

    /**
     * @brief 析构函数
     */
    ~SMenuODWnd();

  protected:
    /**
     * @brief 初始化菜单
     * @param menu 菜单句柄
     */
    void OnInitMenu(HMENU menu);

    /**
     * @brief 初始化弹出菜单
     * @param menuPopup 弹出菜单句柄
     * @param nIndex 索引
     * @param bSysMenu 是否为系统菜单
     */
    void OnInitMenuPopup(HMENU menuPopup, UINT nIndex, BOOL bSysMenu);

    /**
     * @brief 绘制菜单项
     * @param lpDrawItemStruct 绘制项结构
     */
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    /**
     * @brief 测量菜单项
     * @param lpMeasureItemStruct 测量项结构
     */
    void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

    /**
     * @brief 处理菜单选择消息
     * @param nItemID 项ID
     * @param nFlags 标志
     * @param menu 菜单句柄
     */
    void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU menu);

    /**
     * @brief 处理菜单字符消息
     * @param nChar 字符
     * @param nFlags 标志
     * @param hMenu 菜单句柄
     * @return 处理结果
     */
    LRESULT OnMenuChar(UINT nChar, UINT nFlags, HMENU hMenu);

    BEGIN_MSG_MAP_EX(SMenuODWnd)
        MSG_WM_INITMENU(OnInitMenu)
        MSG_WM_INITMENUPOPUP(OnInitMenuPopup)
        MSG_WM_MENUSELECT(OnMenuSelect)
        CHAIN_MSG_MAP(SOwnerDraw<SMenuODWnd>)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

    HWND m_hMenuOwner;             // 菜单所有者窗口句柄
    SAutoRefPtr<SMenuAttr> m_attr; // 菜单属性对象指针
};

/**
 * @class SMenu
 * @brief 菜单类
 */
class SOUI_EXP SMenu : public TObjRefImpl<IMenu> {
  public:
    /**
     * @brief 复制构造函数
     * @param src 源菜单对象
     */
    SMenu(const SMenu &src);

    /**
     * @brief 构造函数
     * @param hMenu 菜单句柄（默认为0）
     */
    SMenu(HMENU hMenu = 0);

    /**
     * @brief 析构函数
     */
    ~SMenu(void);

  public:
    HMENU m_hMenu; // 菜单句柄

  public: // IMenu
    /**
     * @brief 附加菜单句柄
     * @param hMenu 菜单句柄
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, Attach)(THIS_ HMENU hMenu) OVERRIDE;

    /**
     * @brief 分离菜单句柄
     * @return 分离的菜单句柄
     */
    STDMETHOD_(HMENU, Detach)(THIS) OVERRIDE;

    /**
     * @brief 获取菜单句柄
     * @return 菜单句柄
     */
    STDMETHOD_(HMENU, GetHMenu)(THIS) SCONST OVERRIDE;

    /**
     * @brief 加载菜单资源
     * @param resId 资源ID
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, LoadMenu)(THIS_ LPCTSTR resId) OVERRIDE;

    /**
     * @brief 加载菜单资源（UTF-8）
     * @param resId 资源ID
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, LoadMenuU8)(THIS_ LPCSTR resId) OVERRIDE;

    /**
     * @brief 加载菜单资源（XML）
     * @param xmlMenu XML节点
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, LoadMenu2)(THIS_ IXmlNode *xmlMenu) OVERRIDE;

    /**
     * @brief 设置菜单图标皮肤
     * @param icons 图标皮肤对象指针
     */
    STDMETHOD_(void, SetIconSkin)(THIS_ ISkinObj *icons) OVERRIDE;

    /**
     * @brief 插入菜单项
     * @param uPosition 位置
     * @param uFlags 标志
     * @param nIDNewItem 新项ID
     * @param strText 文本
     * @param iIcon 图标索引（默认为-1）
     * @param hIcon 图标句柄（默认为0）
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, UINT_PTR nIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) OVERRIDE;

    /**
     * @brief 追加菜单项
     * @param uFlags 标志
     * @param uIDNewItem 新项ID
     * @param lpNewItem 文本
     * @param iIcon 图标索引（默认为-1）
     * @param hIcon 图标句柄（默认为0）
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR lpNewItem, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) OVERRIDE;
    /**
     * @brief 检查菜单项
     * @param uIdCheckItem 要检查的菜单项ID
     * @param uCheck 检查标志（例如，MF_CHECKED, MF_UNCHECKED）
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) OVERRIDE;

    /**
     * @brief 检查菜单项（单选按钮）
     * @param idFirst 第一个菜单项ID
     * @param idLast 最后一个菜单项ID
     * @param idCheck 要检查的菜单项ID
     * @param uFlags 检查标志（例如，MF_BYCOMMAND, MF_BYPOSITION）
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) OVERRIDE;

    /**
     * @brief 删除菜单项
     * @param uPosition 菜单项位置
     * @param uFlags 删除标志（例如，MF_BYCOMMAND, MF_BYPOSITION）
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) OVERRIDE;

    /**
     * @brief 跟踪弹出菜单
     * @param uFlags 跟踪标志（例如，TPM_LEFTALIGN, TPM_RIGHTALIGN）
     * @param x 菜单左上角的X坐标
     * @param y 菜单左上角的Y坐标
     * @param hWnd 父窗口句柄
     * @param prcRect 可选的矩形区域
     * @param nScale 缩放比例（默认为100）
     * @return 菜单项ID
     */
    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, LPCRECT prcRect DEF_VAL(NULL), int nScale DEF_VAL(100)) OVERRIDE;

    /**
     * @brief 销毁菜单
     */
    STDMETHOD_(void, DestroyMenu)(THIS) OVERRIDE;

    /**
     * @brief 修改菜单项字符串
     * @param uPosition 菜单项位置
     * @param uFlags 标志（例如，MF_BYCOMMAND, MF_BYPOSITION）
     * @param lpItemString 新的菜单项字符串
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, ModifyMenuString)(THIS_ UINT uPosition, UINT uFlags, LPCTSTR lpItemString) OVERRIDE;

    /**
     * @brief 设置菜单项用户数据
     * @param uPosition 菜单项位置
     * @param uFlags 标志（例如，MF_BYCOMMAND, MF_BYPOSITION）
     * @param ulUserData 用户数据
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, SetMenuUserData)(THIS_ UINT uPosition, UINT uFlags, ULONG_PTR ulUserData) OVERRIDE;

    /**
     * @brief 获取菜单项用户数据
     * @param uPosition 菜单项位置
     * @param uFlags 标志（例如，MF_BYCOMMAND, MF_BYPOSITION）
     * @return 用户数据
     */
    STDMETHOD_(ULONG_PTR, GetMenuUserData)(THIS_ UINT uPosition, UINT uFlags) OVERRIDE;

    /**
     * @brief 获取上下文帮助ID
     * @return 上下文帮助ID
     */
    STDMETHOD_(DWORD, GetContextHelpId)(THIS) SCONST OVERRIDE;

    /**
     * @brief 设置上下文帮助ID
     * @param dwId 上下文帮助ID
     */
    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) OVERRIDE;

    /**
     * @brief 获取子菜单
     * @param nPos 子菜单位置
     * @return 子菜单句柄
     */
    STDMETHOD_(HMENU, GetSubMenu)(THIS_ int nPos) OVERRIDE;

    /**
     * @brief 获取菜单项字符串
     * @param uPosition 菜单项位置
     * @param uFlags 标志（例如，MF_BYCOMMAND, MF_BYPOSITION）
     * @param lpItemString 接收菜单项字符串的对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, GetMenuString)(THIS_ UINT uPosition, UINT uFlags, IStringT *lpItemString) OVERRIDE;

  protected:
    /**
     * @brief 更新缩放比例
     * @param nScale 缩放比例
     */
    void UpdateScale(int nScale);

    /**
     * @brief 构建菜单
     * @param menuPopup 弹出菜单句柄
     * @param xmlNode XML节点
     */
    void BuildMenu(HMENU menuPopup, SXmlNode xmlNode);

    /**
     * @brief 初始化菜单项数据
     * @param itemInfo 菜单项数据指针
     * @param strText 菜单项文本
     */
    void InitMenuItemData(SMenuItemData *itemInfo, const SStringW &strText);

    /**
     * @brief 释放菜单项数据
     * @param hMemu 菜单句柄
     */
    void FreeMenuItemData(HMENU hMemu);

    /**
     * @brief 获取菜单属性对象指针
     * @param hMenu 菜单句柄
     * @return 菜单属性对象指针
     */
    SMenuAttr *GetMenuAttr(HMENU hMenu) const;

    /**
     * @brief 设置菜单属性对象指针
     * @param hMenu 菜单句柄
     * @param pMenuAttr 菜单属性对象指针
     */
    void SetMenuAttr(HMENU hMenu, SMenuAttr *pMenuAttr) const;

    bool m_bAttached;              // 是否已附加菜单句柄
    SAutoRefPtr<ISkinObj> m_icons; // 图标皮肤对象指针
};

SNSEND
#endif // __SMENU__H__