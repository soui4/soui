#pragma once
SNSBEGIN

class SRoundWnd : public SWindow
{
	DEF_SOBJECT(SWindow,L"roundWnd")

private:
	int m_cornerSize;
public:
	SRoundWnd(void);
	~SRoundWnd(void);

public:
	SOUI_ATTRS_BEGIN()
		ATTR_INT(L"cornerSize",m_cornerSize,TRUE)
	SOUI_ATTRS_END()

protected:
    void OnCommitSurface(IRenderTarget *pRtDest,
                                 LPCRECT pRcDest,
                                 IRenderTarget *pRtSrc,
                                 LPCRECT pRcSrc,
                                 BYTE alpha) override;

};

SNSEND
