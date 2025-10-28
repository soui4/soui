#ifndef __SLIGHTBUTTON_H__
#define __SLIGHTBUTTON_H__

SNSBEGIN
    /**
     * @class SLightButton
     * @brief 高亮发光按钮控件 - SOUI版本
     * 
     * 功能特性：
     * 1. 可设置文本，居中显示
     * 2. 可设置文本颜色
     * 3. 可设置外边框渐变颜色
     * 4. 可设置里边框渐变颜色
     * 5. 可设置背景色
     * 6. 可设置是否显示矩形
     * 7. 可设置报警颜色、正常颜色
     * 8. 可控制启动报警和停止报警，报警时闪烁
     * 9. 支持预设颜色方案（绿色、红色、黄色、黑色、蓝色等）
     */
    class SLightButton : public SWindow
    {
        DEF_SOBJECT(SWindow, L"lightbutton")

    public:
        SLightButton();
        virtual ~SLightButton();

    protected:
        // 绘制相关方法
        void OnPaint(IRenderTarget *pRT);
        void DrawBorderOut(IRenderTarget *pRT, const CRect& rcClient);
        void DrawBorderIn(IRenderTarget *pRT, const CRect& rcClient);
        void DrawBg(IRenderTarget *pRT, const CRect& rcClient);
        void DrawText(IRenderTarget *pRT, const CRect& rcClient);
        void DrawOverlay(IRenderTarget *pRT, const CRect& rcClient);

        // 鼠标事件处理
        void OnLButtonDown(UINT nFlags, CPoint point);
        void OnLButtonUp(UINT nFlags, CPoint point);
        void OnMouseMove(UINT nFlags, CPoint point);

        // 定时器处理
        void OnTimer(char cTimerID);

        // 消息映射
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_TIMER_EX(OnTimer)
        SOUI_MSG_MAP_END()

    public:
        // 属性定义
        SOUI_ATTRS_BEGIN()
            ATTR_STRINGT(L"text", m_strText, FALSE)
            ATTR_COLOR(L"textColor", m_crTextColor, FALSE)
            ATTR_COLOR(L"alarmColor", m_crAlarmColor, FALSE)
            ATTR_COLOR(L"normalColor", m_crNormalColor, FALSE)
            ATTR_COLOR(L"borderOutColorStart", m_crBorderOutColorStart, FALSE)
            ATTR_COLOR(L"borderOutColorEnd", m_crBorderOutColorEnd, FALSE)
            ATTR_COLOR(L"borderInColorStart", m_crBorderInColorStart, FALSE)
            ATTR_COLOR(L"borderInColorEnd", m_crBorderInColorEnd, FALSE)
            ATTR_COLOR(L"bgColor", m_crBgColor, FALSE)
            ATTR_BOOL(L"showRect", m_bShowRect, FALSE)
            ATTR_BOOL(L"showOverlay", m_bShowOverlay, FALSE)
            ATTR_COLOR(L"overlayColor", m_crOverlayColor, FALSE)
        SOUI_ATTRS_END()

    public:
        // 获取和设置文本
        SStringT GetText() const { return m_strText; }
        void SetText(const SStringT& strText);

        // 获取和设置文本颜色
        COLORREF GetTextColor() const { return m_crTextColor; }
        void SetTextColor(COLORREF crTextColor);

        // 获取和设置报警颜色
        COLORREF GetAlarmColor() const { return m_crAlarmColor; }
        void SetAlarmColor(COLORREF crAlarmColor);

        // 获取和设置正常颜色
        COLORREF GetNormalColor() const { return m_crNormalColor; }
        void SetNormalColor(COLORREF crNormalColor);

        // 获取和设置外边框渐变颜色
        COLORREF GetBorderOutColorStart() const { return m_crBorderOutColorStart; }
        void SetBorderOutColorStart(COLORREF crColor);

        COLORREF GetBorderOutColorEnd() const { return m_crBorderOutColorEnd; }
        void SetBorderOutColorEnd(COLORREF crColor);

        // 获取和设置里边框渐变颜色
        COLORREF GetBorderInColorStart() const { return m_crBorderInColorStart; }
        void SetBorderInColorStart(COLORREF crColor);

        COLORREF GetBorderInColorEnd() const { return m_crBorderInColorEnd; }
        void SetBorderInColorEnd(COLORREF crColor);

        // 获取和设置背景色
        COLORREF GetBgColor() const { return m_crBgColor; }
        void SetBgColor(COLORREF crBgColor);

        // 获取和设置显示矩形
        BOOL GetShowRect() const { return m_bShowRect; }
        void SetShowRect(BOOL bShowRect);

        // 获取和设置是否显示遮罩层
        BOOL GetShowOverlay() const { return m_bShowOverlay; }
        void SetShowOverlay(BOOL bShowOverlay);

        // 获取和设置遮罩层颜色
        COLORREF GetOverlayColor() const { return m_crOverlayColor; }
        void SetOverlayColor(COLORREF crOverlayColor);

    public:
        // 预设颜色方案
        void SetGreen();
        void SetRed();
        void SetYellow();
        void SetBlack();
        void SetGray();
        void SetBlue();
        void SetLightBlue();
        void SetLightRed();
        void SetLightGreen();

        // 报警控制
        void StartAlarm();
        void StopAlarm();

    protected:
        // 内部方法
        void UpdateAlarmState();
        CRect GetScaledRect(const CRect& rcClient, float fScale) const;

    protected:
        // 属性变量
        SStringT m_strText;                 // 文本
        COLORREF m_crTextColor;             // 文字颜色
        COLORREF m_crAlarmColor;            // 报警颜色
        COLORREF m_crNormalColor;           // 正常颜色

        COLORREF m_crBorderOutColorStart;   // 外边框渐变开始颜色
        COLORREF m_crBorderOutColorEnd;     // 外边框渐变结束颜色
        COLORREF m_crBorderInColorStart;    // 里边框渐变开始颜色
        COLORREF m_crBorderInColorEnd;      // 里边框渐变结束颜色
        COLORREF m_crBgColor;               // 背景颜色

        BOOL m_bShowRect;                   // 显示成矩形
        BOOL m_bShowOverlay;                // 是否显示遮罩层
        COLORREF m_crOverlayColor;          // 遮罩层颜色

        BOOL m_bIsAlarm;                    // 是否报警
        char m_cAlarmTimerID;               // 报警定时器ID
    };

SNSEND

#endif // __SLIGHTBUTTON_H__