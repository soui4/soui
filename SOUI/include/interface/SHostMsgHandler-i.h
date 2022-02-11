#pragma once

SNSBEGIN

#undef INTERFACE
#define INTERFACE IHostMsgHandler
DECLARE_INTERFACE(IHostMsgHandler)
{
    /**
     * @brief 响应宿主消息
     * @param bRelayout BOOL--重布局标志
     * @param uMsg UINT--消息ID
     * @param wp WPARAM--消息参数1
     * @param lp LPARAM--消息参数2
     * @return
     * @remark 目前只修改系统字体时使用
     */
    STDMETHOD_(void, OnHostMsg)(THIS_ BOOL bRelayout, UINT uMsg, WPARAM wp, LPARAM lp) PURE;
};

SNSEND