/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 *
 * @file       sapp.h
 * @brief
 * @version    v4.0
 * @author     soui
 * @date       2022-06-20
 *
 * Describe    SOUI应用程序入口
 */

#pragma once
#include <core/ssingleton.h>
#include <interface/sapp-i.h>

#include <res.mgr/SResProviderMgr.h>
#include <res.mgr/SNamedValue.h>

#include <core/smsgloop.h>
#include <core/SObjectFactory.h>
#include <OleAcc.h>
#include <helper/obj-ref-impl.hpp>
#include <interface/SSkinobj-i.h>

#define GETRESPROVIDER    SOUI::SApplication::getSingletonPtr()
#define GETRENDERFACTORY  SOUI::SApplication::getSingleton().GetRenderFactory()
#define GETREALWNDHANDLER SOUI::SApplication::getSingleton().GetRealWndHander()
#define GETTOOLTIPFACTORY SOUI::SApplication::getSingleton().GetToolTipFactory()

#define LOADXML(p1, p2)   SOUI::SApplication::getSingleton().LoadXmlDocment(p1, p2)
#define LOADIMAGE(p1, p2) SOUI::SApplication::getSingleton().LoadImage(p1, p2)
#define LOADIMAGE2(p1)    SOUI::SApplication::getSingleton().LoadImage2(p1)
#define LOADICON(p1, p2)  SOUI::SApplication::getSingleton().LoadIcon(p1, p2, p2)
#define LOADICON2(p1)     SOUI::SApplication::getSingleton().LoadIcon2(p1)
#define TR(p1, p2)        SOUI::SApplication::getSingleton().tr(p1, p2)
#define STR2ID(p1)        SOUI::SApplication::getSingleton().Str2ID(p1)


#define CREATEINTERPOLATOR(x) SOUI::SApplication::getSingleton().CreateInterpolatorByName(x)

SNSBEGIN

class SWindow;
struct IAccProxy;

interface SOUI_EXP ISystemObjectRegister{
    virtual void RegisterLayouts(SObjectFactoryMgr * objFactory) const {} 
	virtual void RegisterSkins(SObjectFactoryMgr * objFactory)  const {} 
	virtual void RegisterWindows(SObjectFactoryMgr * objFactory) const {} 
	virtual void RegisterInterpolator(SObjectFactoryMgr * objFactory) const {} 
	virtual void RegisterAnimation(SObjectFactoryMgr * objFactory) const {} 
	virtual void RegisterValueAnimator(SObjectFactoryMgr * objFactory) const {}
};

class SOUI_EXP SObjectDefaultRegister : public ISystemObjectRegister {
  public:
    void RegisterWindows(SObjectFactoryMgr *objFactory) const;
    void RegisterSkins(SObjectFactoryMgr *objFactory) const;
    void RegisterLayouts(SObjectFactoryMgr *objFactory) const;
    void RegisterInterpolator(SObjectFactoryMgr *objFactory) const;
    void RegisterAnimation(SObjectFactoryMgr *objFactory) const;
    void RegisterValueAnimator(SObjectFactoryMgr *objFactory) const;
};

/**
 * @class     SApplication
 * @brief     SOUI Application
 *
 * Describe   SOUI Application
 */
class SOUI_EXP SApplication
    : public SSingleton<SApplication>
    , public TObjRefImpl<IApplication>
    , public SResProviderMgr
    , public SObjectFactoryMgr {
  public:
    /**
     * SApplication
     * @brief    构造函数
     * @param    IRenderFactory * pRendFactory --  渲染模块
     * @param    HINSTANCE hInst --  应用程序句柄
     * @param    LPCTSTR pszHostClassName --  使用SOUI创建窗口时默认的窗口类名
     * @param    ISystemObjectRegister *pSysObjRegister -- 系统控件注册器
     *
     * Describe
     */
    SApplication(IRenderFactory *pRendFactory,
                 HINSTANCE hInst,
                 LPCTSTR pszHostClassName = _T("SOUIHOST"),
                 const ISystemObjectRegister &sysObjRegister = SObjectDefaultRegister(),
                 BOOL bImeApp = FALSE);

    ~SApplication(void);

  public:
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(THIS) OVERRIDE;

    STDMETHOD_(HMODULE, GetModule)(THIS) SCONST OVERRIDE;
    STDMETHOD_(UINT, LoadSystemNamedResource)(THIS_ IResProvider *pResProvider) OVERRIDE;
    STDMETHOD_(ITranslatorMgr *, GetTranslator)(THIS) OVERRIDE;
    STDMETHOD_(void, SetTranslator)(THIS_ ITranslatorMgr *pTrans) OVERRIDE;
    STDMETHOD_(IToolTipFactory *, GetToolTipFactory)(THIS) OVERRIDE;
    STDMETHOD_(void, SetToolTipFactory)(THIS_ IToolTipFactory *pToolTipFac) OVERRIDE;

    STDMETHOD_(BOOL, SetMsgLoopFactory)(THIS_ IMsgLoopFactory *pMsgLoopFac) OVERRIDE;
    STDMETHOD_(IMsgLoopFactory *, GetMsgLoopFactory)(THIS) OVERRIDE;

    STDMETHOD_(void, SetLogManager)(THIS_ ILogMgr *pLogMgr) OVERRIDE;
    STDMETHOD_(ILogMgr *, GetLogManager)(THIS) OVERRIDE;

    STDMETHOD_(void, SetAttrStorageFactory)
    (THIS_ IAttrStorageFactory *pAttrStorageFactory) OVERRIDE;
    STDMETHOD_(IAttrStorageFactory *, GetAttrStorageFactory)(THIS) OVERRIDE;

    STDMETHOD_(int, Run)(THIS_ HWND hMainWnd) OVERRIDE;
	STDMETHOD_(void,Quit)(THIS_ int nCode) OVERRIDE;
    STDMETHOD_(HWND, GetMainWnd)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, AddMsgLoop)(THIS_ IMessageLoop *pMsgLoop,BOOL bReplace DEF_VAL(FALSE)) OVERRIDE;
    STDMETHOD_(BOOL, RemoveMsgLoop)(THIS) OVERRIDE;

    STDMETHOD_(IMessageLoop *, GetMsgLoop)
    (THIS_ DWORD dwThreadID = ::GetCurrentThreadId()) SCONST OVERRIDE;

    STDMETHOD_(IResProviderMgr *, GetResProviderMgr)(THIS) OVERRIDE;

    STDMETHOD_(IRealWndHandler *, GetRealWndHander)(THIS) OVERRIDE;

    STDMETHOD_(void, SetRealWndHandler)(THIS_ IRealWndHandler *pRealHandler) OVERRIDE;

    STDMETHOD_(void, SetScriptFactory)(THIS_ IScriptFactory *pScriptModule) OVERRIDE;

    STDMETHOD_(void, InitXmlNamedID)
    (THIS_ const LPCWSTR *pNames, const int *nIds, int nCount) OVERRIDE;

    STDMETHOD_(IXmlDoc *, LoadXmlDocment)(THIS_ LPCTSTR strResId) OVERRIDE;
	STDMETHOD_(IXmlDoc *, LoadXmlDocmentA)(THIS_ LPCSTR strResId) OVERRIDE{
		SStringT str = S_CA2T(strResId,CP_UTF8);
		return LoadXmlDocment(str);
	}

    STDMETHOD_(IAnimation *, LoadAnimation)(THIS_ LPCTSTR strResId) OVERRIDE;
	STDMETHOD_(IAnimation *, LoadAnimationA)(THIS_ LPCSTR strResId) OVERRIDE{
		SStringT str = S_CA2T(strResId,CP_UTF8);
		return LoadAnimation(str);
	}
	STDMETHOD_(IBitmapS *,LoadImage)(THIS_ LPCTSTR strResId) OVERRIDE{
		SStringW wcsResId = S_CT2W(strResId);
		return SResProviderMgr::LoadImage2(wcsResId);
	}
	STDMETHOD_(IBitmapS *,LoadImageA)(THIS_ LPCSTR strResId) OVERRIDE{
		SStringT str = S_CA2T(strResId,CP_UTF8);
		return LoadImage(str);
	}

    STDMETHOD_(IValueAnimator *, LoadValueAnimator)(THIS_ LPCTSTR strResId) OVERRIDE;
	STDMETHOD_(IValueAnimator *, LoadValueAnimatorA)(THIS_ LPCSTR strResId) OVERRIDE{
		SStringT str = S_CA2T(strResId,CP_UTF8);
		return LoadValueAnimator(str);
	}

	STDMETHOD_(ITranslator *,LoadTranslator)(THIS_ LPCTSTR strResId) OVERRIDE;
	STDMETHOD_(ITranslator *,LoadTranslatorA)(THIS_ LPCSTR strResId) OVERRIDE{
		SStringT str = S_CA2T(strResId,CP_UTF8);
		return LoadTranslator(str);
	}

	STDMETHOD_(BOOL,InstallTranslator)(THIS_ ITranslator * trModule) OVERRIDE;
	STDMETHOD_(BOOL,UnnstallTranslator)(THIS_  REFGUID langId) OVERRIDE;

	STDMETHOD_(void,EnableNotifyCenter)(THIS_ BOOL bEnable,int interval DEF_VAL(20)) OVERRIDE;

	STDMETHOD_(void *,GetInnerSingleton)(THIS_ SingletonType nType) OVERRIDE;

	STDMETHOD_(IObject *,CreateObject)(THIS_ LPCWSTR pszName,SObjectType nType) SCONST OVERRIDE;

	STDMETHOD_(void,SetCreateObjectCallback)(THIS_ FunCreateObject cbCreateObj) OVERRIDE;

	STDMETHOD_(BOOL,RegisterObjFactory)(THIS_ const IObjectFactory *objFac,BOOL bReplace DEF_VAL(FALSE)) OVERRIDE;
	STDMETHOD_(BOOL,UnregisterObjFactory)(THIS_ const IObjectFactory *objFac) OVERRIDE;
	STDMETHOD_(void,SetDefaultFontInfo)(THIS_ LPCWSTR pszFontInfo) OVERRIDE;

	STDMETHOD_(BOOL,CreateTaskLoop)(THIS_ int nCount,Priority priority,BOOL bAutoStart DEF_VAL(TRUE)) OVERRIDE;
	STDMETHOD_(ITaskLoop *, GetTaskLoop)(THIS_ int iTaskLoop DEF_VAL(0)) OVERRIDE;
	STDMETHOD_(void, SetCreateTaskLoopCallback)(THIS_ FunCrateTaskLoop cbCreateTaskLoop) OVERRIDE;
	STDMETHOD_(HRESULT,CreateScriptModule)(THIS_ IScriptModule **ppScriptModule) OVERRIDE;

  public:
    /**
     * Init
     * @brief    从数组里初始化命名ID列表
     * @param    SNamedID::NAMEDVALUE *pValue --  数组
     * @param    int nCount --  数组长度
     * @param    BOOL bSorted -- 数组关键字从小到大已经有序
     *
     * Describe  SNamedID::NAMEDVALUE应该是由uiresbuilder自动生成的列表数据，不要手动修改
     */
    void InitXmlNamedID(const SNamedID::NAMEDVALUE *pNamedValue, int nCount, BOOL bSorted);

    /**
     * LoadXmlDocment
     * @brief    从资源中加载一个XML Document。
     * @param [out] SXmlDoc & xmlDoc --  输出的xml_document对象
     * @param    const SStringT & strResId --  XML文件在资源中的type:name
	 * @param IResProvider *pResProvider -- 指定的资源包
     * @return   BOOL true-加载成功, false-加载失败
     *
     * Describe
     */
    BOOL LoadXmlDocment(SXmlDoc &xmlDoc, const SStringT &strResId,IResProvider *pResProvider=NULL);


    void SetAppDir(const SStringT &strAppDir);

    SStringT GetAppDir() const;

    template <class T>
    BOOL RegisterWindowClass()
    {
        if (T::GetClassType() != Window)
            return FALSE;
        return TplRegisterFactory<T>();
    }

    template <class T>
    BOOL UnregisterWindowClass()
    {
        if (T::GetClassType() != Window)
            return FALSE;
        return TplUnregisterFactory<T>();
    }
    template <class T>
    BOOL RegisterSkinClass()
    {
        if (T::GetClassType() != Skin)
            return FALSE;
        return TplRegisterFactory<T>();
    }

    template <class T>
    BOOL UnregisterSkinClass()
    {
        if (T::GetClassType() != Skin)
            return FALSE;
        return TplUnregisterFactory<T>();
    }

    int Str2ID(const SStringW &str);

    SStringW tr(const SStringW &strSrc, const SStringW &strCtx) const;
    virtual IWindow *CreateWindowByName(LPCWSTR pszWndClass) const;
    virtual ISkinObj *CreateSkinByName(LPCWSTR pszSkinClass) const;
    virtual IInterpolator *CreateInterpolatorByName(LPCWSTR pszName) const;
    virtual IAnimation *CreateAnimationByName(LPCWSTR pszName) const;
    virtual IValueAnimator *CreateValueAnimatorByName(LPCWSTR pszName) const;

    virtual IAccProxy *CreateAccProxy(SWindow *pWnd) const;
    virtual IAccessible *CreateAccessible(SWindow *pWnd) const;

  protected:
    void _CreateSingletons(HINSTANCE hInst, LPCTSTR pszHostClassName, BOOL bImeApp);
    void _DestroySingletons();
    BOOL _LoadXmlDocment(LPCTSTR pszXmlName,
                         LPCTSTR pszType,
                         SXmlDoc &xmlDoc,
                         IResProvider *pResProvider = NULL);

    SAutoRefPtr<IRealWndHandler> m_pRealWndHandler;
    SAutoRefPtr<IScriptFactory> m_pScriptFactory;
    SAutoRefPtr<IRenderFactory> m_RenderFactory;
    SAutoRefPtr<ITranslatorMgr> m_translator;
    SAutoRefPtr<IToolTipFactory> m_tooltipFactory;
    SAutoRefPtr<IMsgLoopFactory> m_msgLoopFactory;
    SAutoRefPtr<ILogMgr> m_logManager; // log manager
    SAutoRefPtr<IAttrStorageFactory> m_pAttrStroageFactory;

    SNamedID m_namedID;

    SStringT m_strAppDir;
    HMODULE m_hInst;
    HWND m_hMainWnd;

    mutable SCriticalSection m_cs;
	typedef SMap<DWORD, SAutoRefPtr<IMessageLoop> > MsgLoopMap;
    MsgLoopMap m_msgLoopMap;

	FunCreateObject m_cbCreateObj;
	FunCrateTaskLoop m_cbCreateTaskLoop;
	SArray<SAutoRefPtr<ITaskLoop>> m_lstTaskLoop;//task loop list
    //一组单例指针
    void *m_pSingletons[SINGLETON_COUNT];
};

SNSEND