#pragma once
#include <list>
#include <vector>
SNSBEGIN

struct ILrcProvider : IObjRef{
	virtual BOOL load(LPCTSTR pszLrc, int durationMs) = 0;
	virtual float getLineIndexFromTimeMs(int timeMs) const = 0;
	virtual float getWordRatio(int iLine, int timeMs) const = 0;
	virtual int getLineCount() const = 0;
	virtual SStringT getLineText(int iLine) const = 0;
	virtual int getTsEndMs() const = 0;
	virtual bool isHeadLine(int iLine) const = 0;
	
	// 歌词偏移控制
	virtual void setOffset(int nOffsetMs) = 0;
	virtual int getOffset() const = 0;
};

class SYYLrcProvider : public TObjRefImpl<ILrcProvider> {
	struct Tone {
		int nOffset;
		int nLength;
		int tsBeginMs;
		int tsEndMs;
		int pitch;
	};
	struct Sentence {
		int tsBeginMs;
		int tsEndMs;
		SStringT strSent;
		bool isHead;
		std::list<Tone> tones;
	};

	std::vector<Sentence> m_lstSents;
	int m_nOffsetMs;        // 歌词时间偏移（毫秒）
	SStringT m_strName;
	SStringT m_strSinger;	
public:
	SYYLrcProvider();
	~SYYLrcProvider();
	
public:
	// 通过 TObjRefImpl 继承
	BOOL load(LPCTSTR pszLrc, int durationMs) override;
	float getLineIndexFromTimeMs(int timeMs) const override;
	float getWordRatio(int iLine, int timeMs) const override;
	int getLineCount() const override;
	SStringT getLineText(int iLine) const override;
	int getTsEndMs() const override;
	bool isHeadLine(int iLine) const override;
	
	// 歌词偏移控制
	void setOffset(int nOffsetMs) override{
		m_nOffsetMs = nOffsetMs;
	}
	int getOffset() const override{
		return m_nOffsetMs;
	}
protected:
	int ts2line(int iFirst, int iLast, int timeMs) const;
};

/**
 * @brief 传统LRC格式歌词提供者
 * 
 * 支持标准LRC格式：
 * [ti:歌曲名]
 * [ar:歌手]
 * [al:专辑]
 * [mm:ss.xx]歌词内容
 */
class SLrcProvider : public TObjRefImpl<ILrcProvider> {
	struct LrcLine {
		int tsBeginMs;      // 开始时间（毫秒）
		int tsEndMs;        // 结束时间（毫秒）
		SStringT strText;   // 歌词文本
	};

	std::vector<LrcLine> m_lines;
	SStringT m_strTitle;
	SStringT m_strArtist;
	SStringT m_strAlbum;
	int m_durationMs;       // 歌曲总时长
	SStringT m_strLrcPath;  // 歌词文件路径
	int m_nOffsetMs;        // 歌词时间偏移（毫秒）

public:
	SLrcProvider();
	~SLrcProvider();

public:
	// ILrcProvider 接口实现
	BOOL load(LPCTSTR pszLrc, int durationMs) override;
	float getLineIndexFromTimeMs(int timeMs) const override;
	float getWordRatio(int iLine, int timeMs) const override;
	int getLineCount() const override;
	SStringT getLineText(int iLine) const override;
	int getTsEndMs() const override;
	bool isHeadLine(int iLine) const override;
	
	// 歌词偏移控制
	void setOffset(int nOffsetMs) override{
		m_nOffsetMs = nOffsetMs;
	}
	int getOffset() const override{
		return m_nOffsetMs;
	}

protected:
	// 解析时间标签 [mm:ss.xx] 为毫秒
	static int parseTimeTag(const SStringA& timeTag);
	
	// 二分查找时间对应的行索引
	int findLineByTime(int timeMs) const;
	
	// 将偏移应用到 LRC 文件并保存（析构时调用）
	void applyOffsetToFile();
};

class SLrcView : public SWindow
{
	DEF_SOBJECT(SWindow,L"lrcView")
public:
	SLrcView();

	void SetLrc(ILrcProvider* pProvider);
	BOOL SetTimeMs(int nPos);
	ILrcProvider* GetLrcProvider() const { return m_provider; }  // 添加获取 Provider 的方法

protected:
	void drawLine(IRenderTarget* pRT, CRect rc, int iLine, float fProgress, bool bIsCurrent);
	void drawLineWithTransition(IRenderTarget* pRT, CRect rc, int iLine, bool bIsCurrent);  // 带切换动画的绘制
	void OnPaint(IRenderTarget* pRT);
	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
	SOUI_MSG_MAP_END()

	SOUI_ATTRS_BEGIN()
		ATTR_COLOR(L"colorHighlight", m_crHighlight, TRUE)
		ATTR_COLOR(L"colorText", m_crText, TRUE)
		ATTR_LAYOUTSIZE(L"lineHeight",m_lineHei,TRUE)
		ATTR_INT(L"fontSizeNormal", m_nFontSizeNormal, TRUE)
		ATTR_INT(L"fontSizeCurrent", m_nFontSizeCurrent, TRUE)
		ATTR_FLOAT(L"scaleDuration", m_fScaleDuration, TRUE)
	SOUI_ATTRS_END()
private:
	SAutoRefPtr<ILrcProvider> m_provider;
	COLORREF m_crText;
	COLORREF m_crHighlight;

	SLayoutSize m_lineHei;
	int m_nLength;
	int m_timeAniMs;
	int m_timeMs;
	
	// 字体大小配置
	int m_nFontSizeNormal;      // 普通行字号
	int m_nFontSizeCurrent;     // 当前行字号
	float m_fScaleDuration;     // 缩放动画持续时间（秒）
	
	// 歌词切换动画状态
	int m_nCurrentLineIndex;     // 当前行索引
	int m_nLastLineIndex;        // 上一行索引（用于切换动画绘制）
	float m_fTransitionProgress; // 切换动画进度 (0.0 - 1.0)
	bool m_bIsTransitioning;     // 是否正在切换动画中
	int m_nTransitionStartTimeMs; // 切换动画开始时的播放时间（毫秒）
	
	// Y坐标滚动动画
	int m_nLastScrollOffset;    // 上一行的Y偏移
	int m_nCurrentScrollOffset; // 当前行的Y偏移
};

SNSEND

