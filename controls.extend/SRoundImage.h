#pragma once
namespace SOUI
{
	class SRoundImage : public SImageWnd
	{
		DEF_SOBJECT(SImageWnd,L"roundImg")
	private:
		SAutoRefPtr<IBitmapS> m_img;
		int m_cornerSize;
	public:
		SRoundImage();
		~SRoundImage();

	public:
		HRESULT OnAttrCornerSize(const SStringW & value,BOOL bLoading);
		SOUI_ATTRS_BEGIN()
			ATTR_CUSTOM(L"cornerSize",OnAttrCornerSize)
		SOUI_ATTRS_END()
	protected:
		SAutoRefPtr<IBitmapS> GetRoundImage() const;

		void OnPaint(IRenderTarget *pRT);
		void OnSize(UINT nType, CSize size);
		SOUI_MSG_MAP_BEGIN()
			MSG_WM_SIZE(OnSize)
			MSG_WM_PAINT_EX(OnPaint)
		SOUI_MSG_MAP_END()
	};

}

