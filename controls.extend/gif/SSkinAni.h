#pragma once

#include <interface/SRender-i.h>
namespace SOUI
{
    class SSkinAni : public SSkinObjBase
    {
    DEF_SOBJECT_EX(SSkinObjBase, L"ani", L"gif|apng")
		class SAniFrame
		{
		public:
			SAutoRefPtr<IBitmapS> pBmp;
			int                  nDelay;
		};
    public:
        SSkinAni();

        virtual ~SSkinAni();

		virtual long GetFrameDelay(int iFrame = -1) const;

		virtual int LoadFromFile(LPCTSTR pszFileName);

        /**
         * LoadFromMemory
         * @brief    从内存加载APNG
         * @param    LPVOID pBits --  内存地址
         * @param    size_t szData --  内存数据长度
         * @return   int -- APNG帧数，0-失败
         * Describe  
         */    
        virtual int LoadFromMemory(LPVOID pBits, size_t szData);

		SIZE WINAPI GetSkinSize() const;

        int WINAPI GetStates() const;

		/**
        * ActiveNextFrame
        * @brief    激活下一帧
        * @return   void 
        * Describe  
        */    
        void ActiveNextFrame()
        {
            if(m_nFrames>1)
            {
                m_iFrame++;
                if(m_iFrame==m_nFrames) m_iFrame=0;
                SelectActiveFrame(m_iFrame);
            }
        }

        /**
        * SelectActiveFrame
        * @brief    激活指定帧
        * @param    int iFrame --  帧号
        * @return   void
        * Describe  
        */    
        void SelectActiveFrame(int iFrame) const
        {
            if(m_nFrames>1 && iFrame<m_nFrames)
            {
                m_iFrame = iFrame;
            }
        }

		virtual void _DrawByIndex(IRenderTarget *pRT, LPCRECT rcDraw, int iFrame, BYTE byAlpha/*=0xFF*/) const
		{
			if (iFrame != -1) SelectActiveFrame(iFrame);
			_DrawByIndex2(pRT, rcDraw, m_iFrame, byAlpha);
		}

		virtual void _Scale(ISkinObj *pObj, int nScale) override;

		LONG GetExpandCode() const{
			return MAKELONG(m_bTile?EM_TILE:EM_STRETCH,m_filterLevel);
		}
	public:
		SOUI_ATTRS_BEGIN()
			ATTR_RECT(L"margin",m_rcMargin,TRUE)
			ATTR_MARGIN(L"margin2",m_rcMargin,TRUE)
			ATTR_BOOL(L"tile",m_bTile,TRUE)
			ATTR_ENUM_BEGIN(L"filterLevel",FilterLevel,TRUE)
				ATTR_ENUM_VALUE(L"none",kNone_FilterLevel)
				ATTR_ENUM_VALUE(L"low",kLow_FilterLevel)
				ATTR_ENUM_VALUE(L"medium",kMedium_FilterLevel)
				ATTR_ENUM_VALUE(L"high",kHigh_FilterLevel)
			ATTR_ENUM_END(m_filterLevel)
            ATTR_CUSTOM(L"src", OnAttrSrc) // XML文件中指定的图片资源名,(type:name)
		SOUI_ATTRS_END()
	protected:
        HRESULT OnAttrSrc(const SStringW &strValue, BOOL bLoading);
        int _InitImgFrame(IImgX *pImgX);
		virtual void _DrawByIndex2(IRenderTarget *pRT, LPCRECT rcDraw, int iFrame, BYTE byAlpha/*=0xFF*/) const;

	protected:
		int m_nFrames;
        mutable int m_iFrame;
		CRect		m_rcMargin;
		FilterLevel	m_filterLevel;
		BOOL		m_bTile;

        SAniFrame *m_pFrames;
    };
    }