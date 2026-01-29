#ifndef __SMENUEX__H__
#define __SMENUEX__H__

#include <core/SHostWnd.h>
#include <interface/smenuex-i.h>

SNSBEGIN

class SMenuExRoot;
class SMenuEx;

/**
 * @class SMenuExItem
 * @brief 扩展菜单项类
 */
class SOUI_EXP SMenuExItem : public SWindow {
    DEF_SOBJECT(SWindow, L"menuItem")
    friend class SMenuEx;

  public:
    /**
     * @brief 构造函数
     * @param pOwnerMenu 所属菜单对象指针
     * @param pItemSkin 菜单项皮肤对象指针
     */
    SMenuExItem(SMenuEx *pOwnerMenu, ISkinObj *pItemSkin);

    /**
     * @brief 析构函数
     */
    ~SMenuExItem();

    /**
     * @brief 获取子菜单
     * @return 子菜单对象指针
     */
    SMenuEx *GetSubMenu();

    /**
     * @brief 获取所属菜单
     * @return 所属菜单对象指针
     */
    SMenuEx *GetOwnerMenu();

    /**
     * @brief 隐藏子菜单
     */
    void HideSubMenu();

    /**
     * @brief 显示子菜单
     * @param bCheckFirstItem 是否选中第一个子菜单项
     */
    void ShowSubMenu(BOOL bCheckFirstItem);

    /**
     * @brief 获取热键字符
     * @return 热键字符
     */
    WCHAR GetHotKey() const;

  protected:
    void BeforePaint(IRenderTarget *pRT, SPainter &painter) override;
    
    /**
     * @brief 处理子菜单隐藏事件
     * @param bUncheckItem 是否取消选中子菜单项
     */
    void OnSubMenuHided(BOOL bUncheckItem);

    /**
     * @brief 创建子窗口
     * @param xmlNode XML节点
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL CreateChildren(SXmlNode xmlNode) OVERRIDE;

    /**
     * @brief 根据名称创建子窗口
     * @param pszName 子窗口名称
     * @return 子窗口对象指针
     */
    SWindow *CreateChildByName(LPCWSTR pszName) OVERRIDE;

    /**
     * @brief 获取期望的大小
     * @param psz 大小结构指针
     * @param wid 宽度
     * @param hei 高度
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz, int wid, int hei) OVERRIDE;

    /**
     * @brief 获取文本矩形
     * @param pRect 矩形结构指针
     */
    void GetTextRect(LPRECT pRect) OVERRIDE;

  protected:
    /**
     * @brief 处理擦除背景消息
     * @param pRT 渲染目标对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL OnEraseBkgnd(IRenderTarget *pRT);

    /**
     * @brief 处理绘制消息
     * @param pRT 渲染目标对象指针
     */
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

  protected:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"icon", m_iIcon, TRUE)       // 图标索引
        ATTR_INT(L"check", m_bCheck, TRUE)     // 是否选中
        ATTR_INT(L"radio", m_bRadio, TRUE)     // 是否为单选按钮
        ATTR_CHAR(L"hotKey", m_cHotKey, FALSE) // 热键字符
    SOUI_ATTRS_END()

    SMenuEx *m_pSubMenu;   // 子菜单对象指针
    SMenuEx *m_pOwnerMenu; // 所属菜单对象指针
    int m_iIcon;           // 图标索引
    BOOL m_bCheck;         // 是否选中
    BOOL m_bRadio;         // 是否为单选按钮
    WCHAR m_cHotKey;       // 热键字符
};

/**
 * @class SMenuEx
 * @brief 扩展菜单类
 */
class SOUI_EXP SMenuEx
    : protected SHostWnd
    , public IMenuEx {
    friend class SMenuExItem;
    friend class SMenuExRunData;
    friend class SMenuExRoot;
    typedef SHostWnd __baseCls;

  public:
    /**
     * @brief 构造函数
     */
    SMenuEx(void);

    /**
     * @brief 析构函数
     */
    virtual ~SMenuEx(void);

  public:
    //! 添加引用
    STDMETHOD_(long, AddRef)(THIS) OVERRIDE
    {
        return SHostWnd::AddRef();
    }

    //! 释放引用
    STDMETHOD_(long, Release)(THIS) OVERRIDE
    {
        return SHostWnd::Release();
    }

    //! 释放对象
    STDMETHOD_(void, OnFinalRelease)(THIS) OVERRIDE
    {
        delete this;
    }

    //=================================================================
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
     * @brief 插入菜单项
     * @param uPosition 位置
     * @param uFlags 标志
     * @param nIDNewItem 新项ID
     * @param strText 文本
     * @param iIcon 图标索引（默认为-1）
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, int nIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1)) OVERRIDE;

    /**
     * @brief 追加菜单项
     * @param uFlags 标志
     * @param uIDNewItem 新项ID
     * @param lpNewItem 文本
     * @param iIcon 图标索引（默认为-1）
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, int uIDNewItem, LPCTSTR lpNewItem, int iIcon DEF_VAL(-1)) OVERRIDE;

    /**
     * @brief 检查菜单项（单选按钮）
     * @param idFirst 第一个菜单项ID
     * @param idLast 最后一个菜单项ID
     * @param idCheck 要检查的菜单项ID
     * @param uFlags 标志（例如，MF_BYCOMMAND, MF_BYPOSITION）
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) OVERRIDE;

    /**
     * @brief 检查菜单项
     * @param uIdCheckItem 要检查的菜单项ID
     * @param uCheck 检查标志（例如，MF_CHECKED, MF_UNCHECKED）
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) OVERRIDE;

    /**
     * @brief 启用或禁用菜单项
     * @param uIDEnableItem 项目ID或索引
     * @param uEnable MF_BYCOMMAND/MF_BYPOSITION | MF_ENABLED/MF_GRAYED/MF_DISABLED
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, EnableMenuItem)(THIS_ UINT uIDEnableItem, UINT uEnable) OVERRIDE;

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
     * @param nScale 缩放比例（默认为100）
     * @return 菜单项ID
     */
    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, int nScale DEF_VAL(100)) OVERRIDE;

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
     * @return 子菜单对象指针
     */
    STDMETHOD_(IMenuEx *, GetSubMenu)(THIS_ int nPos) OVERRIDE;

    /**
     * @brief 获取菜单项字符串
     * @param uPosition 菜单项位置
     * @param uFlags 标志（例如，MF_BYCOMMAND, MF_BYPOSITION）
     * @param lpItemString 接收菜单项字符串的对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, GetMenuString)(THIS_ UINT uPosition, UINT uFlags, IStringT *lpItemString) OVERRIDE;

    /**
     * @brief 获取宿主窗口
     * @return 宿主窗口对象指针
     */
    STDMETHOD_(IHostWnd *, GetHostWnd)(THIS) OVERRIDE
    {
        return this;
    }

  public:
    /**
     * @brief 结束菜单
     * @param nCmdId 命令ID（默认为0）
     */
    static void EndMenu(int nCmdId = 0);

    /**
     * @brief 获取事件所有者
     * @return 事件所有者对象指针
     */
    static SMenuEx *GetEvtOwner();

    /**
     * @brief 获取父菜单项
     * @return 父菜单项对象指针
     */
    SMenuExItem *GetParentItem()
    {
        return m_pParent;
    }

    /**
     * @brief 获取子菜单
     * @param nID 菜单项ID
     * @param byCmdId 是否按命令ID查找
     * @return 子菜单对象指针
     */
    SMenuEx *GetSubMenu(int nID, BOOL byCmdId);

    /**
     * @brief 获取菜单项
     * @param nID 菜单项ID
     * @param byCmdId 是否按命令ID查找
     * @return 菜单项对象指针
     */
    SMenuExItem *GetMenuItem(int nID, BOOL byCmdId);

  protected:
    /**
     * @brief 初始化一个空菜单（不应在外部调用）
     * @param ParentRoot 父菜单根对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL IniNullMenu(SMenuExRoot *ParentRoot);

    /**
     * @brief 处理鼠标激活消息
     * @param wndTopLevel 顶级窗口句柄
     * @param nHitTest 击中测试结果
     * @param message 消息ID
     * @return 激活状态
     */
    int OnMouseActivate(HWND wndTopLevel, UINT nHitTest, UINT message);

    /**
     * @brief 处理定时器消息
     * @param timeID 定时器ID
     */
    void OnTimer(UINT_PTR timeID);

    /**
     * @brief 处理按键消息
     * @param nChar 字符
     * @param nRepCnt 重复次数
     * @param nFlags 标志
     */
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    BEGIN_MSG_MAP_EX(SMenuEx)
        MSG_WM_MOUSEACTIVATE(OnMouseActivate)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_KEYDOWN(OnKeyDown)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()

  protected:
    /**
     * @brief 处理事件
     * @param pEvt 事件参数对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    virtual BOOL _HandleEvent(IEvtArgs *pEvt);

    /**
     * @brief 从资源ID加载布局
     * @param xmlDoc XML文档对象引用
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL OnLoadLayoutFromResourceID(SXmlDoc &xmlDoc) override;

    /**
     * @brief 创建根窗口
     * @return 根窗口对象指针
     */
    SRootWindow *CreateRoot() override;

    /**
     * @brief 获取翻译上下文
     * @return 翻译上下文字符串
     */
    STDMETHOD_(LPCWSTR, GetTranslatorContext)(THIS) SCONST OVERRIDE;

    /**
     * @brief 获取缩放比例
     * @return 缩放比例
     */
    STDMETHOD_(int, GetScale)() SCONST OVERRIDE;

    /**
     * @brief 查找菜单项
     * @param uPos 位置
     * @param uFlag 标志
     * @return 菜单项对象指针
     */
    SWindow *FindItem(UINT uPos, UINT uFlag);

    /**
     * @brief 显示菜单
     * @param uFlag 标志
     * @param x X坐标
     * @param y Y坐标
     */
    void ShowMenu(UINT uFlag, int x, int y);

    /**
     * @brief 隐藏菜单
     * @param bUncheckParentItem 是否取消选中父菜单项
     */
    void HideMenu(BOOL bUncheckParentItem);

    /**
     * @brief 隐藏子菜单
     */
    void HideSubMenu();

    /**
     * @brief 运行菜单
     * @param hOwner 所有者窗口句柄
     */
    void RunMenu(HWND hOwner);

    /**
     * @brief 弹出子菜单
     * @param pItem 菜单项对象指针
     * @param bCheckFirstItem 是否选中第一个子菜单项
     */
    void PopupSubMenu(SMenuExItem *pItem, BOOL bCheckFirstItem);

    /**
     * @brief 处理子菜单隐藏事件
     * @param bUncheckItem 是否取消选中子菜单项
     */
    void OnSubMenuHided(BOOL bUncheckItem);

    /**
     * @brief 构造函数（带父菜单项）
     * @param pParent 父菜单项对象指针
     */
    SMenuEx(SMenuExItem *pParent);

    /**
     * @brief 发送初始化弹出菜单事件给所有者
     * @param idx 索引
     */
    void SendInitPopupMenu2Owner(int idx);

    SMenuExItem *m_pParent;    // 父菜单项对象指针
    SMenuExItem *m_pHoverItem; // 悬停菜单项对象指针
    SMenuExItem *m_pCheckItem; // 选中菜单项对象指针

    BOOL m_bMenuInitialized; // 菜单是否已初始化
};

SNSEND

#endif // __SMENUEX__H__