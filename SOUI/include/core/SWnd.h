﻿/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       Swnd.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Describe    SOUI基础DUI窗口模块
 */

#pragma once
#include <core/SWindowMgr.h>
#include <interface/SwndContainer-i.h>
#include <interface/slayout-i.h>
#include <interface/saccproxy-i.h>
#include <helper/SwndMsgCracker.h>
#include <layout/SLayoutSize.h>
#include <event/SEventSlot.h>
#include <event/SEvents.h>
#include <event/SEventSet.h>
#include <res.mgr/SUiDef.h>
#include <core/SwndStyle.h>
#include <core/SSkin.h>
#include <animation/SAnimation.h>
#include <interface/SWindow-i.h>
#include <OCIdl.h>

#define SC_WANTARROWS  0x0001     /* Control wants arrow keys         */
#define SC_WANTTAB     0x0002     /* Control wants tab keys           */
#define SC_WANTRETURN  0x0004     /* Control wants return keys        */
#define SC_WANTCHARS   0x0008     /* Want WM_CHAR messages            */
#define SC_WANTALLKEYS 0xFFFF     /* Control wants all keys           */
#define SC_WANTSYSKEY  0x80000000 /* System Key */

#define ICWND_FIRST ((SWindow *)-1) /*子窗口插入在开头*/
#define ICWND_LAST  NULL            /*子窗口插入在末尾*/

#ifdef _DEBUG
#define ASSERT_UI_THREAD() SOUI::SWindow::TestMainThread()
#else
#define ASSERT_UI_THREAD()
#endif

SNSBEGIN

/////////////////////////////////////////////////////////////////////////
enum
{
    NormalShow = 0,
    ParentShow = 1
}; //提供WM_SHOWWINDOW消息识别是父窗口显示还是要显示本窗口
enum
{
    NormalEnable = 0,
    ParentEnable = 1
}; //提供WM_ENABLE消息识别是父窗口可用还是直接操作当前窗口
enum
{
    UM_SETSCALE = (WM_USER + 1000),
    UM_SETLANGUAGE,
    UM_SETCOLORIZE,
    UM_UPDATEFONT,
};
// State Define
enum WndState
{
    WndState_Normal = 0x00000000UL,
    WndState_Hover = 0x00000001UL,
    WndState_PushDown = 0x00000002UL,
    WndState_Check = 0x00000004UL,
    WndState_Invisible = 0x00000008UL,
    WndState_Disable = 0x00000010UL,
};

typedef struct SWNDMSG
{
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
} SWNDMSG, *PSWNDMSG;

class SStateHelper {
  public:
    static void MarkState(DWORD &dwState, WndState state)
    {
        dwState |= state;
    }

    static void ClearState(DWORD &dwState, WndState state)
    {
        dwState &= ~state;
    }

    static bool TestState(DWORD dwState, WndState state)
    {
        return (dwState & state) == state;
    }
};

class SOUI_EXP STimerID {
  public:
    DWORD swnd : 24;      //窗口句柄,如果窗口句柄超过24位范围，则不能使用这种方式设置定时器
    DWORD uTimerID : 7;   //定时器ID，一个窗口最多支持128个定时器。
    DWORD bSwndTimer : 1; //区别通用定时器的标志，标志为1时，表示该定时器为SWND定时器

    STimerID(SWND swnd_, char timeId)
        : swnd(swnd_)
        , uTimerID(timeId)
        , bSwndTimer(1)
    {
        SASSERT(swnd < 0x00FFFFFF && timeId >= 0);
    }
    STimerID(DWORD dwID)
    {
        memcpy(this, &dwID, sizeof(DWORD));
    }
    operator DWORD &() const
    {
        return *(DWORD *)this;
    }
};

class SOUI_EXP SPainter {
  public:
    SPainter()
        : oldTextColor(CR_INVALID)
    {
    }

    SAutoRefPtr<IFontS> oldFont;
    COLORREF oldTextColor;
};

//////////////////////////////////////////////////////////////////////////
// SWindow
//////////////////////////////////////////////////////////////////////////

typedef enum tagGW_CODE
{
    GSW_FIRSTCHILD = 0,
    GSW_LASTCHILD,
    GSW_PREVSIBLING,
    GSW_NEXTSIBLING,
    GSW_PARENT,
    GSW_OWNER,
} GW_CODE;

struct SwndToolTipInfo
{
    SWND swnd;       //拥有tooltip的窗口
    DWORD dwCookie;  // tooltip在窗口内的ID，对应一个窗口不同区域显示不同tip的情况，一般可以不提供
    CRect rcTarget;  // tooltip感应区
    SStringT strTip; // tip字符串
};

enum
{
    HRET_FLAG_STYLE = (1 << 16),
    HRET_FLAG_LAYOUT = (1 << 17),
    HRET_FLAG_LAYOUT_PARAM = (1 << 18),
};

class SWindow;
class SOUI_EXP STrText {
  public:
    STrText(SWindow *pOwner = NULL);
    void SetOwner(SWindow *pOwner);

    void SetText(const SStringT &strText);

    SStringT GetText(BOOL bRawText = FALSE) const;

    void TranslateText();

  protected:
    SWindow *pOwner;

    SStringT strRaw; //原始字符串
    SStringT strTr;  //翻译后的字符串
};

/**
 * @class     SWindow
 * @brief     SOUI窗口基类
 *
 * Describe   SOUI窗口基类,实现窗口的基本接口
 */
class SOUI_EXP SWindow
    : public TObjRefImpl<SObjectImpl<IWindow>>
    , protected IAnimationListener {
    DEF_SOBJECT_EX(TObjRefImpl<SObjectImpl<IWindow>>, L"window", Window)
    friend class SwndLayoutBuilder;
    friend class SWindowRepos;
    friend class SHostWnd;
    friend class SwndContainerImpl;
    friend class FocusSearch;
    friend class SGridLayout;
    friend class SLinearLayout;
    friend class SouiLayout;
	friend class SHostProxy;

    class SAnimationHandler : public ITimelineHandler {
      private:
        SWindow *m_pOwner;
        STransformation m_transform;
        bool m_bFillAfter;
        SWindow *m_pPrevSiblingBackup; // previous sibling of owner.
      public:
        SAnimationHandler(SWindow *pOwner);
        void OnAnimationStart();
        void OnAnimationStop();
        const STransformation &GetTransformation() const;
        bool getFillAfter() const;

      public:
        STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;

      protected:
        BOOL OnOwnerResize(IEvtArgs *e);
    };

  public:
    SWindow();

    virtual ~SWindow();

  public:
    BOOL IsMsgHandled() const;

    void SetMsgHandled(BOOL bHandled);

  public:
	STDMETHOD_(void, OnFinalRelease)(THIS);

    STDMETHOD_(LPCWSTR, GetName)(THIS) SCONST OVERRIDE
    {
        return m_strName;
    }
    STDMETHOD_(void, SetName)(THIS_ LPCWSTR pszName) OVERRIDE
    {
        m_strName = pszName;
    }

    STDMETHOD_(int, GetID)(THIS) SCONST OVERRIDE
    {
        return m_nID;
    }
    STDMETHOD_(void, SetID)(THIS_ int nID) OVERRIDE
    {
        m_nID = nID;
    }

    STDMETHOD_(SWND, GetSwnd)(THIS) SCONST OVERRIDE;

    STDMETHOD_(ILayout *, GetLayout)(THIS) OVERRIDE
    {
        return m_pLayout;
    }

    STDMETHOD_(ILayoutParam *, GetLayoutParam)(THIS) SCONST OVERRIDE
    {
        return m_pLayoutParam;
    }

    STDMETHOD_(BOOL, SetLayoutParam)(THIS_ ILayoutParam *pLayoutParam) OVERRIDE;

    STDMETHOD_(BOOL, IsFloat)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsDisplay)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetWindowText)(THIS_ LPCTSTR lpszText) OVERRIDE;

	STDMETHOD_(void, SetWindowTextA)(THIS_ LPCSTR lpszText) OVERRIDE;

    STDMETHOD_(void, SetToolTipText)(THIS_ LPCTSTR pszText) OVERRIDE;
	STDMETHOD_(void, SetToolTipTextA)(THIS_ LPCSTR pszText) OVERRIDE;

    STDMETHOD_(BOOL, IsChecked)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetCheck)(THIS_ BOOL bCheck) OVERRIDE;

    STDMETHOD_(BOOL, IsDisabled)(THIS_ BOOL bCheckParent DEF_VAL(FALSE)) SCONST OVERRIDE;

    STDMETHOD_(void, EnableWindow)(THIS_ BOOL bEnable, BOOL bUpdate DEF_VAL(FALSE)) OVERRIDE;

    STDMETHOD_(BOOL, IsVisible)(THIS_ BOOL bCheckParent DEF_VAL(FALSE)) SCONST OVERRIDE;

    STDMETHOD_(void, SetVisible)(THIS_ BOOL bVisible, BOOL bUpdate DEF_VAL(FALSE)) OVERRIDE;

    STDMETHOD_(BOOL, IsMsgTransparent)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsMsgTransparentSelf)(THIS) SCONST OVERRIDE;

    STDMETHOD_(ULONG_PTR, GetUserData)(THIS) SCONST OVERRIDE;
    STDMETHOD_(ULONG_PTR, SetUserData)(THIS_ ULONG_PTR uData) OVERRIDE;
    STDMETHOD_(void, GetWindowRect)(THIS_ LPRECT prect) SCONST OVERRIDE;
	STDMETHOD_(BOOL,IsVideoCanvas)(CTHIS) SCONST OVERRIDE;
	STDMETHOD_(void, GetVisibleRect)(CTHIS_ LPRECT prect) SCONST OVERRIDE;
    STDMETHOD_(void, GetClientRect)(THIS_ LPRECT prect) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsContainPoint)(THIS_ POINT pt, BOOL bClientOnly) SCONST OVERRIDE;
    STDMETHOD_(void, DoColorize)(THIS_ COLORREF cr) OVERRIDE;
    STDMETHOD_(COLORREF, GetColorizeColor)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, Destroy)(THIS) OVERRIDE;
    STDMETHOD_(void, BringWindowToTop)(THIS) OVERRIDE;
    STDMETHOD_(UINT, GetChildrenCount)(THIS) SCONST OVERRIDE;
    STDMETHOD_(LRESULT, SSendMessage)
    (THIS_ UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0, BOOL *pbMsgHandled DEF_VAL(NULL) ) OVERRIDE;
    STDMETHOD_(void, SDispatchMessage)
    (THIS_ UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) OVERRIDE;

    STDMETHOD_(void, SetFocus)(THIS) OVERRIDE;
    STDMETHOD_(void, KillFocus)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, IsFocused)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, Invalidate)(THIS) OVERRIDE;
    STDMETHOD_(void, InvalidateRect)(THIS_ LPCRECT lprect) OVERRIDE;

    STDMETHOD_(void, LockUpdate)(THIS) OVERRIDE;
    STDMETHOD_(void, UnlockUpdate)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, IsUpdateLocked)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, Update)(THIS) OVERRIDE;
    STDMETHOD_(void, Move)(THIS_ LPCRECT prect) OVERRIDE;
    STDMETHOD_(void, SetWindowRgn)(THIS_ IRegionS *pRgn, BOOL bRedraw DEF_VAL(TRUE)) OVERRIDE;
    STDMETHOD_(IRegionS *, GetWindowRgn)(THIS) SCONST OVERRIDE;
    STDMETHOD_(void, SetWindowPath)(THIS_ IPathS *pPath, BOOL bRedraw DEF_VAL(TRUE)) OVERRIDE;
    STDMETHOD_(IPathS *, GetWindowPath)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, SetTimer)(THIS_ char id, UINT uElapse) OVERRIDE;
    STDMETHOD_(void, KillTimer)(THIS_ char id) OVERRIDE;

    STDMETHOD_(SWND, GetCapture)(THIS) SCONST OVERRIDE;
    STDMETHOD_(SWND, SetCapture)(THIS) OVERRIDE;
    STDMETHOD_(BOOL, ReleaseCapture)(THIS) OVERRIDE;

    STDMETHOD_(void, SetAnimation)(THIS_ IAnimation *animation) OVERRIDE;

    STDMETHOD_(IAnimation *, GetAnimation)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, StartAnimation)(THIS_ IAnimation *animation) OVERRIDE;

    STDMETHOD_(void, ClearAnimation)(THIS) OVERRIDE;

    STDMETHOD_(void, SetAlpha)(THIS_ BYTE byAlpha) OVERRIDE;

    STDMETHOD_(BYTE, GetAlpha)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetMatrix)(THIS_ const IMatrix *mtx) OVERRIDE;

    STDMETHOD_(void, GetMatrix)(THIS_ IMatrix *mtx) SCONST OVERRIDE;

    STDMETHOD_(int, GetScale)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsSiblingsAutoGroupped)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, RequestRelayout)(THIS) OVERRIDE;
    STDMETHOD_(void, UpdateLayout)(THIS) OVERRIDE;

    STDMETHOD_(UINT, OnGetDlgCode)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsFocusable)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsClipClient)(THIS) SCONST OVERRIDE;
    STDMETHOD_(BOOL, IsLayoutDirty)(THIS) SCONST OVERRIDE;

    STDMETHOD_(IWindow *, GetNextLayoutIChild)(THIS_ const IWindow *pCurChild) SCONST OVERRIDE;

    STDMETHOD_(void, UpdateChildrenPosition)(THIS) OVERRIDE;

    STDMETHOD_(IWindow *, GetIWindow)(THIS_ int uCode) SCONST OVERRIDE;

    STDMETHOD_(IWindow *, GetIChild)(THIS_ int iChild) SCONST OVERRIDE;

    STDMETHOD_(IWindow *, GetIParent)(THIS) SCONST OVERRIDE;

    STDMETHOD_(IWindow *, GetIRoot)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsDescendant)(THIS_ const IWindow *pWnd) SCONST OVERRIDE;

    STDMETHOD_(BOOL, AdjustIZOrder)(THIS_ IWindow *pInsertAfter) OVERRIDE;

    STDMETHOD_(void, InsertIChild)
    (THIS_ IWindow *pNewChild, IWindow *pInsertAfter = ICWND_LAST) OVERRIDE;

    STDMETHOD_(BOOL, RemoveIChild)(THIS_ IWindow *pChild) OVERRIDE;

    STDMETHOD_(BOOL, DestroyIChild)(THIS_ IWindow *pChild) OVERRIDE;

    STDMETHOD_(void, DestroyAllChildren)(THIS) OVERRIDE;

    STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) OVERRIDE;

    STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) OVERRIDE;

	STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) OVERRIDE;

    STDMETHOD_(ISwndContainer *, GetContainer)(THIS) OVERRIDE;
    STDMETHOD_(void, SetContainer)(THIS_ ISwndContainer *pContainer) OVERRIDE;

    STDMETHOD_(void, GetChildrenLayoutRect)(THIS_ RECT* prc) SCONST OVERRIDE;

    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE *psz,int nParentWid, int nParentHei) OVERRIDE;

    STDMETHOD_(void, Move2)(THIS_ int x, int y, int cx DEF_VAL(-1), int cy DEF_VAL(-1)) OVERRIDE;

    STDMETHOD_(int, GetWindowText)(THIS_ TCHAR *pBuf, int nBufLen, BOOL bRawText) OVERRIDE;

	STDMETHOD_(int, GetWindowTextA)(THIS_ IStringA *pStr, BOOL bRawText) OVERRIDE;

    STDMETHOD_(void, SetEventMute)(THIS_ BOOL bMute) OVERRIDE;

    STDMETHOD_(DWORD, GetState)(THIS) SCONST OVERRIDE;
    STDMETHOD_(DWORD, ModifyState)
    (THIS_ DWORD dwStateAdd, DWORD dwStateRemove, BOOL bUpdate DEF_VAL(FALSE)) OVERRIDE;

    STDMETHOD_(void, SetIOwner)(THIS_ IWindow *pOwner) OVERRIDE;
    STDMETHOD_(IWindow *, GetIOwner)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, CreateChildrenFromXml)(THIS_ LPCWSTR pszXml) OVERRIDE;

    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *pNode) OVERRIDE;

    STDMETHOD_(BOOL, GetAttribute)(THIS_ LPCWSTR pszName, IStringW *strValue) SCONST OVERRIDE;

    STDMETHOD_(COLORREF, GetBkgndColor)(THIS) SCONST OVERRIDE;

    STDMETHOD_(IWindow *, GetISelectedSiblingInGroup)(THIS) OVERRIDE;

    STDMETHOD_(IWindow *, GetISelectedChildInGroup)(THIS) OVERRIDE;


	STDMETHOD_(SWND, SwndFromPoint)(THIS_ POINT *pt, BOOL bIncludeMsgTransparent DEF_VAL(FALSE)) SCONST OVERRIDE;

    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    STDMETHOD_(BOOL, FireCommand)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, FireCtxMenu)(THIS_ POINT pt) OVERRIDE;

    STDMETHOD_(BOOL, SubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) OVERRIDE;

    STDMETHOD_(BOOL, UnsubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) OVERRIDE;

    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef **ppRet) OVERRIDE;

	STDMETHOD_(BOOL,AddEvent)(THIS_ DWORD dwEventID, LPCWSTR pszEventHandlerName) OVERRIDE;

	STDMETHOD_(BOOL,RemoveEvent)(THIS_ DWORD dwEventID) OVERRIDE;

	STDMETHOD_(BOOL, SwndProc)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult) OVERRIDE;

	STDMETHOD_(void, SetSwndProc)(THIS_ FunSwndProc swndProc) OVERRIDE;

	STDMETHOD_(HWND, GetHostHwnd)(THIS) OVERRIDE;

	STDMETHOD_(BOOL,RegisterDragDrop)(THIS_ IDropTarget *pDragTarget) OVERRIDE;
	STDMETHOD_(BOOL,UnregisterDragDrop)(THIS) OVERRIDE;

  public: // caret相关方法
    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP pBmp, int nWid, int nHeight) OVERRIDE;
    STDMETHOD_(void, ShowCaret)(THIS_ BOOL bShow) OVERRIDE;
    STDMETHOD_(void, SetCaretPos)(THIS_ int x, int y) OVERRIDE;

  public:
    IAccessible *GetAccessible();
    IAccProxy *GetAccProxy();
    void accNotifyEvent(DWORD dwEvt);

  public:
    /**
     * GetCurMsg
     * @brief    获得当前正在处理的消息
     * @return   PSWNDMSG
     *
     * Describe
     */
    PSWNDMSG GetCurMsg(void) const
    {
        return m_pCurMsg;
    }

    const ISwndContainer *GetContainer() const;

    SWindow *GetWindow(int uCode) const;

    SWindow *GetChild(int iChild) const;

    SWindow *GetParent() const;

    SWindow *GetRoot() const;

    SWindow *GetNextLayoutChild(const SWindow *pCurChild) const;

    /**
     * AdjustZOrder
     * @brief    调整窗口Z序
     * @param    SWindow *pInsertAfter --  插入在这个窗口之后
     * @return   bool,  pInsertAfter与this非同级窗口返回失败
     *
     * Describe  pInsertAfter可以为NULL，或是与this同一级的兄弟窗口
     */
    BOOL AdjustZOrder(SWindow *pInsertAfter);

    /**
     * InsertChild
     * @brief    在窗口树中插入一个子窗口
     * @param    SWindow * pNewChild --  子窗口对象
     * @param    SWindow * pInsertAfter --  插入位置
     * @return   void
     *
     * Describe  一般用于UI初始化的时候创建，插入的窗口不会自动进入布局流程
     */
    void InsertChild(SWindow *pNewChild, SWindow *pInsertAfter = ICWND_LAST);

    /**
     * RemoveChild
     * @brief    从窗口树中移除一个子窗口对象
     * @param    SWindow * pChild --  子窗口对象
     * @return   BOOL
     *
     * Describe  子窗口不会自动释放
     */
    BOOL RemoveChild(SWindow *pChild);

    /**
     * DestroyChild
     * @brief    销毁一个子窗口
     * @param    SWindow * pChild --  子窗口对象
     * @return   BOOL
     *
     * Describe  先调用RemoveChild，再调用pChild->Release来释放子窗口对象
     */
    BOOL DestroyChild(SWindow *pChild);

  public:
    void SetMatrix(const SMatrix &mtx);

    /**
     * GetWindowText
     * @brief    获得窗口文本
     * @return   SStringT
     *
     * Describe
     */
    virtual SStringT GetWindowText(BOOL bRawText = FALSE);

    virtual SStringT GetToolTipText();

    /**
     * GetEventSet
     * @brief    获取SEventSet对象
     * @return   SEventSet * SEventSet对象指针
     *
     * Describe
     */
    SEventSet *GetEventSet()
    {
        return &m_evtSet;
    }

    /**
     * GetStyle
     * @brief    获取style对象
     * @return   SwndStyle& style对象
     *
     * Describe
     */
    const SwndStyle &GetStyle() const;
    SwndStyle &GetStyle();

    void SetOwner(SWindow *pOwner);
    SWindow *GetOwner() const;

    /**
     * GetTextAlign
     * @brief    获得文本的对齐标志
     * @return   UINT
     *
     * Describe  获得文本的对齐标志
     */
    UINT GetTextAlign() const;

    CRect GetWindowRect() const;

    virtual CRect GetClientRect() const;
  public: //窗口树结构相关方法
    /**
     * FindChildByID
     * @brief    通过ID查找对应的子窗口
     * @param    int nID --  窗口ID
     * @param    int nDeep --  搜索深度,-1代表无限度
     * @return   SWindow*
     *
     * Describe
     */
    SWindow *FindChildByID(int nID, int nDeep = -1);

    template <class T>
    T *GetLayoutParamT()
    {
        return sobj_cast<T>(GetLayoutParam());
    }

    /**
     * FindChildByID2
     * @brief    FindChildByID的模板类，支持类型转换
     * @param    int nID --  窗口ID
     * @param    int nDeep --  搜索深度,-1代表无限度
     * @return   T*
     *
     * Describe
     */
    template <class T>
    T *FindChildByID2(int nID, int nDeep = -1)
    {
        SWindow *pTarget = FindChildByID(nID, nDeep);

        if (!pTarget || !pTarget->IsClass(T::GetClassName()))
        {
            SSLOGFMTD(_T("FindChildByID2 Failed, no window of class [%s] with id of [%d] was found within [%d] levels"), T::GetClassName(), nID, nDeep);
            return NULL;
        }
        return (T *)pTarget;
    }

    /**
     * FindChildByName
     * @brief    通过名字查找子窗口
     * @param    const SStringW & pszName --  窗口name属性
     * @param    int nDeep --  搜索深度,-1代表无限度
     * @return   SWindow*
     *
     * Describe
     */
    SWindow *FindChildByName(LPCWSTR strName, int nDeep = -1);

    SWindow *FindChildByName(LPCSTR strName, int nDeep = -1);

    /**
     * FindChildByName2
     * @brief    FindChildByName的模板类，支持类型转换
     * @param    LPCWSTR pszName --  窗口name属性
     * @param    int nDeep --  搜索深度,-1代表无限度
     * @return   T*
     *
     * Describe
     */
    template <class T>
    T *FindChildByName2(LPCWSTR pszName, int nDeep = -1)
    {
        SWindow *pTarget = FindChildByName(pszName, nDeep);
        if (!pTarget || !pTarget->IsClass(T::GetClassName()))
        {
            SSLOGFMTD(_T("FindChildByName2 Failed, no window of class [%s] with name of [%s] was found within [%d] levels"), T::GetClassName(), pszName, nDeep);
            return NULL;
        }
        return (T *)pTarget;
    }

    template <class T>
    T *FindChildByName2(LPCSTR pszName, int nDeep = -1)
    {
        return FindChildByName2<T>(S_CA2W(pszName), nDeep);
    }

    template <class T>
    T *FindChildByClass(int nDeep = -1) const
    {
        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while (pChild)
        {
            if (pChild->IsClass(T::GetClassName()))
                return (T *)pChild;
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }

        if (nDeep > 0)
            nDeep--;
        if (nDeep == 0)
            return NULL;

        pChild = GetWindow(GSW_FIRSTCHILD);
        while (pChild)
        {
            SWindow *pChildFind = pChild->FindChildByClass<T>(nDeep);
            if (pChildFind)
                return (T *)pChildFind;
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }

        return NULL;
    }

  protected:
	  IScriptModule * GetScriptModule();

    /**
     * InvalidateRect
     * @brief    刷新窗口
     * @param    const CRect & rect --  刷新区域
     * @param    BOOL bFromThis --
     * 是否是由本窗口触发的刷新，子窗口的刷新导致父窗口刷新时bFromThis为FALSE
     * @return   void
     *
     * Describe
     */
    void InvalidateRect(const CRect &rect, BOOL bFromThis = TRUE,BOOL bClip=FALSE);
    STransformation GetTransformation() const;

  protected:
    STDMETHOD_(void, OnAnimationStart)(THIS_ IAnimation *animation);
    STDMETHOD_(void, OnAnimationStop)(THIS_ IAnimation *animation);
    STDMETHOD_(void, OnAnimationRepeat)(THIS_ IAnimation *animation);

  protected:
    virtual void OnAnimationInvalidate(IAnimation *pAni, bool bErase);

  protected:
    virtual void OnContentChanged();

    /**
     * OoColorize
     * @brief    调节窗口色调
     * @param    COLORREF cr --  目标颜色
     * @return   void
     *
     * Describe
     */
    virtual void OnColorize(COLORREF cr);

    // Set current cursor, when hover
    virtual BOOL OnSetCursor(const CPoint &pt);

    virtual void OnStateChanging(DWORD dwOldState, DWORD dwNewState);
    virtual void OnStateChanged(DWORD dwOldState, DWORD dwNewState);

	virtual void OnCaptureChanged(BOOL bCaptured);
    /**
     * OnRelayout
     * @brief    窗口位置发生变化
     * @param    const CRect & rcWnd --  新位置
     * @return   void
     *
     * Describe  窗口位置发生变化,更新窗口及计算子窗口位置
     */
    virtual BOOL OnRelayout(const CRect &rcWnd);

  public: // Virtual functions
    virtual SIZE MeasureContent(int nParentWid, int nParentHei);

    /**
     * OnUpdateToolTip
     * @brief    处理tooltip
     * @param    const CPoint & pt --  测试点
     * @param [out]  SwndToolTipInfo & tipInfo -- tip信息
     * @return   BOOL -- TRUE:更新tooltip，FALSE:不更新tooltip
     *
     * Describe
     */
    virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);

    /**
     * GetSelectedSiblingInGroup
     * @brief    获得在一个group中选中状态的窗口
     * @return   SWindow *
     *
     * Describe  不是group中的窗口时返回NULL
     */
    virtual SWindow *GetSelectedSiblingInGroup()
    {
        return NULL;
    }

    /**
     * GetSelectedChildInGroup
     * @brief    获取有选择状态的子窗口
     * @return   SWindow * -- 选中状态窗口
     * Describe
     */
    virtual SWindow *GetSelectedChildInGroup();

    virtual BOOL CreateChildren(SXmlNode xmlNode);

    virtual SWindow *CreateChildByName(LPCWSTR pszName);

    /**
     * RequestRelayout
     * @brief    请求重新布局
     * @param    SWindow *pSource --  请求重新布局的源窗口
     * @return   void
     *
     * Describe
     */
    virtual void RequestRelayout(SWND hSource, BOOL bSourceResizable);

    virtual SStringW tr(const SStringW &strSrc) const;

    virtual SWND SwndFromPoint(CPoint &pt, BOOL bIncludeMsgTransparent = false) const;

    virtual BOOL OnNcHitTest(CPoint pt);

    /**
     * OnUpdateFloatPosition
     * @brief    给float窗口一个自己实现自动布局的机会
     * @param    const CRect & rcParent --  父窗口位置
     * @return   void
     *
     * Describe  2016年3月2日新增加方法
     */
    virtual void OnUpdateFloatPosition(const CRect &rcParent){};

    /**
     * NeedRedrawWhenStateChange
     * @brief    定义状态改变时控件是否重绘
     * @return   BOOL -- TRUE：重绘
     *
     * Describe
     */
    virtual BOOL NeedRedrawWhenStateChange();

    /**
     * GetTextRect
     * @brief    计算窗口中文本的显示位置
     * @param  [out]  LPRECT pRect --  期望文本的显示位置
     * @return   void
     *
     * Describe
     */
    virtual void GetTextRect(LPRECT pRect);

    /**
     * DrawText
     * @brief    绘制文本方法
     * @param    IRenderTarget * pRT --  渲染RT
     * @param    LPCTSTR pszBuf --  文本
     * @param    int cchText --  文本长度
     * @param    LPRECT pRect --  限制渲染范围
     * @param    UINT uFormat --  格式
     * @return   void
     *
     * Describe  子控件重载该方法来定义具体绘制行为
     */
    virtual void DrawText(IRenderTarget *pRT, LPCTSTR pszBuf, int cchText, LPRECT pRect, UINT uFormat);

    /**
     * DrawFocus
     * @brief    绘制控件的焦点状态
     * @param    IRenderTarget * pRT --  渲染RT
     * @return   void
     *
     * Describe  默认绘制一个虚线框
     */
    virtual void DrawFocus(IRenderTarget *pRT);

    /**
     * BeforePaint
     * @brief    为RT准备好当前窗口的绘图环境
     * @param    IRenderTarget * pRT --  渲染RT
     * @param    SPainter & painter --  painter storage
     * @return   void
     *
     * Describe
     */
    virtual void BeforePaint(IRenderTarget *pRT, SPainter &painter);

    /**
     * AfterPaint
     * @brief    恢复由BeforePaint设置的RT状态
     * @param    IRenderTarget * pRT --  渲染RT
     * @param    SPainter & painter --  painter storage
     * @return   void
     *
     * Describe
     */
    virtual void AfterPaint(IRenderTarget *pRT, SPainter &painter);

    /**
     * GetTrCtx
     * @brief    Get Translator Context for The Window
     * @return   SStringW, Translator Context for The Window
     *
     * Describe
     */
    virtual LPCWSTR GetTrCtx() const;

  public: // render相关方法
    /**
     * RedrawRegion
     * @brief    将窗口及子窗口内容绘制到RenderTarget
     * @param    IRenderTarget * pRT --  渲染目标RT
     * @param    IRegion * pRgn --  渲染区域，为NULL时渲染整个窗口
     * @return   void
     *
     * Describe
     */
    void RedrawRegion(IRenderTarget *pRT, IRegionS *pRgn);

    /**
     * GetRenderTarget
     * @brief    获取一个与SWND窗口相适应的内存DC
     * @param    LPCRECT pRc --  RT范围
     * @param    DWORD gdcFlags --  同OLEDCFLAGS
     * @return   IRenderTarget *
     *
     * Describe  使用ReleaseRenderTarget释放
     */
    IRenderTarget *GetRenderTarget(LPCRECT pRc = NULL, GrtFlag gdcFlags = GRT_NODRAW, BOOL bClientRT = TRUE);

    /**
     * GetRenderTarget
     * @brief    获取一个与SWND窗口相适应的内存DC
     * @param    DWORD gdcFlags --  同OLEDCFLAGS
     * @param    IRegion *pRgn --  RT范围
     * @return   IRenderTarget *
     *
     * Describe  使用ReleaseRenderTarget释放
     */
    IRenderTarget *GetRenderTarget(GrtFlag gdcFlags, IRegionS *pRgn);

    /**
     * ReleaseRenderTarget
     * @brief
     * @param    IRenderTarget * pRT --  释放由GetRenderTarget获取的RT
     * @return   void
     *
     * Describe
     */
    void ReleaseRenderTarget(IRenderTarget *pRT);

    /**
     * PaintBackground
     * @brief    画窗口的背景内容
     * @param    IRenderTarget * pRT --  目标RT
     * @param    LPRECT pRc --  目标位置
     * @return   void
     *
     * Describe  目标位置必须在窗口位置内
     */
    void PaintBackground(IRenderTarget *pRT, LPRECT pRc);

    /**
     * PaintForeground
     * @brief    画窗口的前景内容
     * @param    IRenderTarget * pRT --  目标RT
     * @param    LPRECT pRc --  目标位置
	 * @param    SWindow *pStartFrom -- 绘制开始窗口,默认NULL从root开始
     * @return   void
     *
     * Describe  目标位置必须在窗口位置内,不包括当前窗口的子窗口
     */
    void PaintForeground(IRenderTarget *pRT, LPRECT pRc,SWindow *pStartFrom=NULL);
    /**
     * BeforePaintEx
     * @brief    为DC准备好当前窗口的绘图环境,从顶层窗口开始设置
     * @param    IRenderTarget * pRT --  渲染RT
     * @return   void
     *
     * Describe  一般应该和CreateRanderTarget配合使用
     */
    void BeforePaintEx(IRenderTarget *pRT);

    void TransformPoint(CPoint &pt) const;

    void TransformPointEx(CPoint &pt) const;

    BOOL FireEvent(SEvtArgs &evt)
    {
        return FireEvent(&evt);
    }

  protected: // cache相关方法
    /**
     * IsCacheDirty
     * @brief    查询Cache的Dirty标志
     * @return   bool -- true表示Cache已经Dirty
     * Describe
     */
    bool IsCacheDirty() const;

    /**
     * MarkCacheDirty
     * @brief    标记Cache的Dirty标志
     * @param    bool bDirty --  Dirty标志
     * @return   void
     * Describe
     */
    void MarkCacheDirty(bool bDirty);

    /**
     * IsDrawToCache
     * @brief    查看当前是否是把窗口内容绘制到cache上
     * @return   bool -- true绘制到cache上。
     * Describe
     */
    virtual bool IsDrawToCache() const;

    /**
     * IsLayeredWindow
     * @brief    determine whether this window need a render layer.
     * @return   BOOL -- TRUE: need render layer, FALSE: no render layer.
     * Describe
     */
    virtual BOOL IsLayeredWindow() const;

    /**
     * DispatchPaint
     * @brief
     * @param    IRenderTarget * pRT -- 渲染RT
     * @param    IRegion *pRgn -- paint region
     * @param    UINT iBeginZorder -- begin zorder
     * @param    UINT iEndZorder -- end zorder
     * @return   void
     *
     * Describe
     */
    virtual void DispatchPaint(IRenderTarget *pRT, IRegionS *pRgn, UINT iZorderBegin, UINT iZorderEnd);

    /**
     * OnCommitSurface
     * @brief
     * @param    IRenderTarget * pRtDest -- dest rt
     * @param    LPCRECT pRcDest -- dest rect
     * @param    IRenderTarget *pRtSrc -- src rt
     * @param    LPCRECT pRcSrc -- src rect
     * @param    BYTE alpha -- alpha
     * @return   void
     *
     * Describe
     */
    virtual void OnCommitSurface(IRenderTarget *pRtDest, LPCRECT pRcDest, IRenderTarget *pRtSrc, LPCRECT pRcSrc, BYTE alpha);

  protected: // helper functions
    virtual SWindow *_FindChildByID(int nID, int nDeep);
    virtual SWindow *_FindChildByName(const SStringW &strName, int nDeep);

    /**
     * _GetCurrentRenderContainer
     * @brief    获得当前窗口所属的渲染层宿主窗口
     * @return   SWindow * -- 渲染层宿主窗口
     * Describe
     */
    SWindow *_GetCurrentLayeredWindow();

    bool _ApplyMatrix(IRenderTarget *pRT, SMatrix &oriMtx);
    SMatrix _GetMatrixEx() const;
    SAutoRefPtr<IRegionS> _ConvertRect2RenderRegion(const CRect &rc) const;
    bool _WndRectInRgn(const CRect &rc, const IRegionS *rgn) const;

    //将窗口内容绘制到RenderTarget上
    void _PaintClient(IRenderTarget *pRT);
    void _PaintNonClient(IRenderTarget *pRT);
    void _RedrawNonClient();
    void _PaintRegion(IRenderTarget *pRT, IRegionS *pRgn, UINT iZorderBegin, UINT iZorderEnd);

    void _PaintChildren(IRenderTarget *pRT, IRegionS *pRgn, UINT iBeginZorder, UINT iEndZorder);

    void DrawDefFocusRect(IRenderTarget *pRT, CRect rc);

    void UpdateCacheMode();

    void TestMainThread();

    void GetScaleSkin(SAutoRefPtr<ISkinObj> &pSkin, int nScale);

  protected: // Message Handler

    int OnCreate(LPVOID);

    void OnSize(UINT nType, CSize size);

    void OnDestroy();

    BOOL OnEraseBkgnd(IRenderTarget *pRT);

    void OnPaint(IRenderTarget *pRT);

    void OnNcPaint(IRenderTarget *pRT);

    void OnShowWindow(BOOL bShow, UINT nStatus);

    void OnEnable(BOOL bEnable, UINT nStatus);

    void OnLButtonDown(UINT nFlags, CPoint pt);

    void OnLButtonUp(UINT nFlags, CPoint pt);

    void OnRButtonDown(UINT nFlags, CPoint point);

    void OnRButtonUp(UINT nFlags, CPoint point);

    void OnMouseHover(UINT nFlags, CPoint ptPos);

    void OnMouseMove(UINT nFlags, CPoint pt);

    void OnMouseLeave();

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    void OnSetFocus(SWND wndOld);
    void OnKillFocus(SWND wndFocus);

    LRESULT OnSetScale(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnSetLanguage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnSetColorize(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnUpdateFont(UINT uMsg, WPARAM wParam, LPARAM lParam);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
        MSG_WM_NCPAINT_EX(OnNcPaint)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SHOWWINDOW(OnShowWindow)
        MSG_WM_ENABLE_EX(OnEnable)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_RBUTTONDOWN(OnRButtonDown)
        MSG_WM_RBUTTONUP(OnRButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MESSAGE_HANDLER_EX(UM_SETLANGUAGE, OnSetLanguage)
        MESSAGE_HANDLER_EX(UM_SETSCALE, OnSetScale)
        MESSAGE_HANDLER_EX(UM_SETCOLORIZE, OnSetColorize)
        MESSAGE_HANDLER_EX(UM_UPDATEFONT, OnUpdateFont)
    SOUI_MSG_MAP_END_BASE() //消息不再往基类传递，此外使用SOUI_MSG_MAP_END_BASE而不是SOUI_MSG_MAP_END

  protected:
    // 属性处理函数
    HRESULT OnAttrVisible(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrEnable(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrDisplay(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrCache(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrAlpha(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrSkin(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrLayout(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrClass(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrTrackMouseEvent(const SStringW &strValue, BOOL bLoading);
	HRESULT OnAttrVideoCanvas(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrID(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrName(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrTip(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrText(const SStringW &strValue, BOOL bLoading);

    HRESULT DefAttributeProc(const SStringW &strAttribName, const SStringW &strValue, BOOL bLoading);

    virtual HRESULT AfterAttribute(const SStringW &strAttribName, const SStringW &strValue, BOOL bLoading, HRESULT hr);

    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"layout", OnAttrLayout)
        ATTR_CUSTOM(L"class", OnAttrClass) //解析style
        ATTR_CUSTOM(L"id", OnAttrID)
        ATTR_CUSTOM(L"name", OnAttrName)
        ATTR_CUSTOM(L"skin", OnAttrSkin)      //直接获得皮肤对象
        ATTR_SKIN(L"ncskin", m_pNcSkin, TRUE) //直接获得皮肤对象
        ATTR_INT(L"data", m_uData, 0)
        ATTR_CUSTOM(L"text", OnAttrText)
        ATTR_STRINGW(L"trCtx", m_strTrCtx, FALSE)
        ATTR_CUSTOM(L"enable", OnAttrEnable)
        ATTR_CUSTOM(L"visible", OnAttrVisible)
        ATTR_CUSTOM(L"show", OnAttrVisible)
        ATTR_CUSTOM(L"display", OnAttrDisplay)
        ATTR_CUSTOM(L"cache", OnAttrCache)
        ATTR_CUSTOM(L"alpha", OnAttrAlpha)
        ATTR_BOOL(L"layeredWindow", m_bLayeredWindow, TRUE)
        ATTR_CUSTOM(L"trackMouseEvent", OnAttrTrackMouseEvent)
		ATTR_CUSTOM(L"videoCanvas",OnAttrVideoCanvas)
        ATTR_CUSTOM(L"tip", OnAttrTip)
        ATTR_BOOL(L"msgTransparent", m_bMsgTransparent, FALSE)
        ATTR_BOOL(L"msgTransparentSelf", m_bMsgTransparentSelf, FALSE)
        ATTR_LAYOUTSIZE(L"maxWidth", m_nMaxWidth, FALSE)
        ATTR_BOOL(L"clipClient", m_bClipClient, FALSE)
        ATTR_BOOL(L"focusable", m_bFocusable, FALSE)
        ATTR_BOOL(L"drawFocusRect", m_bDrawFocusRect, TRUE)
        ATTR_BOOL(L"float", m_bFloat, FALSE)
        ATTR_CHAIN(m_style, HRET_FLAG_STYLE)                   //交给SwndStyle处理
        ATTR_CHAIN_PTR(m_pLayout, HRET_FLAG_LAYOUT)            //交给Layout处理
        ATTR_CHAIN_PTR(m_pLayoutParam, HRET_FLAG_LAYOUT_PARAM) //交给LayoutParam处理
    SOUI_ATTRS_END()

  protected:
    virtual HRESULT OnLanguageChanged();

    virtual void OnScaleChanged(int scale);

    virtual void OnRebuildFont();

    virtual void OnInsertChild(SWindow *pChild);

    virtual void OnRemoveChild(SWindow *pChild);

    virtual void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer);

  private:
    CRect m_rcWindow; /**<
                         窗口在容器中的位置,由于它的值包含POS_INIT等，调整为private，不允许派生类中直接访问该变量的值
                       */

  protected:
    //布局脏标志类型
    enum LayoutDirtyType
    {
        dirty_clean = 0, //干净
        dirty_self = 1,  //自己变脏
        dirty_child = 2, //有孩子变脏
    };

    SWND m_swnd;   /**< 窗口句柄 */
    BOOL m_bFloat; /**< 窗口位置固定不动的标志 */

    ISwndContainer *m_pContainer; /**< 容器对象 */
    SEventSet m_evtSet;           /**< 窗口事件集合 */

    SAutoRefPtr<ILayout> m_pLayout;
    SAutoRefPtr<ILayoutParam> m_pLayoutParam;

    SWindow *m_pOwner;       /**< 容器Owner，事件分发时，会把事件交给Owner处理 */
    SWindow *m_pParent;      /**< 父窗口 */
    SWindow *m_pFirstChild;  /**< 第一子窗口 */
    SWindow *m_pLastChild;   /**< 最后窗口 */
    SWindow *m_pNextSibling; /**< 后一兄弟窗口 */
    SWindow *m_pPrevSibling; /**< 前一兄弟窗口 */
    UINT m_nChildrenCount;   /**< 子窗口数量 */

    SWNDMSG *m_pCurMsg; /**< 当前正在处理的窗口消息 */

    SwndStyle m_style;        /**< 窗口Style，是一组窗口属性 */
    STrText m_strText;        /**< 窗口文字 */
    STrText m_strToolTipText; /**< 窗口ToolTip */
    SStringW m_strName;       /**< 窗口名称 */
    SStringW m_strTrCtx;      /**< translate context. empty than use container's tr ctx*/
    int m_nID;                /**< 窗口ID */
    UINT m_uZorder;           /**< 窗口Zorder */
    int m_nUpdateLockCnt;     /**< 暂时锁定更新Count，锁定后，不向宿主发送Invalidate */

    DWORD m_dwState;      /**< 窗口在渲染过程中的状态 */
    DWORD m_bVisible : 1; /**< 窗口可见状态 */
    DWORD m_bDisable : 1; /**< 窗口禁用状状态 */
    DWORD m_bDisplay : 1; /**< 窗口隐藏时是否占位，不占位时启动重新布局 */
    DWORD
    m_bClipClient : 1;           /**<
                                    窗口绘制时做clip客户区处理的标志,由于clip可能增加计算量，只在绘制可能走出客户区时才设置*/
    DWORD m_bMsgTransparent : 1; /**< 接收消息标志 TRUE-不处理消息 */
    DWORD m_bMsgTransparentSelf : 1; /**< 接收消息标志 TRUE-不处理消息本身消息但是依旧处理child消息 */
    DWORD m_bFocusable : 1;      /**< 窗口可获得焦点标志 */
    DWORD m_bDrawFocusRect : 1;  /**< 绘制默认的焦点虚框 */
    DWORD m_bCacheDraw : 1;      /**< 支持窗口内容的Cache标志 */
    DWORD m_bCacheDirty : 1;     /**< 缓存窗口脏标志 */
    DWORD m_bLayeredWindow : 1;  /**< 指示是否是一个分层窗口 */
    DWORD m_bMsgHandled : 1;

    LayoutDirtyType m_layoutDirty;         /**< 布局脏标志 参见LayoutDirtyType */
    SAutoRefPtr<IRenderTarget> m_cachedRT; /**< 缓存窗口绘制的RT */
    SAutoRefPtr<IRegionS> m_clipRgn;       /**< 窗口Region */
    SAutoRefPtr<IPathS> m_clipPath;        /**< 窗口Path */
    SAutoRefPtr<ISkinObj> m_pBgSkin;       /**< 背景skin */
    SAutoRefPtr<ISkinObj> m_pNcSkin;       /**< 非客户区skin */
    ULONG_PTR m_uData;                     /**< 窗口的数据位,可以通过GetUserData获得 */

    SLayoutSize m_nMaxWidth; /**< 自动计算大小时，窗口的最大宽度 */

    COLORREF m_crColorize; /**< 调色值 */

    SAutoRefPtr<IAnimation> m_animation; /**< Animation */
    SAnimationHandler m_animationHandler;
    STransformation m_transform;
    bool m_isAnimating;
    bool m_isDestroying;

    typedef struct GETRTDATA
    {
        CRect rcRT;                /**< GETRT调用的有效范围 */
        GrtFlag gdcFlags;          /**< GETRT绘制标志位 */
        SAutoRefPtr<IRegionS> rgn; /**< 保存一个和rcRT对应的IRegion对象 */
        SAutoRefPtr<IRenderTarget> rt;/**< 保存GetRenderTarget时创建的对象,重绘时使用它的缓存绘制 */
    } * PGETRTDATA;

    PGETRTDATA m_pGetRTData;

    SAutoRefPtr<IAttrStorage> m_attrStorage; /**< 属性保存对象 */

	FunSwndProc	m_funSwndProc;
#ifdef SOUI_ENABLE_ACC
    SAutoRefPtr<IAccessible> m_pAcc;
    SAutoRefPtr<IAccProxy> m_pAccProxy;
#endif
#ifdef _DEBUG
    DWORD m_nMainThreadId; /**< 窗口宿线程ID */
#endif
};

SNSEND