#pragma once
#include <interface/obj-ref-i.h>
#include <interface/SResProvider-i.h>
#include <interface/STranslator-i.h>
#include <interface/stooltip-i.h>
#include <interface/slog-i.h>
#include <interface/SAttrStorage-i.h>
#include <interface/smsgloop-i.h>
#include <interface/shostwnd-i.h>
#include <interface/SResProviderMgr-i.h>
#include <interface/SRealWndHandler-i.h>
#include <interface/SRender-i.h>
#include <interface/SScriptModule-i.h>
#include <interface/SAnimation-i.h>
#include <interface/SValueAnimator-i.h>
SNSBEGIN

#undef INTERFACE
#define INTERFACE IApplication
DECLARE_INTERFACE_(IApplication, IObjRef)
{
		//!添加引用
	/*!
	*/
	STDMETHOD_(long,AddRef) (THIS) PURE;

	//!释放引用
	/*!
	*/
	STDMETHOD_(long,Release) (THIS) PURE;

	//!释放对象
	/*!
	*/
	STDMETHOD_(void,OnFinalRelease) (THIS) PURE;

	//////////////////////////////////////////////////////////////////////////
    STDMETHOD_(HMODULE, GetModule)(THIS) SCONST PURE;
    STDMETHOD_(UINT, LoadSystemNamedResource)(THIS_ IResProvider * pResProvider) PURE;
    STDMETHOD_(ITranslatorMgr *, GetTranslator)(THIS) PURE;
    STDMETHOD_(void, SetTranslator)(THIS_ ITranslatorMgr * pTrans) PURE;
    STDMETHOD_(IToolTipFactory *, GetToolTipFactory)(THIS) PURE;
    STDMETHOD_(void, SetToolTipFactory)(THIS_ IToolTipFactory * pToolTipFac) PURE;

    STDMETHOD_(BOOL, SetMsgLoopFactory)(THIS_ IMsgLoopFactory * pMsgLoopFac) PURE;
    STDMETHOD_(IMsgLoopFactory *, GetMsgLoopFactory)(THIS) PURE;

    STDMETHOD_(void, SetLogManager)(THIS_ ILog4zManager * pLogMgr) PURE;
    STDMETHOD_(ILog4zManager *, GetLogManager)(THIS) PURE;

    STDMETHOD_(void, SetAttrStorageFactory)(THIS_ IAttrStorageFactory * pAttrStorageFactory) PURE;
    STDMETHOD_(IAttrStorageFactory *, GetAttrStorageFactory)(THIS) PURE;

    STDMETHOD_(int, Run)(THIS_ HWND hMainWnd) PURE;
    STDMETHOD_(HWND, GetMainWnd)(THIS) PURE;

    STDMETHOD_(BOOL, AddMsgLoop)(THIS_ IMessageLoop * pMsgLoop) PURE;
    STDMETHOD_(BOOL, RemoveMsgLoop)(THIS) PURE;

    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS_ DWORD dwThreadID) SCONST PURE;

    STDMETHOD_(IResProviderMgr *, GetResProviderMgr)(THIS) PURE;

    /**
     * GetRealWndHander
     * @brief    获得RealWndHander
     * @return   IRealWndHandler * -- RealWndHander
     * Describe
     */
    STDMETHOD_(IRealWndHandler *, GetRealWndHander)(THIS) PURE;

    /**
     * SetRealWndHandler
     * @brief    设置RealWnd处理接口
     * @param    IRealWndHandler * pRealHandler --  RealWnd处理接口
     * @return   void
     * Describe
     */
    STDMETHOD_(void, SetRealWndHandler)(THIS_ IRealWndHandler * pRealHandler) PURE;

    /**
     * GetRenderFactory
     * @brief    获得当前的渲染模块
     * @return   IRenderFactory * 渲染模块指针
     *
     * Describe
     */
    STDMETHOD_(IRenderFactory *, GetRenderFactory)(THIS) PURE;

    /**
     * SetScriptModule
     * @brief    设置SOUI中使用的脚本模块类厂
     * @param    IScriptFactory *pScriptModule --  脚本模块类厂
     * @return   void
     *
     * Describe
     */
    STDMETHOD_(void, SetScriptFactory)(THIS_ IScriptFactory * pScriptModule) PURE;

    /**
     * @brief 初始化XML资源的name-id映射表
     * @param pNames const LPCWSTR *--控件名表
     * @param nIds const int *--控件ID表
     * @param nCount int--控件数据
     * @return 
     * @remark 数据表由residbuilder工具自动生成
    */
    STDMETHOD_(void, InitXmlNamedID)(THIS_ const LPCWSTR *pNames, const int *nIds, int nCount) PURE;

    /**
     * @brief 从资源ID加载XML
     * @param strResId LPCTSTR--资源ID，为type:name格式
     * @return IXmlDoc *--XML Doc
     * @remark 使用完成后调用Rlease释放
    */
    STDMETHOD_(IXmlDoc *, LoadXmlDocment)(THIS_ LPCTSTR strResId) PURE;

    /**
     * @brief 从资源加载动画资源
     * @param strResId LPCTSTR--动画资源ID
     * @return IAnimation* 动画对象
     * @remark 使用完成后调用Rlease释放
     */
    STDMETHOD_(IAnimation *, LoadAnimation)(THIS_ LPCTSTR strResId) PURE;

    /**
     * @brief 从资源加载数值动画资源
     * @return IValueAnimator* 数值动画对象
     * @remark 使用完成后调用Rlease释放
     */
    STDMETHOD_(IValueAnimator *, LoadValueAnimator)(THIS_ LPCTSTR strResId) PURE;
};

SNSEND