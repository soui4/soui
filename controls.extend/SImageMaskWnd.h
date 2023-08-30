#pragma once

namespace SOUI
{
class SImageMaskWnd : public SWindow {
    DEF_SOBJECT(SWindow, L"imageMask")
  public:
    SImageMaskWnd(void);
    ~SImageMaskWnd(void);

    void SetImage(IBitmapS *pBitmap, FilterLevel fl = kHigh_FilterLevel);

  protected:
    void OnPaint(IRenderTarget *pRT);

    SOUI_MSG_MAP_BEGIN()
    MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

    HRESULT OnAttrMask(const SStringW &strValue, BOOL bLoading);
    HRESULT OnAttrImage(const SStringW &strValue, BOOL bLoading);

    void UpdateCacheFromSkin(ISkinObj *pSkin);
    void UpdateCacheFromImage(IBitmapS *pBitmap, FilterLevel fl);
    void MakeCacheAlpha();

    SOUI_ATTRS_BEGIN()
    ATTR_CUSTOM(L"mask", OnAttrMask) // image.a
    ATTR_CUSTOM(L"skin", OnAttrImage)
    SOUI_ATTRS_END()

    SStringW m_strSkin;
    SAutoRefPtr<IBitmapS> m_bmpCache;
    SAutoRefPtr<IBitmapS> m_bmpMask;
    int m_iMaskChannel;
};
} // namespace SOUI
