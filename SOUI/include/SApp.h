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

#ifndef __SAPP__H__
#define __SAPP__H__
#include <windows.h>
#include <core/SSingleton.h>
#include <interface/sapp-i.h>

#include <res.mgr/SResProviderMgr.h>
#include <res.mgr/SNamedValue.h>

#include <core/SMsgLoop.h>
#include <core/SObjectFactory.h>
#include <helper/obj-ref-impl.hpp>
#include <interface/SSkinobj-i.h>

#define GETRESPROVIDER    SNS::SApplication::getSingletonPtr()
#define GETRENDERFACTORY  SNS::SApplication::getSingleton().GetRenderFactory()
#define GETREALWNDHANDLER SNS::SApplication::getSingleton().GetRealWndHander()
#define GETTOOLTIPFACTORY SNS::SApplication::getSingleton().GetToolTipFactory()

#define LOADXML(p1, p2)   SNS::SApplication::getSingleton().LoadXmlDocment(p1, p2)
#define LOADIMAGE(p1,p2)  ((IResProviderMgr*)SNS::SApplication::getSingletonPtr())->LoadImage(p1,p2)
#define LOADIMAGE2(p1)    SNS::SApplication::getSingleton().LoadImage2(p1)
#define LOADICON(p1, p2)  SNS::SApplication::getSingleton().LoadIcon(p1, p2, p2)
#define LOADICON2(p1)     SNS::SApplication::getSingleton().LoadIcon2(p1)
#define TR(p1, p2)        SNS::SApplication::getSingleton().tr(p1, p2)
#define STR2ID(p1)        SNS::SApplication::getSingleton().Str2ID(p1)


#define CREATEINTERPOLATOR(x) SNS::SApplication::getSingleton().CreateInterpolatorByName(x)

SNSBEGIN

struct IAccProxy;

interface SOUI_EXP ISystemObjectRegister
{
    virtual void RegisterLayouts(SObjectFactoryMgr * objFactory) const PURE; 
	virtual void RegisterSkins(SObjectFactoryMgr * objFactory) const PURE; 
	virtual void RegisterWindows(SObjectFactoryMgr * objFactory) const PURE; 
	virtual void RegisterInterpolator(SObjectFactoryMgr * objFactory) const PURE; 
	virtual void RegisterAnimation(SObjectFactoryMgr * objFactory) const PURE; 
	virtual void RegisterValueAnimator(SObjectFactoryMgr * objFactory) const PURE;
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
      * @brief Constructor for SApplication
      * @param pRendFactory Pointer to the render factory
      * @param hInst Handle to the application instance
      * @param pszHostClassName Default window class name for SOUI windows
      * @param sysObjRegister System object register
      * @param bImeApp Boolean indicating if the application is an IME application
      */
     SApplication(IRenderFactory *pRendFactory,
                  HINSTANCE hInst,
                  LPCTSTR pszHostClassName = _T("SOUIHOST"),
                  const ISystemObjectRegister &sysObjRegister = SObjectDefaultRegister(),
                  BOOL bImeApp = FALSE);
 
     /**
      * @brief Destructor for SApplication
      */
     ~SApplication(void);
 
   public:
     /**
      * @brief Get the render factory
      * @return Pointer to the render factory
      */
     STDMETHOD_(IRenderFactory *, GetRenderFactory)(THIS) OVERRIDE;
 
     /**
      * @brief Set the render factory
      * @param renderFac Pointer to the new render factory
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL, SetRenderFactory)(THIS_ IRenderFactory * renderFac) OVERRIDE;
 
     /**
      * @brief Get the module handle
      * @return Module handle
      */
     STDMETHOD_(HMODULE, GetModule)(THIS) SCONST OVERRIDE;
 
     /**
      * @brief Load system named resources
      * @param pResProvider Resource provider
      * @return Number of resources loaded
      */
     STDMETHOD_(UINT, LoadSystemNamedResource)(THIS_ IResProvider *pResProvider) OVERRIDE;
 
     /**
      * @brief Get the translator manager
      * @return Pointer to the translator manager
      */
     STDMETHOD_(ITranslatorMgr *, GetTranslator)(THIS) OVERRIDE;
 
     /**
      * @brief Set the translator manager
      * @param pTrans Pointer to the new translator manager
      */
     STDMETHOD_(void, SetTranslator)(THIS_ ITranslatorMgr *pTrans) OVERRIDE;
 
     /**
      * @brief Get the tooltip factory
      * @return Pointer to the tooltip factory
      */
     STDMETHOD_(IToolTipFactory *, GetToolTipFactory)(THIS) OVERRIDE;
 
     /**
      * @brief Set the tooltip factory
      * @param pToolTipFac Pointer to the new tooltip factory
      */
     STDMETHOD_(void, SetToolTipFactory)(THIS_ IToolTipFactory *pToolTipFac) OVERRIDE;
 
     /**
      * @brief Set the message loop factory
      * @param pMsgLoopFac Pointer to the new message loop factory
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL, SetMsgLoopFactory)(THIS_ IMsgLoopFactory *pMsgLoopFac) OVERRIDE;
 
     /**
      * @brief Get the message loop factory
      * @return Pointer to the message loop factory
      */
     STDMETHOD_(IMsgLoopFactory *, GetMsgLoopFactory)(THIS) OVERRIDE;
 
     /**
      * @brief Set the log manager
      * @param pLogMgr Pointer to the new log manager
      */
     STDMETHOD_(void, SetLogManager)(THIS_ ILogMgr *pLogMgr) OVERRIDE;
 
     /**
      * @brief Get the log manager
      * @return Pointer to the log manager
      */
     STDMETHOD_(ILogMgr *, GetLogManager)(THIS) OVERRIDE;
 
     /**
      * @brief Set the attribute storage factory
      * @param pAttrStorageFactory Pointer to the new attribute storage factory
      */
     STDMETHOD_(void, SetAttrStorageFactory)
     (THIS_ IAttrStorageFactory *pAttrStorageFactory) OVERRIDE;
 
     /**
      * @brief Get the attribute storage factory
      * @return Pointer to the attribute storage factory
      */
     STDMETHOD_(IAttrStorageFactory *, GetAttrStorageFactory)(THIS) OVERRIDE;
 
     /**
      * @brief Run the application message loop
      * @param hMainWnd Handle to the main window
      * @return Exit code of the application
      */
     STDMETHOD_(int, Run)(THIS_ HWND hMainWnd) OVERRIDE;
 
     /**
      * @brief Quit the application
      * @param nCode Exit code
      */
     STDMETHOD_(void,Quit)(THIS_ int nCode) OVERRIDE;
 
     /**
      * @brief Get the main window handle
      * @return Handle to the main window
      */
     STDMETHOD_(HWND, GetMainWnd)(THIS) OVERRIDE;
 
     /**
      * @brief Add a message loop
      * @param pMsgLoop Pointer to the message loop to add
      * @param bReplace Boolean indicating if the existing message loop should be replaced
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL, AddMsgLoop)(THIS_ IMessageLoop *pMsgLoop,BOOL bReplace DEF_VAL(FALSE)) OVERRIDE;
 
     /**
      * @brief Remove the current message loop
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL, RemoveMsgLoop)(THIS) OVERRIDE;
 
     /**
      * @brief Get the message loop for a specific thread
      * @param tid Thread ID
      * @return Pointer to the message loop
      */
     STDMETHOD_(IMessageLoop *, GetMsgLoop)
     (THIS_ tid_t tid = ::GetCurrentThreadId()) SCONST OVERRIDE;
 
     /**
      * @brief Get the resource provider manager
      * @return Pointer to the resource provider manager
      */
     STDMETHOD_(IResProviderMgr *, GetResProviderMgr)(THIS) OVERRIDE;
 
     /**
      * @brief Get the real window handler
      * @return Pointer to the real window handler
      */
     STDMETHOD_(IRealWndHandler *, GetRealWndHander)(THIS) OVERRIDE;
 
     /**
      * @brief Set the real window handler
      * @param pRealHandler Pointer to the new real window handler
      */
     STDMETHOD_(void, SetRealWndHandler)(THIS_ IRealWndHandler *pRealHandler) OVERRIDE;
 
     /**
      * @brief Set the script factory
      * @param pScriptModule Pointer to the new script factory
      */
     STDMETHOD_(void, SetScriptFactory)(THIS_ IScriptFactory *pScriptModule) OVERRIDE;
 
     /**
      * @brief Initialize XML named IDs from arrays
      * @param pNames Array of names
      * @param nIds Array of IDs
      * @param nCount Number of elements in the arrays
      */
     STDMETHOD_(void, InitXmlNamedID)
     (THIS_ const LPCWSTR *pNames, const int *nIds, int nCount) OVERRIDE;
 
     /**
      * @brief Load an XML document from a resource
      * @param strResId Resource ID of the XML document
      * @return Pointer to the loaded XML document
      */
     STDMETHOD_(IXmlDoc *, LoadXmlDocment)(THIS_ LPCTSTR strResId) OVERRIDE;
 
     /**
      * @brief Load an XML document from a UTF-8 encoded resource
      * @param strResId Resource ID of the XML document
      * @return Pointer to the loaded XML document
      */
     STDMETHOD_(IXmlDoc *, LoadXmlDocmentU8)(THIS_ LPCSTR strResId) OVERRIDE
     {
         SStringT str = S_CA2T(strResId, CP_UTF8);
         return LoadXmlDocment(str);
     }
 
     /**
      * @brief Load an animation from a resource
      * @param strResId Resource ID of the animation
      * @return Pointer to the loaded animation
      */
     STDMETHOD_(IAnimation *, LoadAnimation)(THIS_ LPCTSTR strResId) OVERRIDE;
 
     /**
      * @brief Load an animation from a UTF-8 encoded resource
      * @param strResId Resource ID of the animation
      * @return Pointer to the loaded animation
      */
     STDMETHOD_(IAnimation *, LoadAnimationU8)(THIS_ LPCSTR strResId) OVERRIDE
     {
         SStringT str = S_CA2T(strResId, CP_UTF8);
         return LoadAnimation(str);
     }
 
     /**
      * @brief Load an image from a resource
      * @param strResId Resource ID of the image
      * @return Pointer to the loaded image
      */
     STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR strResId) OVERRIDE
     {
         SStringW wcsResId = S_CT2W(strResId);
         return SResProviderMgr::LoadImage2(wcsResId);
     }
 
     /**
      * @brief Load an image from a UTF-8 encoded resource
      * @param strResId Resource ID of the image
      * @return Pointer to the loaded image
      */
     STDMETHOD_(IBitmapS *, LoadImageU8)(THIS_ LPCSTR strResId) OVERRIDE
     {
         SStringT str = S_CA2T(strResId, CP_UTF8);
         return LoadImage(str);
     }
 
     /**
      * @brief Load a value animator from a resource
      * @param strResId Resource ID of the value animator
      * @return Pointer to the loaded value animator
      */
     STDMETHOD_(IValueAnimator *, LoadValueAnimator)(THIS_ LPCTSTR strResId) OVERRIDE;
 
     /**
      * @brief Load a value animator from a UTF-8 encoded resource
      * @param strResId Resource ID of the value animator
      * @return Pointer to the loaded value animator
      */
     STDMETHOD_(IValueAnimator *, LoadValueAnimatorU8)(THIS_ LPCSTR strResId) OVERRIDE
     {
         SStringT str = S_CA2T(strResId, CP_UTF8);
         return LoadValueAnimator(str);
     }
 
     /**
      * @brief Load a translator from a resource
      * @param strResId Resource ID of the translator
      * @return Pointer to the loaded translator
      */
     STDMETHOD_(ITranslator *,LoadTranslator)(THIS_ LPCTSTR strResId) OVERRIDE;
 
     /**
      * @brief Load a translator from a UTF-8 encoded resource
      * @param strResId Resource ID of the translator
      * @return Pointer to the loaded translator
      */
     STDMETHOD_(ITranslator *, LoadTranslatorU8)(THIS_ LPCSTR strResId) OVERRIDE
     {
         SStringT str = S_CA2T(strResId, CP_UTF8);
         return LoadTranslator(str);
     }

     /**
      * @brief Install a translator
      * @param trModule Pointer to the translator module
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL,InstallTranslator)(THIS_ ITranslator * trModule) OVERRIDE;
 
     /**
      * @brief Uninstall a translator by language ID
      * @param langId Language ID
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL,UnnstallTranslator)(THIS_  REFGUID langId) OVERRIDE;
 
     /**
      * @brief Enable or disable the notification center
      * @param bEnable Boolean indicating if the notification center should be enabled
      * @param interval Interval for notifications
      */
     STDMETHOD_(void,EnableNotifyCenter)(THIS_ BOOL bEnable,int interval DEF_VAL(20)) OVERRIDE;
 
     /**
      * @brief Get an inner singleton
      * @param nType Type of the singleton
      * @return Pointer to the singleton
      */
     STDMETHOD_(void *,GetInnerSingleton)(THIS_ SingletonType nType) OVERRIDE;
 
     /**
      * @brief Create an object by name and type
      * @param pszName Name of the object
      * @param nType Type of the object
      * @return Pointer to the created object
      */
     STDMETHOD_(IObject *,CreateObject)(THIS_ LPCWSTR pszName,SObjectType nType) SCONST OVERRIDE;
 
     /**
      * @brief Set the callback for creating objects
      * @param cbCreateObj Callback function
      */
     STDMETHOD_(void,SetCreateObjectCallback)(THIS_ FunCreateObject cbCreateObj) OVERRIDE;
 
     /**
      * @brief Register an object factory
      * @param objFac Pointer to the object factory
      * @param bReplace Boolean indicating if the existing factory should be replaced
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL,RegisterObjFactory)(THIS_ const IObjectFactory *objFac,BOOL bReplace DEF_VAL(FALSE)) OVERRIDE;
 
     /**
      * @brief Unregister an object factory
      * @param objFac Pointer to the object factory
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL,UnregisterObjFactory)(THIS_ const IObjectFactory *objFac) OVERRIDE;
 
     /**
      * @brief Set the default font information
      * @param pszFontInfo Font information string
      */
     STDMETHOD_(void,SetDefaultFontInfo)(THIS_ LPCWSTR pszFontInfo) OVERRIDE;
 
     /**
      * @brief Create task loops
      * @param nCount Number of task loops to create
      * @param priority Priority of the task loops
      * @param bAutoStart Boolean indicating if the task loops should start automatically
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL,CreateTaskLoop)(THIS_ int nCount,Priority priority,BOOL bAutoStart DEF_VAL(TRUE)) OVERRIDE;
 
     /**
      * @brief Get a task loop by index
      * @param iTaskLoop Index of the task loop
      * @return Pointer to the task loop
      */
     STDMETHOD_(ITaskLoop *, GetTaskLoop)(THIS_ int iTaskLoop DEF_VAL(0)) OVERRIDE;
 
     /**
      * @brief Set the callback for creating task loops
      * @param cbCreateTaskLoop Callback function
      */
     STDMETHOD_(void, SetCreateTaskLoopCallback)(THIS_ FunCrateTaskLoop cbCreateTaskLoop) OVERRIDE;
 
     /**
      * @brief Create a script module
      * @param ppScriptModule Pointer to the created script module
      * @return HRESULT indicating success or failure
      */
     STDMETHOD_(HRESULT,CreateScriptModule)(THIS_ IScriptModule **ppScriptModule) OVERRIDE;
 
     /**
      * @brief Set the resource ID for the edit context menu template
      * @param resId Resource ID of the template
      * @param pResProvider Resource provider
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL,SetEditCtxMenuTemplateResId)(THIS_ LPCTSTR resId,IResProvider *pResProvider DEF_VAL(NULL)) OVERRIDE;
 
     /**
      * @brief Set the resource ID for the message box template
      * @param resId Resource ID of the template
      * @param pResProvider Resource provider
      * @return TRUE if successful, FALSE otherwise
      */
     STDMETHOD_(BOOL,SetMessageBoxTemplateResId)(THIS_ LPCTSTR resId,IResProvider *pResProvider DEF_VAL(NULL)) OVERRIDE;
 
   public:
    /**
     * @brief Initialize XML named IDs from an array of named values
     * @param pNamedValue Array of named values
     * @param nCount Number of elements in the array
     * @param bSorted Boolean indicating if the array is sorted
     */
    void InitXmlNamedID(const SNamedID::NAMEDVALUE *pNamedValue, int nCount, BOOL bSorted);

    /**
     * @brief Load an XML document from a resource
     * @param xmlDoc Output XML document object
     * @param strResId Resource ID of the XML document
     * @param pResProvider Resource provider
     * @return TRUE if successful, FALSE otherwise
     */
    BOOL LoadXmlDocment(SXmlDoc &xmlDoc, const SStringT &strResId, IResProvider *pResProvider = NULL);

    /**
     * @brief Set the application directory
     * @param strAppDir Application directory path
     */
    void SetAppDir(const SStringT &strAppDir);

    /**
     * @brief Get the application directory
     * @return Application directory path
     */
    SStringT GetAppDir() const;

    /**
     * @brief Register a window class
     * @tparam T Window class type
     * @return TRUE if successful, FALSE otherwise
     */
    template <class T>
    BOOL RegisterWindowClass()
    {
        if (T::GetClassType() != Window)
            return FALSE;
        return TplRegisterFactory<T>();
    }

    /**
     * @brief Unregister a window class
     * @tparam T Window class type
     * @return TRUE if successful, FALSE otherwise
     */
    template <class T>
    BOOL UnregisterWindowClass()
    {
        if (T::GetClassType() != Window)
            return FALSE;
        return TplUnregisterFactory<T>();
    }

    /**
     * @brief Register a skin class
     * @tparam T Skin class type
     * @return TRUE if successful, FALSE otherwise
     */
    template <class T>
    BOOL RegisterSkinClass()
    {
        if (T::GetClassType() != Skin)
            return FALSE;
        return TplRegisterFactory<T>();
    }

    /**
     * @brief Unregister a skin class
     * @tparam T Skin class type
     * @return TRUE if successful, FALSE otherwise
     */
    template <class T>
    BOOL UnregisterSkinClass()
    {
        if (T::GetClassType() != Skin)
            return FALSE;
        return TplUnregisterFactory<T>();
    }

    /**
     * @brief Convert a string to an ID
     * @param str Source string
     * @return ID corresponding to the string
     */
    int Str2ID(const SStringW &str);

    /**
     * @brief Get the message box template XML node
     * @return XML node for the message box template
     */
    SXmlNode GetMessageBoxTemplate() const;

    /**
     * @brief Get the edit context menu template XML node
     * @return XML node for the edit context menu template
     */
    SXmlNode GetEditCtxMenuTemplate() const;

    /**
     * @brief Translate a string with context
     * @param strSrc Source string
     * @param strCtx Context string
     * @return Translated string
     */
    SStringW tr(const SStringW &strSrc, const SStringW &strCtx) const;

    /**
     * @brief Create a window by name
     * @param pszWndClass Name of the window class
     * @return Pointer to the created window
     */
    virtual IWindow *CreateWindowByName(LPCWSTR pszWndClass) const;

    /**
     * @brief Create a skin by name
     * @param pszSkinClass Name of the skin class
     * @return Pointer to the created skin
     */
    virtual ISkinObj *CreateSkinByName(LPCWSTR pszSkinClass) const;

    /**
     * @brief Create an interpolator by name
     * @param pszName Name of the interpolator
     * @return Pointer to the created interpolator
     */
    virtual IInterpolator *CreateInterpolatorByName(LPCWSTR pszName) const;

    /**
     * @brief Create an animation by name
     * @param pszName Name of the animation
     * @return Pointer to the created animation
     */
    virtual IAnimation *CreateAnimationByName(LPCWSTR pszName) const;

    /**
     * @brief Create a value animator by name
     * @param pszName Name of the value animator
     * @return Pointer to the created value animator
     */
    virtual IValueAnimator *CreateValueAnimatorByName(LPCWSTR pszName) const;

#ifdef _WIN32
    /**
     * @brief Create an accessibility proxy for a window
     * @param pWnd Pointer to the window
     * @return Pointer to the created accessibility proxy
     */
    virtual IAccProxy *CreateAccProxy(IWindow *pWnd) const;

    /**
     * @brief Create an accessible object for a window
     * @param pWnd Pointer to the window
     * @return Pointer to the created accessible object
     */
    virtual IAccessible *CreateAccessible(IWindow *pWnd) const;
#endif
  protected:
    void _CreateSingletons();
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
    typedef SMap<tid_t, SAutoRefPtr<IMessageLoop>> MsgLoopMap;
    MsgLoopMap m_msgLoopMap;

    FunCreateObject m_cbCreateObj;
    FunCrateTaskLoop m_cbCreateTaskLoop;
    SArray<SAutoRefPtr<ITaskLoop>> m_lstTaskLoop; // task loop list
    // 一组单例指针
    void *m_pSingletons[SINGLETON_COUNT];
    SXmlDoc m_xmlEditCtxMenuTemplate;
    SXmlDoc m_xmlMessageBoxTemplate;
};

SNSEND
#endif // __SAPP__H__