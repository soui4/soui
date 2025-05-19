/**
 * @file SCaption.h
 * @brief 标题栏控件
 * @version v1.0
 * @author soui
 * @date 2014-05-28
 *
 * @details 此类完成标题栏控件的实现。
 */

#ifndef __SCAPTION__H__
#define __SCAPTION__H__

#include "core/SWnd.h"

SNSBEGIN

/**
 * @class SCaption
 * @brief 标题栏控件类
 * @details 此类实现了一个标题栏控件，支持鼠标事件处理。
 */
class SOUI_EXP SCaption : public SWindow {
    DEF_SOBJECT(SWindow, L"caption")

  public:
    /**
     * @brief 构造函数
     * @details 初始化标题栏控件。
     */
    SCaption(void);

    /**
     * @brief 析构函数
     * @details 释放标题栏控件资源。
     */
    virtual ~SCaption(void);

  protected:
    /**
     * @brief 处理左键按下事件
     * @param nFlags 标志
     * @param point 鼠标坐标
     * @details 此函数是消息响应函数，处理左键按下事件。
     */
    void OnLButtonDown(UINT nFlags, CPoint point);

    /**
     * @brief 处理左键双击事件
     * @param nFlags 标志
     * @param point 鼠标坐标
     * @details 此函数是消息响应函数，处理左键双击事件。
     */
    void OnLButtonDblClk(UINT nFlags, CPoint point);

    /**
     * @brief 处理鼠标移动事件
     * @param nFlags 标志
     * @param point 鼠标坐标
     * @details 此函数是消息响应函数，处理鼠标移动事件。
     */
    void OnMouseMove(UINT nFlags, CPoint point);

    /**
     * @brief 处理左键释放事件
     * @param nFlags 标志
     * @param point 鼠标坐标
     * @details 此函数是消息响应函数，处理左键释放事件。
     */
    void OnLButtonUp(UINT nFlags, CPoint point);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_LBUTTONUP(OnLButtonUp)
    SOUI_MSG_MAP_END()

  private:
    /**
     * @brief 标记最大化时鼠标左键是否已经按下
     */
    BOOL m_bIsMaxDown;
};

SNSEND

#endif // __SCAPTION__H__