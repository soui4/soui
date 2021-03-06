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
     * @brief 获取当前app的hModule
     * @return HMODULE--当前app的hModule
     */
    STDMETHOD_(HMODULE, GetModule)(THIS) SCONST PURE;

    /**
     * @brief 从指定的ResProvider里加载系统资源
     * @param pResProvider 提供系统资源的资源包
     * @return UINT 0-成功,其它-失败
     */
    STDMETHOD_(UINT, LoadSystemNamedResource)(THIS_ IResProvider * pResProvider) PURE;

    /**
     * @brief 获取翻译接口
     * @return ITranslatorMgr *--翻译接口
     */
    STDMETHOD_(ITranslatorMgr *, GetTranslator)(THIS) PURE;

    /**
     * @brief 设置翻译接口
     * @param pTrans -- 翻译接口
     * @return void
     * @remark 翻译接口可以通过加载翻译模块来创建
     */
    STDMETHOD_(void, SetTranslator)(THIS_ ITranslatorMgr * pTrans) PURE;

    /**
     * @brief 获取ToolTip类厂
     * @return IToolTipFactory *--ToolTip类厂
     */
    STDMETHOD_(IToolTipFactory *, GetToolTipFactory)(THIS) PURE;

    /**
     * @brief 设置tooltip类厂
     * @param IToolTipFactory * pToolTipFac -- tooltip类厂
     * @return void
     * @remark 通过实现tooltip类厂可以自定义tooltip的显示效果
     */
    STDMETHOD_(void, SetToolTipFactory)(THIS_ IToolTipFactory * pToolTipFac) PURE;

    /**
     * @brief 设定MsgLoop类厂
     * @param IMsgLoopFactory * pMsgLoopFac -- MsgLoop类厂
     * @return BOOL TRUE-成功
     */
    STDMETHOD_(BOOL, SetMsgLoopFactory)(THIS_ IMsgLoopFactory * pMsgLoopFac) PURE;

    /**
     * @brief 获取当前的MsgLoop类厂
     * @return IMsgLoopFactory *--MsgLoop类厂
     */
    STDMETHOD_(IMsgLoopFactory *, GetMsgLoopFactory)(THIS) PURE;

    /**
     * @brief 设置日志输出模块
     * @param ILog4zManager *pLogMgr --日志输出模块
     * @return
     */
    STDMETHOD_(void, SetLogManager)(THIS_ ILog4zManager * pLogMgr) PURE;

    /**
     * @brief 获取日志输出模块
     * @return  ILog4zManager *--日志输出模块
     */
    STDMETHOD_(ILog4zManager *, GetLogManager)(THIS) PURE;

    /**
     * @brief 设置属性持久久类厂
     * @param IAttrStorageFactory* pAttrStorageFactory --属性持久久类厂
     * @remark
     * 实现该接口并设置给SApp，可以保存SObject通过SetAttribute设置的属性，在GetAttribute时提供数据
     */
    STDMETHOD_(void, SetAttrStorageFactory)(THIS_ IAttrStorageFactory * pAttrStorageFactory) PURE;

    /**
     * @brief 获取属性持久久类厂
     * @return IAttrStorageFactory *--属性持久久类厂
     */
    STDMETHOD_(IAttrStorageFactory *, GetAttrStorageFactory)(THIS) PURE;

    /**
     * @brief 进入消息循环运行程序
     * @param HWND hMainWnd -- 主窗口句柄
     * @return int -- 程序退出的返回值
     */
    STDMETHOD_(int, Run)(THIS_ HWND hMainWnd) PURE;

    /**
     * @brief 获取程序主窗口
     * @return HWND--在Run方法中设定的窗口
     */
    STDMETHOD_(HWND, GetMainWnd)(THIS) PURE;

    /**
     * @brief 将当前UI线程的msgLoop保存到SApp中
     * @param IMessageLoop *pMsgLoop -- msgLoop
     * @return BOOL TRUE-成功
     * @remark 在多UI线程程序中，需要将工作线程的msgLoop交给SApp管理
     */
    STDMETHOD_(BOOL, AddMsgLoop)(THIS_ IMessageLoop * pMsgLoop) PURE;

    /**
     * @brief 从SApp中删除当前线程的msgLoop
     * @return BOOL TRUE-成功
     */
    STDMETHOD_(BOOL, RemoveMsgLoop)(THIS) PURE;

    /**
     * @brief 获取指定UI线程的msgLoop
     * @param DWORD dwThreadID--线程ID
     * @return IMessageLoop *--指定线程的msgloop
     */
    STDMETHOD_(IMessageLoop *, GetMsgLoop)(THIS_ DWORD dwThreadID) SCONST PURE;

    /**
     * @brief 获取IResProviderMgr接口
     * @return IResProviderMgr *--IResProviderMgr接口
     */
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