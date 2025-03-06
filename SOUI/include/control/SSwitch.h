#pragma once
#include <core/SWnd.h>

SNSBEGIN

	class  SSwitch : public SWindow, protected IAnimatorListener, IAnimatorUpdateListener
	{
		DEF_SOBJECT(SWindow,L"switch")
	public:
		SSwitch();
		~SSwitch();

	protected:
		STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator* pAnimator) override {}
		STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator* pAnimator) override;
		STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator* pAnimator) override {}
		STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator* pAnimator) override;
	protected://SWindow的虚函数
		STDMETHOD_(void, GetDesiredSize)(THIS_ SIZE* psz, int nParentWid, int nParentHei) OVERRIDE;
		BOOL NeedRedrawWhenStateChange() override
		{
			return TRUE;
		}
	protected://消息处理，SOUI控件的消息处理和WTL，MFC很相似，采用相似的映射表，相同或者相似的消息映射宏
		void OnPaint(IRenderTarget *pRT);
		void OnLButtonUp(UINT nFlags, CPoint point);
		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

		//SOUI控件消息映射表
		SOUI_MSG_MAP_BEGIN()	
			MSG_WM_PAINT_EX(OnPaint)    //窗口绘制消息
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_KEYDOWN(OnKeyDown)
		SOUI_MSG_MAP_END()

		SAutoRefPtr<ISkinObj> m_pSkin;  /**< ISkinObj对象 */
		SAutoRefPtr<ISkinObj> m_pSkinForce; //

		HRESULT OnAttrAnimator(const SStringW& value, BOOL bLoading);
        HRESULT OnAttrCheck(const SStringW &strValue, BOOL bLoading);
		SOUI_ATTRS_BEGIN()
			ATTR_SKIN(L"skin", m_pSkin, TRUE)
			ATTR_SKIN(L"skinforce", m_pSkinForce, TRUE)
			ATTR_CUSTOM(L"animator",OnAttrAnimator)
            ATTR_CUSTOM(L"checked", OnAttrCheck)
			ATTR_CHAIN_PTR(m_animator,0)
		SOUI_ATTRS_END()
	private:
		SAutoRefPtr<IValueAnimator> m_animator;
	};

SNSEND