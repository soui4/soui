#pragma once

#include <interface/sobject-i.h>
#include <interface/sstring-i.h>
#include <interface/SAnimation-i.h>
#include <interface/SRender-i.h>
#include <interface/SMatrix-i.h>
#include <interface/SWndContainer-i.h>
#include <interface/SScriptModule-i.h>
SNSBEGIN

typedef struct ILayout ILayout;
typedef struct ILayoutParam ILayoutParam;
typedef struct IWindow IWindow;

#ifndef UM_GETDESIREDSIZE
#define UM_GETDESIREDSIZE (WM_USER+10000) //wp=parent wid,lp=parent hei, return size
#endif
typedef BOOL (* FunSwndProc)(IWindow * pSwnd,UINT uMsg,WPARAM wp,LPARAM lp,LRESULT *pbHandled);

#undef INTERFACE
#define INTERFACE IWindow
DECLARE_INTERFACE_(IWindow, IObject)
{
#include <interface/SobjectApi.h>

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief ��ѯ�ؼ��ӿ�
     * @param id REFGUID--�ؼ��ӿ�ID
     * @param [out] ppRet IObjRef * *--�ؼ��ӿ�
     * @return S_OK--�ɹ�
     */
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFGUID id, IObjRef * *ppRet) PURE;

    /**
     * @brief ��ȡ�ؼ������ӿ�
     * @return ISwndContainer *--�ؼ������ӿ�
     */
    STDMETHOD_(ISwndContainer *, GetContainer)(THIS) PURE;

    /**
     * @brief ���ÿؼ������ӿ�
     * @param pContainer ISwndContainer *--�ؼ������ӿ�
     * @return
     * @remark �ڲ�����
     */
    STDMETHOD_(void, SetContainer)(THIS_ ISwndContainer * pContainer) PURE;

    /**
     * @brief ��ȡSOUI�������
     * @return SWND--SOUI�������
     */
    STDMETHOD_(SWND, GetSwnd)(CTHIS) SCONST PURE;

    /**
     * @brief ��ȡ���ֶ���
     * @return ILayout *--���ֶ���
     */
    STDMETHOD_(ILayout *, GetLayout)(THIS) PURE;

    /**
     * @brief ��ȡ���ֲ�������
     * @return ILayoutParam *--���ֲ�������
     */
    STDMETHOD_(ILayoutParam *, GetLayoutParam)(CTHIS) SCONST PURE;

    /**
     * @brief ���ò��ֲ�������
     * @param pLayoutParam ILayoutParam *--���ֲ�������
     * @return TRUE--�ɹ���FALSE--ʧ��
     * @remark ֻ�в��ֲ�������ʹ��ڵĲ�������ƥ��Ż����óɹ�
     */
    STDMETHOD_(BOOL, SetLayoutParam)(THIS_ ILayoutParam * pLayoutParam) PURE;

    /**
     * @brief ��ȡ��ǰ���ڲ����Զ����ֱ�־
     * @return TRUE--���벼��,FALSE--�����벼�֣���Ҫ�û��ڴ������ֶ�����
     */
    STDMETHOD_(BOOL, IsFloat)(CTHIS) SCONST PURE;

    /**
     * @brief ��ȡ����ռλ��־
     * @return TRUE--�������ص�ʱ��ռ�иò���λ�ã�FALSE--����ʱ��ռλ
     */
    STDMETHOD_(BOOL, IsDisplay)(CTHIS) SCONST PURE;

    /**
     * @brief ��ȡ�����Ƿ��������Ϣ
     * @return TRUE--�����Ϣ͸����������
     */
    STDMETHOD_(BOOL, IsMsgTransparent)(CTHIS) SCONST PURE;
    STDMETHOD_(BOOL, IsMsgTransparentSelf)(CTHIS) SCONST PURE;

    /**
     * @brief ��ȡ�����Ƿ��Զ����ÿͻ���
     * @return TRUE--�Զ����ÿͻ���
     * @remark ���ÿͻ����������ƴ��ڼ������Ӵ��ڻ��Ʋ�������ǰ���ڿͻ���������Ҫ����һ�μ��ò���
     */
    STDMETHOD_(BOOL, IsClipClient)(CTHIS) SCONST PURE;

    /**
     * @brief ����tooltip
     * @param pszText LPCTSTR--tooltip�ַ���
     * @return
     * @remark ֧�ֶ�����
     */
    STDMETHOD_(void, SetToolTipText)(THIS_ LPCTSTR pszText) PURE;

	/**
     * @brief ����tooltip
     * @param pszText LPCSTR--tooltip�ַ���
     * @return
     * @remark ֧�ֶ�����
     */
    STDMETHOD_(void, SetToolTipTextA)(THIS_ LPCSTR pszText) PURE;

    /**
     * @brief ��ȡ���ڵ�check״̬��־
     * @return TRUE--checked
     */
    STDMETHOD_(BOOL, IsChecked)(CTHIS) SCONST PURE;

    /**
     * @brief ���ô���check״̬
     * @param bCheck BOOL--check״̬
     * @return
     */
    STDMETHOD_(void, SetCheck)(THIS_ BOOL bCheck) PURE;

    /**
     * @brief ��ȡ���ڵĽ���״̬
     * @return TRUE--���ڽ���
     */
    STDMETHOD_(BOOL, IsDisabled)(CTHIS_ BOOL bCheckParent) SCONST PURE;

    /**
     * @brief ���ô�������״̬
     * @param bEnable BOOL--���ô���
     * @param bUpdate BOOL--ͬʱˢ�´��ڱ�־
     * @return
     */
    STDMETHOD_(void, EnableWindow)(THIS_ BOOL bEnable, BOOL bUpdate) PURE;

    /**
     * @brief ��ȡ���ڵĿɼ���־
     * @return TRUE--�ɼ�
     */
    STDMETHOD_(BOOL, IsVisible)(CTHIS_ BOOL bCheckParent) SCONST PURE;

    /**
     * @brief ���ô��ڿɼ���־
     * @param bVisible BOOL--�ɼ���
     * @param bUpdate  BOOL--ͬʱˢ�´��ڱ�־
     * @return
     */
    STDMETHOD_(void, SetVisible)(THIS_ BOOL bVisible, BOOL bUpdate) PURE;

    /**
     * @brief ��ȡ���ڵ��û�����
     * @return ULONG_PTR--�û�����
     */
    STDMETHOD_(ULONG_PTR, GetUserData)(CTHIS) SCONST PURE;

    /**
     * @brief ���ô����û�����
     * @param uData ULONG_PTR--���û�����
     * @return ULONG_PTR--ԭ�û�����
     */
    STDMETHOD_(ULONG_PTR, SetUserData)(THIS_ ULONG_PTR uData) PURE;

    /**
     * @brief ִ�е�ɫ
     * @param cr COLORREF--Ŀ��ɫ��
     * @return
     */
    STDMETHOD_(void, DoColorize)(THIS_ COLORREF cr) PURE;

    /**
     * @brief ��ȡ��ǰ��ɫ��ɫ��
     * @return COLORREF--ɫ��
     */
    STDMETHOD_(COLORREF, GetColorizeColor)(CTHIS) SCONST PURE;

    /**
     * @brief ����һ����Ϣ
     * @param uMsg UINT-��Ϣ����
     * @param wParam WPARAM--����1
     * @param lParam LPARAM--����2
     * @param [out] pbMsgHandled BOOL*--��Ϣ�������־
     * @return
     */
    STDMETHOD_(LRESULT, SSendMessage)
    (THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL * pbMsgHandled) PURE;

    /**
     * @brief �ַ�һ����Ϣ����ǰ���ڼ��������Ӵ���
     * @param uMsg UINT-��Ϣ����
     * @param wParam WPARAM--����1
     * @param lParam LPARAM--����2
     * @return
     */
    STDMETHOD_(void, SDispatchMessage)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;

    /**
     * @brief ��ȡ�����Ƿ���������Ϊ����
     * @return TRUE--����
     */
    STDMETHOD_(BOOL, IsFocusable)(CTHIS) SCONST PURE;

    /**
     * @brief �ж��Ƿ�Ϊ���㴰��
     * @return TRUE--���㴰��
     */
    STDMETHOD_(BOOL, IsFocused)(CTHIS) SCONST PURE;

    /**
     * @brief ����ǰ��������Ϊ���㴰��
     * @return
     */
    STDMETHOD_(void, SetFocus)(THIS) PURE;

    /**
     * @brief ȡ������
     * @return
     */
    STDMETHOD_(void, KillFocus)(THIS) PURE;

    /**
     * @brief ˢ�µ�ǰ���ڿͻ���
     * @return
     */
    STDMETHOD_(void, Invalidate)(THIS) PURE;

    /**
     * @brief ˢ�µ�ǰ����ָ��λ��
     * @param lprect LPCRECT--����λ��
     * @return
     * @remark ���԰����ǿͻ��������ǲ�ˢ�´�����λ��
     */
    STDMETHOD_(void, InvalidateRect)(THIS_ LPCRECT lprect) PURE;

    /**
     * @brief ��סˢ�£��ж�ˢ�µ���
     * @return
     */
    STDMETHOD_(void, LockUpdate)(THIS) PURE;

    /**
     * @brief ����ˢ�£��ָ�ˢ�µ���
     * @return
     */
    STDMETHOD_(void, UnlockUpdate)(THIS) PURE;

    /**
     * @brief ��ȡˢ�¼���״̬
     * @return TRUE--��ǰˢ��Ϊ����״̬
     */
    STDMETHOD_(BOOL, IsUpdateLocked)(CTHIS) SCONST PURE;

    /**
     * @brief �����������´���
     * @return
     * @remark ͨ��Ҫˢ��һ�����ڣ�Ӧ��ʹ��Invalidate/InvalidateRect������ϵͳ���������󶼻���
     ��һ��ˢ�µ�ʱ�����»��ơ���������ܻ������ʱ��Ϊ���ܹ��������»��ƣ����Ե���Update
    */
    STDMETHOD_(void, Update)(THIS) PURE;

    /**
     * @brief ���ô�����ʾ����
     * @param pRgn IRegionS*--��ʾ����
     * @param bRedraw BOOL--���»���
     * @return
     * @remark ��������Ͻ�����Ϊ(0,0),����ʾ�����л���ݴ�����ʵ�������λ��
     */
    STDMETHOD_(void, SetWindowRgn)(THIS_ IRegionS * pRgn, BOOL bRedraw) PURE;

    /**
     * @brief ��ȡ������ʾ����
     * @return IRegionS*--��ʾ����
     */
    STDMETHOD_(IRegionS *, GetWindowRgn)(CTHIS) SCONST PURE;

    /**
     * @brief ���ô�����ʾ����
     * @param pRgn IPathS*--��ʾ����
     * @param bRedraw BOOL--���»���
     * @return
     * @remark ��������Ͻ�����Ϊ(0,0),����ʾ�����л���ݴ�����ʵ�������λ��
     */
    STDMETHOD_(void, SetWindowPath)(THIS_ IPathS * pPath, BOOL bRedraw) PURE;

    /**
     * @brief ��ȡ������ʾ����
     * @return IPathS*--��ʾ����
     */
    STDMETHOD_(IPathS *, GetWindowPath)(CTHIS) SCONST PURE;

    /**
     * Move2
     * @brief    �������ƶ���ָ��λ��
     * @param    int x --  left
     * @param    int y --  top
     * @param    int cx --  width
     * @param    int cy --  height
     * @return   void
     *
     * Describe
     * @see     Move(LPRECT prect)
     */
    STDMETHOD_(void, Move2)(THIS_ int x, int y, int cx DEF_VAL(-1), int cy DEF_VAL(-1)) PURE;

    /**
     * @brief �������ƶ���ָ��λ��
     * @param prect LPCRECT--��������
     * @return
     * remark �����������е�����
     */
    STDMETHOD_(void, Move)(THIS_ LPCRECT prect) PURE;

    /**
     * @brief ��ȡ���ڵ���ʾλ�ã��������ڵķǿͻ���
     * @param [out] LPRECT--������ʾλ��
     * @remark SOUI����λ�����Ͻǲ���(0,0),�������������е�����
     */
    STDMETHOD_(void, GetWindowRect)(CTHIS_ LPRECT prect) SCONST PURE;

    /**
     * @brief ��ȡ���ڵ���ʾλ�ã����������ڵķǿͻ���
     * @param [out] LPRECT--���ڿͻ�����ʾλ��
     * @remark SOUI����λ�����Ͻǲ���(0,0),�������������е�����
     */
    STDMETHOD_(void, GetClientRect)(CTHIS_ LPRECT prect) SCONST PURE;

	/**
     * @brief ��ȡ���ڵľ��������ڼ��ú����ʾλ��
     * @param [out] LPRECT--���ڿͻ�����ʾλ��
     * @remark �������Ӵ��ڼ��ֵܴ��ڵļ���
     */
	STDMETHOD_(void, GetVisibleRect)(CTHIS_ LPRECT prect) SCONST PURE;
    /**
     * @brief ���һ�����ǲ����ڴ��ڷ�Χ��
     * @param pt POINT--���������
     * @param bClientOnly BOOL--���Կͻ�����־
     * @return TRUE--�ڴ��ڷ�Χ��
     */
    STDMETHOD_(BOOL, IsContainPoint)(CTHIS_ POINT pt, BOOL bClientOnly) SCONST PURE;

    /**
     * @brief ���ptָ��Ĵ���
     * @param [in,out] pt POINT *--��������
     * @param bIncludeMsgTransparent BOOL--������Ϣ͸�����ڱ�־
     * @return SWND--����ָ���������㴰��
     */
    STDMETHOD_(SWND, SwndFromPoint)(CTHIS_ POINT * pt, BOOL bIncludeMsgTransparent DEF_VAL(FALSE)) SCONST PURE;

    /**
     * @brief ����������ʱ��
     * @param id char--��ʱ��ID����Χ��0-127
     * @param uElapse UINT--��ʱ����ʱ
     * @return TRUE--�ɹ�
     */
    STDMETHOD_(BOOL, SetTimer)(THIS_ char id, UINT uElapse) PURE;

    /**
     * @brief ɾ��SetTimer�����Ķ�ʱ��
     * @param id char--SetTimer�����Ķ�ʱ��ID
     * @return
     */
    STDMETHOD_(void, KillTimer)(THIS_ char id) PURE;


    /**
     * @brief ��ȡ��겶��
     * @return SWND--��겶�񴰿ھ��
     */
    STDMETHOD_(SWND, GetCapture)(CTHIS) SCONST PURE;

    /**
     * @brief ��������Ϊ��겶��
     * @return SWND--ǰ��겶�񴰿ھ��
     */
    STDMETHOD_(SWND, SetCapture)(THIS) PURE;

    /**
     * @brief �ͷ���겶��
     * @return TRUE--�ͷųɹ�
     */
    STDMETHOD_(BOOL, ReleaseCapture)(THIS) PURE;

    /**
     * @brief ����һ�����ڶ���
     * @param animation IAnimation *--��������
     * @return
     */
    STDMETHOD_(void, SetAnimation)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief ��������һ����������
     * @param animation IAnimation *--��������
     * @return
     * @remark ��setAnimation��ͬ�������������������ǵȴ����������е���ʱ����ʱ��
     */
    STDMETHOD_(void, StartAnimation)(THIS_ IAnimation * animation) PURE;

    /**
     * @brief ��ȡ��ǰ�������еĶ�������
     * @return IAnimation *--��������
     */
    STDMETHOD_(IAnimation *, GetAnimation)(CTHIS) SCONST PURE;

    /**
     * @brief �������
     * @return
     */
    STDMETHOD_(void, ClearAnimation)(THIS) PURE;

    /**
     * @brief ���ô���͸����
     * @param byAlpha BYTE--͸����,[0,255]
     * @return
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE byAlpha) PURE;

    /**
     * @brief ��ȡ����͸����
     * @return BYTE--����͸����
     */
    STDMETHOD_(BYTE, GetAlpha)(CTHIS) SCONST PURE;

    /**
     * @brief ���ô��ڱ任����
     * @param mtx const IMatrix *--�任����
     * @return
     */
    STDMETHOD_(void, SetMatrix)(THIS_ const IMatrix *mtx) PURE;

    /**
     * @brief ��ȡ���ڱ任����
     */
    STDMETHOD_(void, GetMatrix)(CTHIS_ IMatrix * mtx) SCONST PURE;

    /**
     * @brief ��ȡdpi�Ŵ���
     * @return int--dpi�Ŵ���,��100Ϊ����
     */
    STDMETHOD_(int, GetScale)(CTHIS) SCONST PURE;

    /**
     * @brief �������²���
     * @return
     */
    STDMETHOD_(void, RequestRelayout)(THIS) PURE;

    /**
     * @brief ���²��ֵ�ǰ���ڵ��Ӵ��ڣ�ֻ�ڴ��ڲ������˲���Ч
     * @return
     */
    STDMETHOD_(void, UpdateLayout)(THIS) PURE;

    /**
     * @brief ǿ�����²����Ӵ���
     * @return
     */
    STDMETHOD_(void, UpdateChildrenPosition)(THIS) PURE;

    /**
     * @brief ��ȡ�������־
     * @return TRUE--������
     */
    STDMETHOD_(BOOL, IsLayoutDirty)(CTHIS) SCONST PURE;

    /**
     * @brief ��ȡ������Ӧ���̵ı�־λ
     * @return UINT--��Ӧ���̵ı�־λ
     */
    STDMETHOD_(UINT, OnGetDlgCode)(CTHIS) SCONST PURE;

    /**
     * @brief ��ȡ������
     * @return IWindow *--�����ڶ���
     */
    STDMETHOD_(IWindow *, GetIRoot)(CTHIS) SCONST PURE;

    /**
     * @brief ��ȡ������
     * @return IWindow *--�����ڶ���
     */
    STDMETHOD_(IWindow *, GetIParent)(CTHIS) SCONST PURE;

    /**
     * @brief ����uCode��ȡ�뵱ǰ������dom���Ϲ����Ĵ���
     * @param uCode UINT--��������
     * @return IWindow *--��ǰ������dom���Ϲ����Ĵ���
     */
    STDMETHOD_(IWindow *, GetIWindow)(CTHIS_ int uCode) SCONST PURE;

    /**
     * @brief ��ȡ�Ӵ���
     * @param iChild int--�Ӵ������
     * @return
     */
    STDMETHOD_(IWindow *, GetIChild)(CTHIS_ int iChild) SCONST PURE;

    /**
     * @brief ��ȡ�Ӵ�������
     * @return UINT--�Ӵ�������
     */
    STDMETHOD_(UINT, GetChildrenCount)(CTHIS) SCONST PURE;

    /**
     * @brief �ж�һ�������ǲ��ǵ�ǰ���ڵ����ﴰ��
     * @param pTest const IWindow *--���Դ���
     * @return TRUE--�����ﴰ��
     */
    STDMETHOD_(BOOL, IsDescendant)(CTHIS_ const IWindow *pTest) SCONST PURE;

    /**
     * @brief ���ô��ڵ�Owner
     * @param pOwner IWindow *--Owner����
     * @return
     */
    STDMETHOD_(void, SetIOwner)(THIS_ IWindow * pOwner) PURE;

    /**
     * @brief ��ȡOwner����
     * @return IWindow *--Owner����
     */
    STDMETHOD_(IWindow *, GetIOwner)(CTHIS) SCONST PURE;

    /**
     * @brief ����ǰ�����ƶ���ͬ���ֵܴ��ڵ�zorder���
     * @param
     * @return
     */
    STDMETHOD_(void, BringWindowToTop)(THIS) PURE;

    /**
     * AdjustZOrder
     * @brief    ��������Z��
     * @param    SWindow *pInsertAfter --  �������������֮��
     * @return   bool,  pInsertAfter��this��ͬ�����ڷ���ʧ��
     *
     * Describe  pInsertAfter����ΪNULL��������thisͬһ�����ֵܴ���
     */
    STDMETHOD_(BOOL, AdjustIZOrder)(THIS_ IWindow * pInsertAfter) PURE;

    /**
     * InsertChild
     * @brief    �ڴ������в���һ���Ӵ���
     * @param    SWindow * pNewChild --  �Ӵ��ڶ���
     * @param    SWindow * pInsertAfter --  ����λ��
     * @return   void
     *
     * Describe  һ������UI��ʼ����ʱ�򴴽�������Ĵ��ڲ����Զ����벼������
     */
    STDMETHOD_(void, InsertIChild)(THIS_ IWindow * pNewChild, IWindow * pInsertAfter) PURE;

    /**
     * RemoveChild
     * @brief    �Ӵ��������Ƴ�һ���Ӵ��ڶ���
     * @param    SWindow * pChild --  �Ӵ��ڶ���
     * @return   BOOL
     *
     * Describe  �Ӵ��ڲ����Զ��ͷ�
     */
    STDMETHOD_(BOOL, RemoveIChild)(THIS_ IWindow * pChild) PURE;

    /**
     * CreateChildren
     * @brief    ��XML�����Ӵ���
     * @param    LPCWSTR pszXml --  �Ϸ���utf16����XML�ַ���
     * @return   BOOL �Ƿ񴴽��ɹ�
     *
     * Describe
     */
    STDMETHOD_(BOOL, CreateChildrenFromXml)(THIS_ LPCWSTR pszXml) PURE;

    /**
     * @brief ����ID�����Ӵ���
     * @param nId int--�Ӵ���ID
     * @return IWindow*--ƥ�䴰��
     * @remark ���ù�������㷨����ƥ���Ӵ���
     */
    STDMETHOD_(IWindow *, FindIChildByID)(THIS_ int nId) PURE;

    /**
     * @brief ����Name�����Ӵ���
     * @param pszName LPCWSTR--�Ӵ���Name
     * @return IWindow*--ƥ�䴰��
     * @remark ���ù�������㷨����ƥ���Ӵ���
     */
    STDMETHOD_(IWindow *, FindIChildByName)(THIS_ LPCWSTR pszName) PURE;

    /**
     * @brief ����Name�����Ӵ���
     * @param pszName LPCSTR--�Ӵ���Name
     * @return IWindow*--ƥ�䴰��
     * @remark ���ù�������㷨����ƥ���Ӵ���
     */
    STDMETHOD_(IWindow *, FindIChildByNameA)(THIS_ LPCSTR pszName) PURE;

    /**
     * DestroyChild
     * @brief    ����һ���Ӵ���
     * @param    SWindow * pChild --  �Ӵ��ڶ���
     * @return   BOOL
     *
     * Describe  �ȵ���RemoveChild���ٵ���pChild->Release���ͷ��Ӵ��ڶ���
     */
    STDMETHOD_(BOOL, DestroyIChild)(THIS_ IWindow * pChild) PURE;

    /**
     * @brief ���������Ӵ���
     * @return
     */
    STDMETHOD_(void, DestroyAllChildren)(THIS) PURE;

    /**
     * @brief ���ٴ���
     * @return TRUE--�ɹ�
     */
    STDMETHOD_(BOOL, Destroy)(THIS) PURE;

    /**
     * @brief ��ȡָ�����ڵ���һ�����벼�ֵĴ��ڶ���
     * @param pCurChild const IWindow *--��ǰ����
     * @return IWindow *--��һ�����ִ���
     */
    STDMETHOD_(IWindow *, GetNextLayoutIChild)(CTHIS_ const IWindow *pCurChild) SCONST PURE;

    /**
     * GetChildrenLayoutRect
     * @brief    ����Ӵ��ڵĲ��ֿռ�
	 * @param  LPRECT prc--����λ��
     * @return   void
     *
     * Describe  ͨ���ǿͻ���������tab,group�����Ŀؼ���һ��
     */
    STDMETHOD_(void, GetChildrenLayoutRect)(CTHIS_ RECT* prc) SCONST PURE;

    /**
     * GetDesiredSize
     * @brief    ��û��ָ�����ڴ�Сʱ��ͨ����Ƥ�����㴰�ڵ�������С
     * @param    int nParentWid -- ������ȣ�<0�����������������ǰ���ڿ��
     * @param    int nParentHei -- �����߶ȣ�<0���������߶�������ǰ���ڸ߶�
     * @return   CSize
     *
     * Describe
     */
    STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE* pSize, int nParentWid, int nParentHei) PURE;

    /**
     * @brief ��ȡ���ڱ���ɫ
     * @return COLORREF--���ڱ���ɫ
     */
    STDMETHOD_(COLORREF, GetBkgndColor)(CTHIS) SCONST PURE;

    /**
     * @brief ��ƴ�����ʾ�ı�
     * @param lpszText LPCTSTR--������ʾ�ı�
     * @return
     * @remark ֧���Զ�����
     */
    STDMETHOD_(void, SetWindowText)(THIS_ LPCTSTR lpszText) PURE;

    /**
     * @brief ��ƴ�����ʾ�ı�
     * @param lpszText LPCSTR--������ʾ�ı�
     * @return
     * @remark ֧���Զ�����
     */
	STDMETHOD_(void, SetWindowTextA)(THIS_ LPCSTR lpszText) PURE;

    /**
     * @brief ��ȡ������ʾ�ı�
     * @param pBuf TCHAR*--������
     * @param nBufLen int--����������
     * @param bRawText BOOL--��ȡ����ǰ�ı���־
     * @return int--���Ƶ��ı����ȣ�pBufΪNULLʱ�����ܻ���������
     */
    STDMETHOD_(int, GetWindowText)(THIS_ TCHAR * pBuf, int nBufLen, BOOL bRawText) PURE;

	STDMETHOD_(int, GetWindowTextA)(THIS_ IStringA *pStr, BOOL bRawText) PURE;

    /**
     * @brief ��ȡ����״̬
     * @return DWORD -- ����״̬
     */
    STDMETHOD_(DWORD, GetState)(CTHIS) SCONST PURE;

    /**
     * @brief �޸Ĵ���״̬
     * @param dwStateAdd DWORD--����״̬λ
     * @param dwStateRemove DWORD--ɾ��״̬λ
     * @param bUpdate BOOL--ˢ�´���
     * @return DWORD--��״̬
     */
    STDMETHOD_(DWORD, ModifyState)(THIS_ DWORD dwStateAdd, DWORD dwStateRemove, BOOL bUpdate DEF_VAL(FALSE)) PURE;

    /**
     * GetISelectedSiblingInGroup
     * @brief    �����һ��group��ѡ��״̬�Ĵ���
     * @return   SWindow *
     *
     * Describe  ����group�еĴ���ʱ����NULL
     */
    STDMETHOD_(IWindow *, GetISelectedSiblingInGroup)(THIS) PURE;

    /**
     * GetSelectedChildInGroup
     * @brief    ��ȡ��ѡ��״̬���Ӵ���
     * @return   IWindow * -- ѡ��״̬����
     * Describe
     */
    STDMETHOD_(IWindow *, GetISelectedChildInGroup)(THIS) PURE;

    /**
     * @brief ͬ�����ֵܴ����Զ������־
     * @return TRUE--�Զ�����
     * @remark �Զ�����Ĵ������û���ť�����ʱ�����ڴ���ѭ��˳���ý���
     */
    STDMETHOD_(BOOL, IsSiblingsAutoGroupped)(CTHIS) SCONST PURE;

    // caret��ط���

    /**
     * @brief �������
     * @param pBmp ��������λͼ,��Ϊnull
     * @param nWid int--�����
     * @param nHeight int--���߶�
     * @return TRUE--�ɹ�
     */
    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP pBmp, int nWid, int nHeight) PURE;

    /**
     * @brief ���ƹ����ʾ/����
     * @param bShow BOOL--��ʾ��־
     * @return
     */
    STDMETHOD_(void, ShowCaret)(THIS_ BOOL bShow) PURE;

    /**
     * @brief ���ù����ʾλ��
     * @param x int-X
     * @param y int-Y
     * @return
     */
    STDMETHOD_(void, SetCaretPos)(THIS_ int x, int y) PURE;

    /**
     * @brief ���õ�ǰ�ؼ��Ƿ��������¼�
     * @param bMute BOOL-�������¼���־
     * @return
     */
    STDMETHOD_(void, SetEventMute)(THIS_ BOOL bMute) PURE;

    /**
     * @brief ���Ĵ����¼�
     * @param evtId DWORD--�¼�ID
     * @param pSlot const IEvtSlot *--�¼��������
     * @return TRUE--�ɹ�
     */
    STDMETHOD_(BOOL, SubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) PURE;

    /**
     * @brief ȡ�������¼�����
     * @param evtId DWORD--�¼�ID
     * @param pSlot const IEvtSlot *--�¼��������
     * @return TRUE--�ɹ�
     */
    STDMETHOD_(BOOL, UnsubscribeEvent)(THIS_ DWORD evtId, const IEvtSlot *pSlot) PURE;

    /**
     * @brief ���Ӵ����¼�
     * @param evtId DWORD--�¼�ID
     * @param LPCWSTR pszEventHandlerName--�ַ�����ʶ���¼������ű�ʹ�ã�
     * @return TRUE--�ɹ�
     */
	STDMETHOD_(BOOL,AddEvent)(THIS_ DWORD dwEventID, LPCWSTR pszEventHandlerName) PURE;
	
    /**
     * @brief ɾ�������¼�
     * @param evtId DWORD--�¼�ID
     * @return TRUE--�ɹ�
     */
	STDMETHOD_(BOOL,RemoveEvent)(THIS_ DWORD dwEventID) PURE;
    /**
     * @brief  ����һ���¼���Ӧ�ò�
     * @param evt IEvtArgs *--�¼�����
     * @return TRUE--�ɹ�
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs * evt) PURE;

    /**
     * FireCommand
     * @brief    ����ڵ�EVT_CMD�¼�
     * @return   BOOL-- true:EVT_CMD�¼�������
     *
     * Describe
     */
    STDMETHOD_(BOOL, FireCommand)(THIS) PURE;

    /**
     * FireCtxMenu
     * @brief    �����ݲ˵��¼�
     * @param    CPoint pt --  �����λ��
     * @return   BOOL -- true:�ⲿ�����˿�ݲ˵��¼�
     *
     * Describe
     */
    STDMETHOD_(BOOL, FireCtxMenu)(THIS_ POINT pt) PURE;

    /**
     * SwndProc
     * @brief    Ĭ�ϵ���Ϣ������
     * @param    UINT uMsg --  ��Ϣ����
     * @param    WPARAM wParam --  ����1
     * @param    LPARAM lParam --  ����2
     * @param    LRESULT & lResult --  ��Ϣ����ֵ
     * @return   BOOL �Ƿ񱻴���
     *
     * Describe  ����Ϣӳ�����û�д������Ϣ����ú�������
     */
    STDMETHOD_(BOOL, SwndProc)(THIS_ UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult) PURE;

	 /**
     * SetSwndProc
     * @brief    ������Ϣ������
	 * @param    FunSwndProc swndProc --  ��Ϣ������
     * @return   void
     *
     * Describe  
     */
	STDMETHOD_(void, SetSwndProc)(THIS_ FunSwndProc swndProc) PURE;

    /**
     * GetHostHwnd
     * @brief    ��ȡHost Hwnd
     * @return   HWND -- host hwnd
     *
     * Describe  
     */
	STDMETHOD_(HWND, GetHostHwnd)(THIS) PURE;

	/**
     * IsVideoCanvas
     * @brief    ��ѯ�����Ƿ�Ϊ��Ƶ����
     * @return   BOOL -- TRUE:����Ƶ����
     *
     * Describe  
     */
	STDMETHOD_(BOOL,IsVideoCanvas)(CTHIS) SCONST PURE;

	STDMETHOD_(BOOL,RegisterDragDrop)(THIS_ IDropTarget *pDragTarget) PURE;
	STDMETHOD_(BOOL,UnregisterDragDrop)(THIS) PURE;
};

SNSEND