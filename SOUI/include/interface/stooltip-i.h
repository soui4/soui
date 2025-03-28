﻿#ifndef __STOOLTIP_I__H__
#define __STOOLTIP_I__H__

#include <interface/obj-ref-i.h>
#include <interface/SMsgLoop-i.h>

SNSBEGIN

/**
 * @struct    TIPID
 * @brief     标识一个tooltip的ID
 *
 * Describe
 */
typedef struct TIPID
{
    DWORD dwHi;  // ID1, 用来保存一个SWND。
    DWORD dwLow; // ID2, 用来保存附加数据
    BOOL bNcTip; // 非客户区tip
} TIPID;

/**
 * @struct    IToolTip
 * @brief     tooltip对象接口
 *
 * Describe
 */

typedef enum TipAlign
{
    TA_AUTO = 0,
    TA_X_LEFT = 1,
    TA_X_CENTER = 2,
    TA_X_RIGHT = 3,
    TA_X_MASK = 0x0f,
    TA_Y_TOP = 0x10,
    TA_Y_CENTER = 0x20,
    TA_Y_BOTTOM = 0x30,
    TA_Y_MASK = 0xF0,
} TipAlign;

#undef INTERFACE
#define INTERFACE IToolTip
DECLARE_INTERFACE_(IToolTip, IMsgFilter)
{
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG * pMsg) PURE;

    /**
     * UpdateTip
     * @brief    更新当前的tooltip
     * @param    const TIPID & id --  tooltip的ID
     * @param    CRect rc --  tooltip的感应区
     * @param    LPCTSTR pszTip --  tooltip字符串
     * @param    int nScale -- 当前的缩放比例
     * @return   void
     *
     * Describe
     */
    STDMETHOD_(void, UpdateTip)(THIS_ const TIPID *id, RECT rc, LPCTSTR pszTip, int nScale) PURE;

    /**
     * @brief 在指定位置显示tooltip
     * @param LPCRECT rc tip显示位置，相对于container
     * @param UINT tipAlign tip对齐方式
     * @param pszTip LPCTSTR tip内容，为null时退出固定tip
     * @param int nScale -- 当前的缩放比例
     * @return
     */
    STDMETHOD_(void, SetToolTip)(THIS_ LPCRECT rc, UINT tipAlign, LPCTSTR pszTip, int nScale) PURE;

    /**
     * ClearTip
     * @brief    清除当前的tooltip
     * @return   void
     *
     * Describe
     */
    STDMETHOD_(void, ClearTip)(THIS) PURE;

    /**
     * RelayEvent
     * @brief    鼠标消息的中继
     * @param    const MSG * pMsg --  鼠标消息
     * @return   void
     *
     * Describe
     */
    STDMETHOD_(void, RelayEvent)(THIS_ const MSG *pMsg) PURE;
};

/**
 * @struct    IToolTipFactory
 * @brief     tooltip类厂
 *
 * Describe
 */
#undef INTERFACE
#define INTERFACE IToolTipFactory
DECLARE_INTERFACE_(IToolTipFactory, IObjRef)
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

    /**
     * CreateToolTip
     * @brief    创建tooltip对象
     * @param    HWND hHost --  soui host
     * @return   IToolTip * 创建的tooltip对象
     *
     * Describe  不要将hHost做为tooltip的Owner，否则可能导致程序运行异常。
     */
    STDMETHOD_(IToolTip *, CreateToolTip)(THIS_ HWND hHost) PURE;

    /**
     * DestroyToolTip
     * @brief    销毁tooltip对象
     * @param    IToolTip * pToolTip --  待销毁tooltip
     * @return   void
     *
     * Describe
     */
    STDMETHOD_(void, DestroyToolTip)(THIS_ IToolTip * pToolTip) PURE;
};

SNSEND
#endif // __STOOLTIP_I__H__