#pragma once

namespace SOUI
{
class SButtonEx : public SButton {
    DEF_SOBJECT(SButton, L"buttonex")

    enum
    {
        FREE_DRAW,
        VERT_ICON_TEXT,
        VERT_TEXT_ICON,
        HORZ_ICON_TEXT,
        HORZ_TEXT_ICON,
    };

  public:
    SButtonEx(void);
    ~SButtonEx(void);

    HICON SetIcon(HICON hIcon, int nIconSize);
    void SetIconVisible(bool bVisible);

  public:
    SOUI_ATTRS_BEGIN()
        ATTR_LAYOUTSIZE2(L"pushOffset", m_ptPushOffet, FALSE) //下压状态整体偏移
        ATTR_SKIN(L"icon", m_pIcon, FALSE)              //图标SKIN
        ATTR_INT(L"iconIndex", m_iIcon, FALSE)          //图标在Skin中的索引
        ATTR_ENUM_BEGIN(L"drawMode", int, FALSE)
            ATTR_ENUM_VALUE(L"freeDraw", FREE_DRAW)
            ATTR_ENUM_VALUE(L"vertIconText", VERT_ICON_TEXT)
            ATTR_ENUM_VALUE(L"vertTextIcon", VERT_TEXT_ICON)
            ATTR_ENUM_VALUE(L"horzIconText", HORZ_ICON_TEXT)
            ATTR_ENUM_VALUE(L"horzTextIcon", HORZ_TEXT_ICON)
        ATTR_ENUM_END(m_drawMode)
        ATTR_LAYOUTSIZE(L"sepSize", m_nSepSize, FALSE) // FREE_DRAW时无效
        ATTR_LAYOUTSIZE2(L"iconPos", m_ptIcon, FALSE) //图标显示位置,FREE_DRAW时有效
        ATTR_LAYOUTSIZE2(L"textPos", m_ptText, FALSE) //文本显示位置,FREE_DRAW时有效
    SOUI_ATTRS_END()

  protected:
    virtual CSize GetDesiredSize(LPCRECT pRcContainer);

    CSize GetDesiredSize(IRenderTarget *pRT, LPCRECT pRcContainer);
    CSize GetIconSize();
    void DrawIcon(IRenderTarget *pRT, CRect rcIcon);

    void OnScaleChanged(int scale);

  protected:
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
	    MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

  protected:
    HICON m_hIcon;
    SLayoutSize m_nIconSize;

    SAutoRefPtr<ISkinObj> m_pIcon;
    int m_iIcon; //-1代表m_pIcon是一个独占的皮肤
    SLayoutSize m_ptIcon[2];
    SLayoutSize m_ptText[2];
    SLayoutSize m_ptPushOffet[2];
    SLayoutSize m_nSepSize;
    int m_drawMode;
    bool m_bIconVisible;
};

} // namespace SOUI
