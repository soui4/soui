#ifndef __STOOLTIP__H__
#define __STOOLTIP__H__

#include <interface/stooltip-i.h>
#include <core/SNativeWnd.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @class STipCtrl
 * @brief 提示控件类，继承自IToolTip和SNativeWnd
 */
class STipCtrl
    : public IToolTip
    , public SNativeWnd {
  public:
    /**
     * @brief 构造函数
     */
    STipCtrl(void);

    /**
     * @brief 析构函数
     */
    virtual ~STipCtrl(void);

    /**
     * @brief 创建提示控件
     * @return 如果成功创建返回TRUE，否则返回FALSE
     */
    BOOL Create();

  public: // IToolTip
    /**
     * @brief 预翻译消息
     * @param pMsg 消息结构指针
     * @return 如果消息被处理返回TRUE，否则返回FALSE
     */
    STDMETHOD_(BOOL, PreTranslateMessage)(THIS_ MSG *pMsg) OVERRIDE;

    /**
     * @brief 更新提示信息
     * @param id 提示ID指针
     * @param rc 目标矩形区域
     * @param pszTip 提示文本
     * @param nScale 缩放比例
     */
    STDMETHOD_(void, UpdateTip)(THIS_ const TIPID *id, RECT rc, LPCTSTR pszTip, int nScale) OVERRIDE;

    /**
     * @brief 清除提示信息
     */
    STDMETHOD_(void, ClearTip)(THIS) OVERRIDE;

    /**
     * @brief 传递事件
     * @param pMsg 消息结构指针
     */
    STDMETHOD_(void, RelayEvent)(THIS_ const MSG *pMsg) OVERRIDE;

    /**
     * @brief 设置提示信息
     * @param rc 目标矩形区域
     * @param tipAlign 提示对齐方式
     * @param pszTip 提示文本
     * @param nScale 缩放比例
     */
    STDMETHOD_(void, SetToolTip)(THIS_ LPCRECT rc, UINT tipAlign, LPCTSTR pszTip, int nScale) OVERRIDE;

  protected:
    /**
     * @brief 处理最终消息
     * @param hWnd 窗口句柄
     */
    virtual void OnFinalMessage(HWND hWnd);

    /**
     * @brief 更新字体
     * @param nScale 缩放比例
     */
    void UpdateFont(int nScale);

    /**
     * @brief 处理定时器消息
     * @param idEvent 定时器ID
     */
    void OnTimer(UINT_PTR idEvent);

    /**
     * @brief 处理绘制消息
     * @param dc 设备上下文句柄
     */
    void OnPaint(HDC dc);

    /**
     * @brief 显示或隐藏提示
     * @param bShow 是否显示提示
     */
    void ShowTip(BOOL bShow);

    BEGIN_MSG_MAP_EX(STipCtrl)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_TIMER(OnTimer)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

  protected:
    int m_nDelay;      // 显示延迟时间（毫秒）
    int m_nShowSpan;   // 显示持续时间（毫秒）
    SStringT m_strTip; // 提示文本
    int m_nScale;      // 缩放比例
    CRect m_rcTarget;  // 目标矩形区域
    HFONT m_font;      // 字体句柄

    TIPID m_id; // 提示ID

    UINT m_tipAlign; // 提示对齐方式
};

SNSEND

#endif // __STOOLTIP__H__