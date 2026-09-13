#ifndef __SAPP_I__H__
#define __SAPP_I__H__
#include <interface/obj-ref-i.h>
#include <interface/SResProvider-i.h>
#include <interface/STranslator-i.h>
#include <interface/stooltip-i.h>
#include <interface/slog-i.h>
#include <interface/SAttrStorage-i.h>
#include <interface/SMsgLoop-i.h>
#include <interface/shostwnd-i.h>
#include <interface/SResProviderMgr-i.h>
#include <interface/SRealWndHandler-i.h>
#include <interface/SRender-i.h>
#include <interface/SScriptModule-i.h>
#include <interface/SAnimation-i.h>
#include <interface/SValueAnimator-i.h>
#include <interface/SNotifyCenter-i.h>
#include <interface/SObjFactory-i.h>
#include <interface/SHostPresenter-i.h>
#include <interface/STaskLoop-i.h>
#include <interface/SAttrAlias-i.h>

SNSBEGIN

typedef enum _SingletonType
{
    SINGLETON_UIDEF = 0,
    SINGLETON_SWNDMGR,
    SINGLETON_TIMERGENERATOR,
    SINGLETON_RICHEDITMENUDEF,
    SINGLETON_HOSTMGR,
    SINGLETON_NOTIFYCENTER,

    SINGLETON_COUNT,
} SingletonType;

typedef struct IApplication IApplication;
typedef IObject *(*FunCreateObject)(const IApplication *app, LPCWSTR pszName, SObjectType nType);
typedef ITaskLoop *(*FunCrateTaskLoop)();

#undef INTERFACE
#define INTERFACE IApplication
DECLARE_INTERFACE_(IApplication, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Get the current app's hModule
     * @return HMODULE -- the current app's hModule
     */
    STDMETHOD_(HMODULE, GetModule)(CTHIS) SCONST PURE;

    /**
     * @brief Load system resources from the specified ResProvider
     * @param pResProvider IResProvider* -- the resource package providing system resources
     * @return UINT -- 0: success, otherwise: failure
     */
    STDMETHOD_(UINT, LoadSystemNamedResource)(THIS_ IResProvider * pResProvider) PURE;

    /**
     * @brief Get the translation interface
     * @return ITranslatorMgr* -- the translation interface
     */
    STDMETHOD_(ITranslatorMgr *, GetTranslator)(THIS) PURE;

    /**
     * @brief Set the translation interface
     * @param pTrans ITranslatorMgr* -- the translation interface
     * @return void
     */
    STDMETHOD_(void, SetTranslator)(THIS_ ITranslatorMgr * pTrans) PURE;

    /**
     * @brief Get the ToolTip factory
     * @return IToolTipFactory* -- the ToolTip factory
     */
    STDMETHOD_(IToolTipFactory *, GetToolTipFactory)(THIS) PURE;

    /**
     * @brief Set the ToolTip factory
     * @param pToolTipFac IToolTipFactory* -- the ToolTip factory
     * @return void
     */
    STDMETHOD_(void, SetToolTipFactory)(THIS_ IToolTipFactory * pToolTipFac) PURE;

    /**
     * @brief Set the MsgLoop factory
     * @param pMsgLoopFac IMsgLoopFactory* -- the MsgLoop factory
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, SetMsgLoopFactory)(THIS_ IMsgLoopFactory * pMsgLoopFac) PURE;

    /**
     * @brief Get the current MsgLoop factory
     * @return IMsgLoopFactory* -- the MsgLoop factory
     */
    STDMETHOD_(IMsgLoopFactory *, GetMsgLoopFactory)(THIS) PURE;

    /**
     * @brief Set the log output module
     * @param pLogMgr ILogMgr* -- the log output module
     * @return void
     */
    STDMETHOD_(void, SetLogManager)(THIS_ ILogMgr * pLogMgr) PURE;

    /**
     * @brief Get the log output module
     * @return ILogMgr* -- the log output module
     */
    STDMETHOD_(ILogMgr *, GetLogManager)(THIS) PURE;

    /**
     * @brief Set the attribute persistence factory
     * @param pAttrStorageFactory IAttrStorageFactory* -- the attribute persistence factory
     * @return void
     */
    STDMETHOD_(void, SetAttrStorageFactory)(THIS_ IAttrStorageFactory * pAttrStorageFactory) PURE;

    /**
     * @brief Get the attribute persistence factory
     * @return IAttrStorageFactory* -- the attribute persistence factory
     */
    STDMETHOD_(IAttrStorageFactory *, GetAttrStorageFactory)(THIS) PURE;

    /**
     * @brief Enter the message loop to run the program
     * @param hMainWnd HWND -- main window handle
     * @return int -- the program's exit return value
     */
    STDMETHOD_(int, Run)(THIS_ HWND hMainWnd) PURE;

    /**
     * @brief Exit the message loop
     * @param nCode int -- thread exit code
     * @return void
     */
    STDMETHOD_(void, Quit)(THIS_ int nCode) PURE;

    /**
     * @brief Get the program's main window
     * @return HWND -- the window set in the Run method
     */
    STDMETHOD_(HWND, GetMainWnd)(THIS) PURE;

    /**
     * @brief Save the current UI thread's msgLoop into SApp
     * @param pMsgLoop IMessageLoop* -- msgLoop
     * @param bReplace BOOL -- replace flag
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, AddMsgLoop)(THIS_ IMessageLoop * pMsgLoop, BOOL bReplace DEF_VAL(FALSE)) PURE;

    /**
     * @brief Remove the current thread's msgLoop from SApp
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, RemoveMsgLoop)(THIS) PURE;

    /**
     * @brief Get the specified UI thread's msgLoop
     * @param  tid -- thread ID
     * @return IMessageLoop* -- the msgLoop of the specified thread
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(CTHIS_ tid_t dwThreadID DEF_VAL(GetCurrentThreadId())) SCONST PURE;

    /**
     * @brief Get the IResProviderMgr interface
     * @return IResProviderMgr* -- the IResProviderMgr interface
     */
    STDMETHOD_(IResProviderMgr *, GetResProviderMgr)(THIS) PURE;

    /**
     * @brief Get the RealWndHandler
     * @return IRealWndHandler* -- the RealWndHandler
     */
    STDMETHOD_(IRealWndHandler *, GetRealWndHander)(THIS) PURE;

    /**
     * @brief Set the RealWnd handling interface
     * @param pRealHandler IRealWndHandler* -- the RealWnd handling interface
     * @return void
     */
    STDMETHOD_(void, SetRealWndHandler)(THIS_ IRealWndHandler * pRealHandler) PURE;

    /**
     * @brief Get the current render module
     * @return IRenderFactory* -- render module pointer
     */
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(THIS) PURE;

    /**
     * @brief Set the render module
     * @param renderFac IRenderFactory* -- render module pointer
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, SetRenderFactory)(THIS_ IRenderFactory * renderFac) PURE;

    /**
     * @brief Set the script module factory used in SOUI
     * @param pScriptModule IScriptFactory* -- the script module factory
     * @return void
     */
    STDMETHOD_(void, SetScriptFactory)(THIS_ IScriptFactory * pScriptModule) PURE;

    /**
     * @brief Initialize the name-id mapping table for XML resources
     * @param pNames const LPCWSTR* -- control name table
     * @param nIds const int* -- control ID table
     * @param nCount int -- control count
     * @return void
     */
    STDMETHOD_(void, InitXmlNamedID)(THIS_ const LPCWSTR *pNames, const int *nIds, int nCount) PURE;

    /**
     * @brief Load XML from a resource ID
     * @param strResId LPCTSTR -- resource ID, in type:name format
     * @return IXmlDoc* -- XML Doc
     */
    STDMETHOD_(IXmlDoc *, LoadXmlDocment)(THIS_ LPCTSTR strResId) PURE;
    STDMETHOD_(IXmlDoc *, LoadXmlDocmentU8)(THIS_ LPCSTR strResId) PURE;

    /**
     * @brief Load an animation resource from resources
     * @param strResId LPCTSTR -- animation resource ID
     * @return IAnimation* -- animation object
     */
    STDMETHOD_(IAnimation *, LoadAnimation)(THIS_ LPCTSTR strResId) PURE;
    STDMETHOD_(IAnimation *, LoadAnimationU8)(THIS_ LPCSTR strResId) PURE;

    /**
     * @brief Load a value animator resource from resources
     * @param strResId LPCTSTR -- animation resource ID
     * @return IValueAnimator* -- value animator object
     */
    STDMETHOD_(IValueAnimator *, LoadValueAnimator)(THIS_ LPCTSTR strResId) PURE;
    STDMETHOD_(IValueAnimator *, LoadValueAnimatorU8)(THIS_ LPCSTR strResId) PURE;

    /**
     * @brief Load an image from resources
     * @param strResId LPCTSTR -- image resource ID
     * @return IBitmapS* -- image object
     */
    STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR strResId) PURE;
    STDMETHOD_(IBitmapS *, LoadImageU8)(THIS_ LPCSTR strResId) PURE;

    /**
     * @brief Load a translation package from resources
     * @param strResId LPCTSTR -- translation package resource ID
     * @return ITranslator* -- translation package object
     */
    STDMETHOD_(ITranslator *, LoadTranslator)(THIS_ LPCTSTR strResId) PURE;
    STDMETHOD_(ITranslator *, LoadTranslatorU8)(THIS_ LPCSTR strResId) PURE;

    /**
     * @brief Use the translation package
     * @param trModule ITranslator* -- the translation package
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, InstallTranslator)(THIS_ ITranslator * trModule) PURE;

    /**
     * @brief Unload the translation package
     * @param langId REFGUID -- translation package ID
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, UnnstallTranslator)(THIS_ REFGUID langId) PURE;

    /**
     * @brief Enable NotifyCenter
     * @param bEnable BOOL -- whether to enable
     * @param interval int -- event processing interval
     * @return void
     */
    STDMETHOD_(void, EnableNotifyCenter)(THIS_ BOOL bEnable, int interval DEF_VAL(20)) PURE;

    /**
     * @brief Get several singleton objects defined internally in SApp
     * @param type SingletonType -- internal singleton type
     * @return void* -- singleton type pointer
     */
    STDMETHOD_(void *, GetInnerSingleton)(THIS_ SingletonType type) PURE;

    /**
     * @brief Create an object
     * @param pszName LPCWSTR -- the type's name in XML
     * @param nType SObjectType -- type ID
     * @return IObject* -- the created type
     */
    STDMETHOD_(IObject *, CreateObject)(CTHIS_ LPCWSTR pszName, SObjectType nType) SCONST PURE;

    /**
     * @brief Set the callback for CreateObject
     * @param cbCreateObj FunCreateObject -- the callback function for CreateObject
     * @return void
     */
    STDMETHOD_(void, SetCreateObjectCallback)(THIS_ FunCreateObject cbCreateObj) PURE;

    /**
     * @brief Register an extension object
     * @param objFac const IObjectFactory* -- object factory
     * @param bReplace BOOL -- flag to replace an existing factory
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, RegisterObjFactory)(THIS_ const IObjectFactory *objFac, BOOL bReplace DEF_VAL(FALSE)) PURE;

    /**
     * @brief Unregister an extension object
     * @param objFac const IObjectFactory* -- object factory
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, UnregisterObjFactory)(THIS_ const IObjectFactory *objFac) PURE;

    /**
     * @brief Set the default font
     * @param pszFontInfo LPCWSTR -- font description, see fontpool
     * @return void
     */
    STDMETHOD_(void, SetDefaultFontInfo)(THIS_ LPCWSTR pszFontInfo) PURE;

    /**
     * @brief Create TaskLoop objects
     * @param nCount int -- number of taskloop objects
     * @param priority Priority -- thread priority of taskloop
     * @param bAutoStart BOOL -- auto-start flag
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, CreateTaskLoop)(THIS_ int nCount, Priority priority, BOOL bAutoStart DEF_VAL(TRUE)) PURE;

    /**
     * @brief Get a TaskLoop object
     * @param iTaskLoop int -- taskloop index
     * @return ITaskLoop* -- taskloop interface, returns NULL on failure
     */
    STDMETHOD_(ITaskLoop *, GetTaskLoop)(THIS_ int iTaskLoop DEF_VAL(0)) PURE;

    /**
     * @brief Set the callback interface for creating TaskLoop objects
     * @param cbCreateTaskLoop FunCrateTaskLoop -- the callback interface for creating TaskLoop objects
     * @return void
     */
    STDMETHOD_(void, SetCreateTaskLoopCallback)(THIS_ FunCrateTaskLoop cbCreateTaskLoop) PURE;

    /**
     * @brief Create a script module object
     * @param ppScriptModule IScriptModule** -- script module object
     * @return HRESULT -- S_OK: created successfully
     */
    STDMETHOD_(HRESULT, CreateScriptModule)(THIS_ IScriptModule * *ppScriptModule) PURE;

    /**
     * @brief Set the resource ID of the edit context menu template
     * @param resId LPCTSTR -- resource ID
     * @param pResProvider IResProvider* -- resource provider
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, SetEditCtxMenuTemplateResId)(THIS_ LPCTSTR resId, IResProvider * pResProvider DEF_VAL(NULL)) PURE;

    /**
     * @brief Set the resource ID of the message box template
     * @param resId LPCTSTR -- resource ID
     * @param pResProvider IResProvider* -- resource provider
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, SetMessageBoxTemplateResId)(THIS_ LPCTSTR resId, IResProvider * pResProvider DEF_VAL(NULL)) PURE;

    /**
     * @brief Set attribute alias retrieval interface
     * @param pAttrAlias IAttrAlias* -- attribute alias interface
     * @return void
     */
    STDMETHOD_(void, SetAttrAlias)(THIS_ IAttrAlias * pAttrAlias) PURE;

    /**
     * @brief Get the attribute alias interface
     * @return const IAttrAlias* -- the attribute alias interface
     */
    STDMETHOD_(const IAttrAlias *, GetAttrAlias)(CTHIS) SCONST PURE;

    /**
     * @brief Get the base class name of an object
     * @param pszClassName LPCWSTR -- object class name
     * @param objType int -- object type
     * @param pszBaseClassName[MAX_OBJNAME] wchar_t -- base class name
     * @return BOOL -- TRUE: success
     */
    STDMETHOD_(BOOL, GetBaseClassName)(CTHIS_ LPCWSTR pszClassName, int objType, wchar_t pszBaseClassName[MAX_OBJNAME]) SCONST PURE;
};

SNSEND
#endif /**< __SAPP_I__H__ */
