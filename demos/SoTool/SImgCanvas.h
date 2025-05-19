#ifndef _SIMG_CANVAS_H_
#define _SIMG_CANVAS_H_

SNSBEGIN

class SImgCanvas : public SWindow
    {
        DEF_SOBJECT(SWindow,L"imgcanvas")
    public:
        SImgCanvas(void);
        ~SImgCanvas(void);
        
        BOOL AddFile(LPCWSTR pszFileName);
        void Clear();
		BOOL Save2IconFile(LPCWSTR pszFileName);
		void PushIconSize(int size);
        BOOL Save2File(LPCWSTR pszFileName);
		void SaveSplits(LPCWSTR pszFileName);
		void Split(int n);
        void SetVertical(BOOL bVert);		
		bool IsCanSave() { return !m_lstImg.IsEmpty(); }
    protected:
        void OnPaint(IRenderTarget *pRT);
        void OnRButtonDown(UINT nFlags, CPoint point);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_RBUTTONDOWN(OnRButtonDown)
		SOUI_MSG_MAP_END()

		SList<int>		m_size;
        BOOL            m_bVert;
        SList<IBitmap*> m_lstImg;
    };
SNSEND

#endif//_SIMG_CANVAS_H_
