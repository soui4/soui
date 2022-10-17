/*
SOUI窗口布局接口
*/
#pragma once

#include <interface/sobject-i.h>

SNSBEGIN

typedef struct IWindow IWindow;

typedef enum ORIENTATION
{
    Horz,
    Vert,
    Any,
    Both,
} ORIENTATION;

enum
{
    SIZE_UNDEF = -3,
    SIZE_WRAP_CONTENT = -1,
    SIZE_MATCH_PARENT = -2,
    SIZE_SPEC = 0,
};

#undef INTERFACE
#define INTERFACE ILayoutParam
DECLARE_INTERFACE_(ILayoutParam, IObject)
{
#include <interface/SobjectApi.h>

    //----------------------------------------------------------

    /**
     * @brief 清空数据
     * @return
     */
    STDMETHOD_(void, Clear)(THIS) PURE;

    /**
     * @brief 布局充满父窗口标志
     * @param orientation ORIENTATION--布局方向
     * @return TRUE--布局充满父窗口
     */
    STDMETHOD_(BOOL, IsMatchParent)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 布局适应窗口内容标志
     * @param orientation ORIENTATION--布局方向
     * @return TRUE--适应窗口内容标志
     */
    STDMETHOD_(BOOL, IsWrapContent)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 布局指定大小标志
     * @param orientation ORIENTATION--布局方向
     * @return TRUE--指定大小
     */
    STDMETHOD_(BOOL, IsSpecifiedSize)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 获取指定的布局大小
     * @param orientation ORIENTATION--布局方向
     * @return SLayoutSize--布局大小
     */
    STDMETHOD_(SLayoutSize, GetSpecifiedSize)(CTHIS_ ORIENTATION orientation) SCONST PURE;

    /**
     * @brief 设定布局适应父窗口大小
     * @param orientation ORIENTATION--布局方向
     * @return
     */
    STDMETHOD_(void, SetMatchParent)(THIS_ ORIENTATION orientation) PURE;

    /**
     * @brief 设定布局适应内容
     * @param orientation ORIENTATION--布局方向
     * @return
     */
    STDMETHOD_(void, SetWrapContent)(THIS_ ORIENTATION orientation) PURE;

    /**
     * @brief 设定布局大小
     * @param orientation ORIENTATION--布局方向
     * @param layoutSize SLayoutSize--布局大小
     * @return
     */
    STDMETHOD_(void, SetSpecifiedSize)
    (THIS_ ORIENTATION orientation, const SLayoutSize &layoutSize) PURE;

    /**
     * @brief 获取布局结构体数据
     * @return void*布局结构体
     */
    STDMETHOD_(void *, GetRawData)(THIS) PURE;

    /**
     * @brief Clone布局参数
     * @return ILayoutParam *--布局参数对象
     */
    STDMETHOD_(ILayoutParam *, Clone)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE ILayout
DECLARE_INTERFACE_(ILayout, IObject)
{
#include <interface/SobjectApi.h>
    //------------------------------------------------------------------------

    /**
     * @brief 判断当前布局类型和布局参数是否匹配
     * @param pLayoutParam const ILayoutParam *--布局参数
     * @return TRUE--匹配
     */
    STDMETHOD_(BOOL, IsParamAcceptable)(CTHIS_ const ILayoutParam *pLayoutParam) SCONST PURE;

    /**
     * @brief 布局指定控件的子窗口
     * @param pParent IWindow *--布局控件
     * @return
     */
    STDMETHOD_(void, LayoutChildren)(THIS_ IWindow * pParent) PURE;

    /**
     * @brief 创建和当前布局匹配的布局参数对象
     * @return ILayoutParam *--布局参数对象
     */
    STDMETHOD_(ILayoutParam *, CreateLayoutParam)(CTHIS) SCONST PURE;

    /**
     * @brief 计算窗口大小
     * @param pParent const IWindow*--目标窗口
     * @param nWidth int--父窗口宽度
     * @param nHeight int--父窗口高度
     * @return SIZE--目标窗口大小
     */
    STDMETHOD_(SIZE, MeasureChildren)
    (CTHIS_ const IWindow *pParent, int nWidth, int nHeight) SCONST PURE;
};

SNSEND