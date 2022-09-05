#pragma once

namespace SOUI
{
	class SMaskWnd : public SWindow {
		DEF_SOBJECT(SWindow, L"maskWnd")
	public:
		SMaskWnd(void);
		~SMaskWnd(void);

	protected:
		virtual void OnCommitSurface(IRenderTarget *pRtDest, LPCRECT pRcDest, IRenderTarget *pRtSrc, LPCRECT pRcSrc, BYTE alpha) OVERRIDE;

	protected:
		HRESULT OnAttrMask(const SStringW &strValue, BOOL bLoading);

		void MakeCacheApha(IBitmapS* cache);

		SOUI_ATTRS_BEGIN()
			ATTR_CUSTOM(L"mask", OnAttrMask) // image.a
		SOUI_ATTRS_END()

		SAutoRefPtr<IBitmapS> m_bmpMask;
		int m_iMaskChannel;
	};
} // namespace SOUI
