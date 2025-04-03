#pragma once

class CDesignWnd : public SHostWnd
{
public:
	CDesignWnd();
	virtual ~CDesignWnd();

protected:
	void RefreshDesignLayout();
	void OnSize(UINT nType, CSize size);

	BEGIN_MSG_MAP_EX(CDesignWnd)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()
};
