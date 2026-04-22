#include "stdafx.h"
#include "SLrcView.h"
#include <algorithm>
#include <string>
#include <vector>
#include <helper/slog.h>
#define kLogTag "SLrcView"

SNSBEGIN

BOOL SYYLrcProvider::load(LPCTSTR pszLrc, int durationMs)
{
	SXmlDoc xmlDoc;
	if (!xmlDoc.load_file(pszLrc))
		return FALSE;
	SXmlNode xmlSong = xmlDoc.root().child(L"song");
	if (!xmlSong)
		return FALSE;
	m_strName = S_CW2T(xmlSong.child(L"general").child(L"name").Text());
	m_strSinger = S_CW2T(xmlSong.child(L"general").child(L"singer").Text());
	SXmlNode xmlLrc = xmlSong.child(L"midi_lrc");
	SXmlNode xmlComment = xmlSong.child(L"comment");
	if (!xmlLrc && !xmlComment)
	{
		return FALSE;
	}
	if (xmlLrc)
	{
		SXmlNode xmlPara = xmlLrc.child(L"paragraph");

		while (xmlPara) {
			SXmlNode xmlSent = xmlPara.child(L"sentence");
			while (xmlSent) {
				SXmlNode xmlTone = xmlSent.child(L"tone");
				int nOffset = 0;
				Sentence sent;
				sent.isHead = false;
				while (xmlTone) {
					Tone tone;
					tone.nOffset = nOffset;
					tone.tsBeginMs = xmlTone.attribute(L"begin").as_float() * 1000;
					tone.tsEndMs = xmlTone.attribute(L"end").as_float() * 1000;
					tone.pitch = xmlTone.attribute(L"pitch").AsInt();
					SStringT strWord = S_CW2T(xmlTone.child(L"word").Text());
					tone.nLength = strWord.GetLength();
					nOffset += tone.nLength;
					sent.strSent.Append(strWord);
					sent.tones.push_back(tone);

					xmlTone = xmlTone.next_sibling(L"tone");
				}
				if (!sent.tones.empty()) {
					sent.tsBeginMs = sent.tones.front().tsBeginMs;
					sent.tsEndMs = sent.tones.back().tsEndMs;
					m_lstSents.push_back(sent);
				}
				xmlSent = xmlSent.next_sibling(L"sentence");
			}
			xmlPara = xmlPara.next_sibling(L"paragraph");
		}

		if (m_lstSents.empty())
			return FALSE;
		//insert title as the first sent
		Sentence sent;
		sent.isHead = true;
		sent.tsBeginMs = 0;
		sent.strSent = SStringT().Format(_T("%s\n%s"), m_strName.c_str(), m_strSinger.c_str());
		Tone tone;
		tone.nLength = sent.strSent.GetLength();
		tone.pitch = 0;
		tone.nOffset = 0;
		tone.tsBeginMs = 0;
		tone.tsEndMs = m_lstSents.front().tsBeginMs;
		sent.tsEndMs = tone.tsEndMs;
		sent.tones.push_back(tone);
		m_lstSents.insert(m_lstSents.begin(), sent);
		return TRUE;
	}
	else {
		SStringT strComment = S_CW2T(xmlComment.Text());
		Sentence sent;
		sent.isHead = true;
		sent.tsBeginMs = 0;
		sent.strSent = strComment;
		Tone tone;
		tone.nLength = sent.strSent.GetLength();
		tone.pitch = 0;
		tone.nOffset = 0;
		tone.tsBeginMs = 0;
		tone.tsEndMs = durationMs;
		sent.tsEndMs = tone.tsEndMs;
		sent.tones.push_back(tone);
		m_lstSents.push_back(sent);
		return TRUE;
	}

}

float SYYLrcProvider::getLineIndexFromTimeMs(int timeMs) const
{
	// 应用偏移量：实际查找时间 = 播放时间 - 偏移量
	int adjustedTimeMs = timeMs - m_nOffsetMs;
	
	int iLine = ts2line(0, m_lstSents.size(), adjustedTimeMs);
	const Sentence& sent = m_lstSents[iLine];
	if (iLine == m_lstSents.size() - 1) {
		//last line
		int duration = sent.tsEndMs - sent.tsBeginMs;
		int fraction = adjustedTimeMs - sent.tsBeginMs;
		return iLine + (float)fraction / duration;
	}
	else {
		const Sentence& sent2 = m_lstSents[iLine+1];
		if (adjustedTimeMs < sent.tsBeginMs) {
			return iLine;
		}
		int duration = sent2.tsBeginMs - sent.tsBeginMs;
		int fraction = adjustedTimeMs - sent.tsBeginMs;
		return iLine + (float)fraction / duration;
	}
}

float SYYLrcProvider::getWordRatio(int iLine, int timeMs) const
{
	// 应用偏移量
	int adjustedTimeMs = timeMs - m_nOffsetMs;
	
	const Sentence& sent = m_lstSents[iLine];
	if(adjustedTimeMs <= sent.tsBeginMs)
		return 0.0f;
	if (adjustedTimeMs >= sent.tsEndMs)
		return 1.0f;
	for (std::list<Tone>::const_iterator it = sent.tones.begin(); it != sent.tones.end();it++) {
		if (adjustedTimeMs >= it->tsBeginMs) {
			float ratioOfWord = float(adjustedTimeMs - it->tsBeginMs) / (it->tsEndMs - it->tsBeginMs);
			return (float(it->nOffset) + ratioOfWord) / sent.strSent.GetLength();
		}
	}
	return 1.0f;
}

int SYYLrcProvider::getLineCount() const
{
	return m_lstSents.size();
}

SStringT SYYLrcProvider::getLineText(int iLine) const
{
	SASSERT(iLine >= 0 && iLine < m_lstSents.size());
	return m_lstSents[iLine].strSent;
}

int SYYLrcProvider::getTsEndMs() const
{
	if (m_lstSents.empty())
		return 0;
	return m_lstSents.back().tsEndMs;
}

bool SYYLrcProvider::isHeadLine(int iLine) const
{
	return m_lstSents[iLine].isHead;
}

SYYLrcProvider::SYYLrcProvider()
{
	m_nOffsetMs = 0;
}
SYYLrcProvider::~SYYLrcProvider()
{
	// YY格式不支持保存偏移，直接放弃
	if (m_nOffsetMs != 0)
	{
		SLOGI() << "SYYLrcProvider destructor: discarding offset " << m_nOffsetMs << "ms (YY format not supported)";
	}
}

int SYYLrcProvider::ts2line(int iFirst, int iLast, int timeMs) const
{
	int iMid = (iFirst + iLast) / 2;
	const Sentence& sentMid = m_lstSents[iMid];
	if (timeMs < sentMid.tsBeginMs)
	{
		if (iFirst == iMid)
			return iMid;
		return ts2line(iFirst, iMid, timeMs);
	}
	if (timeMs > sentMid.tsEndMs) {
		if (iMid + 1 == iLast)
			return iMid;
		return ts2line(iMid + 1, iLast, timeMs);
	}
	return iMid;
}


///////////////////////////////////////////////////////////////////////////////
// SLrcProvider Implementation - Traditional LRC Format Parser
///////////////////////////////////////////////////////////////////////////////

SLrcProvider::SLrcProvider()
	: m_durationMs(0)
	, m_nOffsetMs(0)
{
}

SLrcProvider::~SLrcProvider()
{
	// 析构时如果有偏移量，自动应用到文件
	if (m_nOffsetMs != 0 && !m_strLrcPath.IsEmpty())
	{
		SLOGI() << "SLrcProvider destructor: applying offset " << m_nOffsetMs << "ms to file";
		applyOffsetToFile();
	}
}

/**
 * @brief 检测BOM并返回编码类型和数据偏移
 * @param pData 文件数据指针
 * @param dataSize 数据大小
 * @param outOffset 输出的数据偏移量（跳过BOM）
 * @return 代码页 (CP_UTF8, CP_ACP, 或 -1表示UTF-16)
 */
static int DetectEncodingAndBOM(const char* pData, size_t dataSize, size_t& outOffset)
{
	outOffset = 0;
	
	if (dataSize >= 3 && 
		(unsigned char)pData[0] == 0xEF && 
		(unsigned char)pData[1] == 0xBB && 
		(unsigned char)pData[2] == 0xBF)
	{
		// UTF-8 with BOM
		outOffset = 3;
		SLOGI() << "Detected UTF-8 with BOM";
		return CP_UTF8;
	}
	else if (dataSize >= 2 && 
			 (unsigned char)pData[0] == 0xFF && 
			 (unsigned char)pData[1] == 0xFE)
	{
		// UTF-16 LE with BOM
		outOffset = 2;
		SLOGI() << "Detected UTF-16 LE with BOM";
		return -1;  // Special handling needed
	}
	else if (dataSize >= 2 && 
			 (unsigned char)pData[0] == 0xFE && 
			 (unsigned char)pData[1] == 0xFF)
	{
		// UTF-16 BE with BOM
		outOffset = 2;
		SLOGI() << "Detected UTF-16 BE with BOM";
		return -1;  // Special handling needed
	}
	else
	{
		// No BOM - will try UTF-8 first, then ACP
		SLOGI() << "No BOM detected, will try UTF-8 then ACP";
		return 0;  // Signal to try detection
	}
}

/**
 * @brief 将文件内容转换为UTF-8字符串
 * @param pszFilePath 文件路径
 * @param outUtf8Text 输出的UTF-8文本
 * @return 成功返回TRUE
 */
static BOOL ConvertFileToUtf8(LPCTSTR pszFilePath, std::string& outUtf8Text)
{
	// 以二进制模式读取整个文件
	FILE* fp = _tfopen(pszFilePath, _T("rb"));
	if (!fp)
	{
		SLOGW() << "Failed to open file: " << pszFilePath;
		return FALSE;
	}
	
	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	if (fileSize <= 0 || fileSize > 10 * 1024 * 1024)  // 最大10MB
	{
		fclose(fp);
		SLOGW() << "Invalid file size: " << fileSize;
		return FALSE;
	}
	
	std::vector<char> fileBuffer(fileSize);
	size_t bytesRead = fread(&fileBuffer[0], 1, fileSize, fp);
	fclose(fp);
	
	if (bytesRead == 0)
	{
		SLOGW() << "Failed to read file";
		return FALSE;
	}
	
	const char* pData = &fileBuffer[0];
	size_t dataSize = bytesRead;
	size_t dataOffset = 0;
	
	// 检测编码
	int codePage = DetectEncodingAndBOM(pData, dataSize, dataOffset);
	
	if (codePage == -1)
	{
		// UTF-16处理：直接转换为Unicode，再转UTF-8
		const unsigned char* pSrc = reinterpret_cast<const unsigned char*>(pData + dataOffset);
		size_t wideCharCount = (dataSize - dataOffset) / 2;
		
		if (wideCharCount == 0)
			return FALSE;
		
		std::vector<wchar_t> wideBuffer(wideCharCount);
		
		if (codePage == -1 && dataOffset == 2 && 
			(unsigned char)pData[0] == 0xFF && (unsigned char)pData[1] == 0xFE)
		{
			// UTF-16 LE
			for (size_t i = 0; i < wideCharCount; ++i)
			{
				wideBuffer[i] = pSrc[i*2] | (pSrc[i*2+1] << 8);
			}
		}
		else
		{
			// UTF-16 BE
			for (size_t i = 0; i < wideCharCount; ++i)
			{
				wideBuffer[i] = (pSrc[i*2] << 8) | pSrc[i*2+1];
			}
		}
		
		// UTF-16 -> UTF-8
		int utf8Len = WideCharToMultiByte(CP_UTF8, 0, &wideBuffer[0], 
										  static_cast<int>(wideCharCount),
										  NULL, 0, NULL, NULL);
		if (utf8Len <= 0)
			return FALSE;
		
		outUtf8Text.resize(utf8Len);
		WideCharToMultiByte(CP_UTF8, 0, &wideBuffer[0], 
						   static_cast<int>(wideCharCount),
						   &outUtf8Text[0], utf8Len, NULL, NULL);
		
		SLOGI() << "Converted UTF-16 to UTF-8";
		return TRUE;
	}
	else if (codePage == CP_UTF8)
	{
		// 已经是UTF-8，直接使用（跳过BOM）
		outUtf8Text.assign(pData + dataOffset, dataSize - dataOffset);
		return TRUE;
	}
	else
	{
		// 无BOM，尝试UTF-8
		int wideLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
										  pData, static_cast<int>(dataSize),
										  NULL, 0);
		
		if (wideLen > 0)
		{
			// UTF-8有效
			std::vector<wchar_t> wideBuffer(wideLen);
			MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
							  pData, static_cast<int>(dataSize),
							  &wideBuffer[0], wideLen);
			
			// 验证转换后的内容是否合理（检查是否有替换字符）
			bool hasReplacementChar = false;
			for (int i = 0; i < wideLen; ++i)
			{
				if (wideBuffer[i] == 0xFFFD)  // Unicode replacement character
				{
					hasReplacementChar = true;
					break;
				}
			}
			
			if (!hasReplacementChar)
			{
				// UTF-8转换成功且无错误
				outUtf8Text.assign(pData, dataSize);
				SLOGI() << "Detected as valid UTF-8 without BOM";
				return TRUE;
			}
		}
		
		// UTF-8失败，使用ACP
		SLOGI() << "UTF-8 validation failed, using ANSI (ACP)";
		
		wideLen = MultiByteToWideChar(CP_ACP, 0, pData, static_cast<int>(dataSize), NULL, 0);
		if (wideLen <= 0)
			return FALSE;
		
		std::vector<wchar_t> wideBuffer(wideLen);
		MultiByteToWideChar(CP_ACP, 0, pData, static_cast<int>(dataSize),
						   &wideBuffer[0], wideLen);
		
		// WideChar -> UTF-8
		int utf8Len = WideCharToMultiByte(CP_UTF8, 0, &wideBuffer[0],
										  wideLen, NULL, 0, NULL, NULL);
		if (utf8Len <= 0)
			return FALSE;
		
		outUtf8Text.resize(utf8Len);
		WideCharToMultiByte(CP_UTF8, 0, &wideBuffer[0], wideLen,
						   &outUtf8Text[0], utf8Len, NULL, NULL);
		
		return TRUE;
	}
}

/**
 * @brief 从UTF-8文本中按行分割
 * @param utf8Text UTF-8编码的文本
 * @param outLines 输出的行列表（Unicode字符串）
 */
static void SplitUtf8TextToLines(const std::string& utf8Text, std::vector<SStringA>& outLines)
{
	outLines.clear();
	SStringA fullText(utf8Text.data());
	
	// 按行分割
	int lineStart = 0;
	for (int i = 0; i < fullText.GetLength(); ++i)
	{
		if (fullText[i] == '\n')
		{
			SStringA line = fullText.Mid(lineStart, i - lineStart);
			line.TrimRight('\r');
			if (!line.IsEmpty())
				outLines.push_back(line);
			lineStart = i + 1;
		}
	}
	// 最后一行
	if (lineStart < fullText.GetLength())
	{
		SStringA line = fullText.Mid(lineStart);
		if (!line.IsEmpty())
			outLines.push_back(line);
	}
}

/**
 * @brief 解析时间标签 [mm:ss.xx] 为毫秒
 * @param timeTag 时间标签字符串（不含方括号）
 * @return 时间（毫秒），失败返回-1
 */
int SLrcProvider::parseTimeTag(const SStringA& timeTag)
{
	// 格式: mm:ss.xx 或 mm:ss.xxx
	int minutes = 0;
	int seconds = 0;
	int milliseconds = 0;
	
	// 查找冒号和点号的位置
	int colonPos = timeTag.FindChar(':');
	int dotPos = timeTag.FindChar('.');
	
	if (colonPos < 0 || dotPos < 0 || dotPos <= colonPos)
		return -1;
	
	// 解析分钟
	SStringA strMin = timeTag.Mid(0, colonPos);
	minutes = atoi(strMin);
	
	// 解析秒
	SStringA strSec = timeTag.Mid(colonPos + 1, dotPos - colonPos - 1);
	seconds = atoi(strSec);
	
	// 解析毫秒部分
	SStringA strMs = timeTag.Mid(dotPos + 1);
	if (strMs.GetLength() == 2)
	{
		// xx -> xx0 (百分之一秒)
		milliseconds = atoi(strMs) * 10;
	}
	else if (strMs.GetLength() == 3)
	{
		// xxx (毫秒)
		milliseconds = atoi(strMs);
	}
	else
	{
		return -1;
	}
	
	return minutes * 60 * 1000 + seconds * 1000 + milliseconds;
}

struct TimedLine {
	int timeMs;
	SStringT text;
};
static bool TimedLineCmp(const TimedLine& a, const TimedLine& b) {
	return a.timeMs < b.timeMs;
}

/**
 * @brief 加载LRC歌词文件
 * @param pszLrc LRC文件路径
 * @param durationMs 歌曲总时长（毫秒）
 * @return 成功返回TRUE
 */
BOOL SLrcProvider::load(LPCTSTR pszLrc, int durationMs)
{
	m_lines.clear();
	m_strTitle.Empty();
	m_strArtist.Empty();
	m_strAlbum.Empty();
	m_durationMs = durationMs;
	
	// 步骤1: 读取文件并转换为UTF-8
	std::string utf8Text;
	if (!ConvertFileToUtf8(pszLrc, utf8Text))
	{
		SLOGW() << "Failed to read or convert LRC file: " << pszLrc;
		return FALSE;
	}
	
	// 步骤2: 将UTF-8文本分割为行
	std::vector<SStringA> fileLines;
	SplitUtf8TextToLines(utf8Text, fileLines);
	
	if (fileLines.empty())
	{
		SLOGW() << "Empty LRC file";
		return FALSE;
	}
	
	SLOGI() << "Successfully loaded " << fileLines.size() << " lines from LRC file";
	
	// 临时存储所有带时间的歌词行

	std::vector<TimedLine> timedLines;
	
	// 步骤3: 解析每一行（与原有逻辑保持一致）
	for (std::vector<SStringA>::iterator it=fileLines.begin();it!=fileLines.end();it++)
	{
		SStringA & strLine = *it;
		if (strLine.IsEmpty())
			continue;
		
		// 检查是否是元数据标签
		if (strLine.Left(4) == "[ti:")
		{
			int endPos = strLine.FindChar(']');
			if (endPos > 0)
				m_strTitle = S_CA2T(strLine.Mid(4, endPos - 4),CP_UTF8, CP_ACP);
		}
		else if (strLine.Left(4) == "[ar:")
		{
			int endPos = strLine.FindChar(_T(']'));
			if (endPos > 0)
				m_strArtist = S_CA2T(strLine.Mid(4, endPos - 4),CP_UTF8, CP_ACP);;
		}
		else if (strLine.Left(4) == "[al:")
		{
			int endPos = strLine.FindChar(_T(']'));
			if (endPos > 0)
				m_strAlbum = S_CA2T(strLine.Mid(4, endPos - 4),CP_UTF8, CP_ACP);;
		}
		else if (strLine[0] == _T('['))
		{
			// 解析时间标签和歌词文本
			int endBracket = strLine.FindChar(_T(']'));
			if (endBracket > 0)
			{
				SStringA timeTag = strLine.Mid(1, endBracket - 1);
				SStringA lyricText = strLine.Mid(endBracket + 1);
				lyricText.TrimLeft();
				
				int timeMs = parseTimeTag(timeTag);
				if (timeMs >= 0 && !lyricText.IsEmpty())
				{
					TimedLine tl;
					tl.timeMs = timeMs;
					tl.text = S_CA2T(lyricText,CP_UTF8,CP_ACP);
					timedLines.push_back(tl);
				}
			}
		}
	}
	
	if (timedLines.empty())
	{
		SLOGW() << "No valid lyrics found in LRC file";
		return FALSE;
	}
	
	// 按时间排序（有些LRC文件可能不是严格排序的）
	std::sort(timedLines.begin(), timedLines.end(),TimedLineCmp);
	
	// 构建歌词行，计算每行的结束时间
	for (size_t i = 0; i < timedLines.size(); ++i)
	{
		LrcLine line;
		line.tsBeginMs = timedLines[i].timeMs;
		line.strText = timedLines[i].text;
		
		// 下一行的开始时间作为当前行的结束时间
		if (i + 1 < timedLines.size())
		{
			line.tsEndMs = timedLines[i + 1].timeMs;
		}
		else
		{
			// 最后一行使用歌曲总时长
			line.tsEndMs = m_durationMs > 0 ? m_durationMs : (line.tsBeginMs + 5000);
		}
		
		m_lines.push_back(line);
	}
	
	// 在第一行之前插入标题行
	if (!m_lines.empty())
	{
		LrcLine headLine;
		headLine.tsBeginMs = 0;
		headLine.tsEndMs = m_lines.front().tsBeginMs;
		headLine.strText = m_strTitle;
		if (!m_strArtist.IsEmpty())
		{
			headLine.strText += _T("\n") + m_strArtist;
		}
		m_lines.insert(m_lines.begin(), headLine);
	}
	
	// 保存歌词文件路径
	m_strLrcPath = pszLrc;
		
	SLOGI() << "Loaded LRC: " << m_lines.size() << " lines, title=" 
	               << m_strTitle.c_str() << ", artist=" << m_strArtist.c_str();
	
	return !m_lines.empty();
}

/**
 * @brief 根据时间获取行索引（支持小数表示行内进度）
 */
float SLrcProvider::getLineIndexFromTimeMs(int timeMs) const
{
	if (m_lines.empty())
		return 0.0f;
	
	// 应用偏移量：实际查找时间 = 播放时间 - 偏移量
	int adjustedTimeMs = timeMs - m_nOffsetMs;
	
	int lineIndex = findLineByTime(adjustedTimeMs);
	
	if (lineIndex < 0 || lineIndex >= (int)m_lines.size())
		return 0.0f;
	
	const LrcLine& line = m_lines[lineIndex];
	
	// 如果是最后一行
	if (lineIndex == (int)m_lines.size() - 1)
	{
		int duration = line.tsEndMs - line.tsBeginMs;
		if (duration <= 0)
			return (float)lineIndex;
		
		int fraction = adjustedTimeMs - line.tsBeginMs;
		return lineIndex + (float)fraction / duration;
	}
	else
	{
		// 使用下一行的开始时间计算进度
		const LrcLine& nextLine = m_lines[lineIndex + 1];
		if (adjustedTimeMs < line.tsBeginMs)
			return (float)lineIndex;
		
		int duration = nextLine.tsBeginMs - line.tsBeginMs;
		if (duration <= 0)
			return (float)lineIndex;
		
		int fraction = adjustedTimeMs - line.tsBeginMs;
		return lineIndex + (float)fraction / duration;
	}
}

/**
 * @brief 获取行内单词进度比例（LRC不支持逐字，返回0或1）
 */
float SLrcProvider::getWordRatio(int iLine, int timeMs) const
{
	if (iLine < 0 || iLine >= (int)m_lines.size())
		return 0.0f;
	
	// 应用偏移量
	int adjustedTimeMs = timeMs - m_nOffsetMs;
	
	const LrcLine& line = m_lines[iLine];
	
	if (adjustedTimeMs <= line.tsBeginMs)
		return 0.0f;
	
	if (adjustedTimeMs >= line.tsEndMs)
		return 1.0f;
	
	// LRC格式不支持逐字高亮，简单线性插值
	float ratio = (float)(adjustedTimeMs - line.tsBeginMs) / (line.tsEndMs - line.tsBeginMs);
	return smin(1.0f, smax(0.0f, ratio));
}

/**
 * @brief 获取歌词行数
 */
int SLrcProvider::getLineCount() const
{
	return m_lines.size();
}

/**
 * @brief 获取指定行的歌词文本
 */
SStringT SLrcProvider::getLineText(int iLine) const
{
	if (iLine < 0 || iLine >= (int)m_lines.size())
		return _T("");
	
	return m_lines[iLine].strText;
}

/**
 * @brief 获取歌词总时长
 */
int SLrcProvider::getTsEndMs() const
{
	if (m_lines.empty())
		return 0;
	
	return m_lines.back().tsEndMs;
}

/**
 * @brief 判断是否为标题行
 */
bool SLrcProvider::isHeadLine(int iLine) const
{
	return iLine == 0;  // 第一行是标题行
}

/**
 * @brief 二分查找时间对应的行索引
 */
int SLrcProvider::findLineByTime(int timeMs) const
{
	if (m_lines.empty())
		return -1;
	
	int left = 0;
	int right = m_lines.size() - 1;
	
	// 如果时间在第一个之前
	if (timeMs < m_lines[0].tsBeginMs)
		return 0;
	
	// 如果时间在最后一个之后
	if (timeMs >= m_lines.back().tsBeginMs)
		return m_lines.size() - 1;
	
	// 二分查找
	while (left <= right)
	{
		int mid = (left + right) / 2;
		
		if (timeMs >= m_lines[mid].tsBeginMs && 
		    timeMs < m_lines[mid].tsEndMs)
		{
			return mid;
		}
		else if (timeMs < m_lines[mid].tsBeginMs)
		{
			right = mid - 1;
		}
		else
		{
			left = mid + 1;
		}
	}
	
	return left < (int)m_lines.size() ? left : m_lines.size() - 1;
}

// 私有方法：将偏移应用到 LRC 文件并保存
void SLrcProvider::applyOffsetToFile()
{
	if (m_strLrcPath.IsEmpty() || m_nOffsetMs == 0)
		return;
	
	SLOGI() << "Applying offset " << m_nOffsetMs << "ms to LRC file: " << m_strLrcPath;
	
	// 读取原始 LRC 文件内容
	FILE* fpRead = _tfopen(m_strLrcPath, _T("r"));
	if (!fpRead)
	{
		SLOGW() << "Failed to open LRC file for reading: " << m_strLrcPath;
		return;
	}
	
	// 逐行读取并处理
	std::vector<SStringA> lines;
	char line[2048];
	while (fgets(line, sizeof(line), fpRead))
	{
		SStringA strLine(line);
		strLine.TrimRight();
		
		// 跳过已有的 offset 标签（如果存在）
		if (strLine.Left(8) == "[offset:")
			continue;
		
		// 处理时间标签行
		if (strLine[0] == '[' && strLine.GetLength() > 9)
		{
			int endBracket = strLine.FindChar(']', 1);
			if (endBracket > 0)
			{
				SStringA timeTag = strLine.Mid(1, endBracket - 1);
				
				// 检查是否是时间标签格式 [mm:ss.xx]
				if (timeTag.GetLength() >= 8 && timeTag[2] == ':' && timeTag[5] == '.')
				{
					// 解析原时间
					int minutes = atoi(timeTag.Mid(0, 2));
					int seconds = atoi(timeTag.Mid(3, 2));
					int centiseconds = atoi(timeTag.Mid(6, 2));
					
					// 转换为毫秒并应用偏移
					int originalMs = minutes * 60000 + seconds * 1000 + centiseconds * 10;
					int adjustedMs = originalMs + m_nOffsetMs;
					
					// 确保时间不为负
					if (adjustedMs < 0)
						adjustedMs = 0;
					
					// 转换回 [mm:ss.xx] 格式
					int newMinutes = adjustedMs / 60000;
					int newSeconds = (adjustedMs % 60000) / 1000;
					int newCentiseconds = (adjustedMs % 1000) / 10;
					
					SStringA newTimeTag;
					newTimeTag.Format("[%02d:%02d.%02d]", newMinutes, newSeconds, newCentiseconds);
					
					// 替换时间标签
					strLine = newTimeTag + strLine.Mid(endBracket + 1);
				}
			}
		}
		
		lines.push_back(strLine);
	}
	fclose(fpRead);
	
	// 写回文件
	FILE* fpWrite = _tfopen(m_strLrcPath, _T("w"));
	if (!fpWrite)
	{
		SLOGW() << "Failed to open LRC file for writing: " << m_strLrcPath;
		return;
	}
	
	for (size_t i = 0; i < lines.size(); ++i)
	{
		fprintf(fpWrite, "%s\n", lines[i].c_str());
	}
	fclose(fpWrite);
	
	SLOGI() << "Successfully saved LRC file with offset applied";
}

///////////////////////////////////////////////////////////////////////
SLrcView::SLrcView()
	: m_lineHei(50)
	, m_crText(RGBA(180,180,180,255))
	, m_crHighlight(RGBA(255,255,255,255))
	, m_nFontSizeNormal(16)
	, m_nFontSizeCurrent(24)
	, m_fScaleDuration(0.3f)
{
	m_animator.Attach((SIntAnimator*)SApplication::getSingleton().CreateValueAnimatorByName(SIntAnimator::GetClassName()));
	m_animator->addUpdateListener(this);
}

void SLrcView::SetLrc(ILrcProvider* pProvider)
{
	m_provider = pProvider;
	m_nLength = pProvider?pProvider->getTsEndMs():0;
	m_timeMs = 0;
	m_timeAniMs = 0;
	Invalidate();
}

BOOL SLrcView::SetTimeMs(int timeMs)
{
	if(!m_provider)
		return FALSE;
	if (m_animator->isRunning())
		return FALSE;
	if (timeMs < 0)
		timeMs = 0;
	else if(timeMs > m_nLength)
		timeMs = m_nLength;
	float lineNow = m_provider->getLineIndexFromTimeMs(m_timeAniMs);
	float lineNew = m_provider->getLineIndexFromTimeMs(timeMs);
	if (floorf(lineNow) != floorf(lineNew)) {
		//different line
		m_animator->setRange(m_timeAniMs, timeMs);
		m_animator->start(GetContainer());
	}
	else{
		//update same line progress
		m_timeMs = timeMs;
		Invalidate();
	}
	return TRUE;
}

void SLrcView::onAnimationUpdate(IValueAnimator* pAnimator)
{
	m_timeMs = m_timeAniMs = m_animator->getValue();
	Invalidate();
}

void SLrcView::drawLine(IRenderTarget* pRT, CRect rc, int iLine, float fProgress, bool bIsCurrent)
{
	if (iLine < 0 || iLine >= m_provider->getLineCount())
		return;
	
	SStringT strText = m_provider->getLineText(iLine);
	if (strText.IsEmpty())
		return;
	
	bool bHead = m_provider->isHeadLine(iLine);
	
	// 计算透明度和字号（仅当前行使用大字号）
	int nFontSize = bIsCurrent ? m_nFontSizeCurrent : m_nFontSizeNormal;
	BYTE byAlpha = 255;
	
	if (!bIsCurrent)
	{
		// 非当前行：根据距离计算透明度（保持字号不变）
		float fDistance = fabsf((float)iLine - fProgress);
		if (fDistance <= 1.0f)
		{
			// 相邻行：透明度 40%-100%
			byAlpha = (BYTE)(255 * (0.4f + 0.6f * (1.0f - fDistance)));
		}
		else
		{
			// 远处行：固定较低透明度
			byAlpha = (BYTE)(255 * 0.3f);
		}
	}
	
	// 使用 SOUI 字体 API 设置字号
	SAutoRefPtr<IFontS> pOldFont;
	{
		SAutoRefPtr<IFontS> pCurFont = (IFontS*)pRT->GetCurrentObject(OT_FONT);
		SASSERT(pCurFont);
		const LOGFONT *plf = pCurFont->LogFont();
		LOGFONT lf;
		memcpy(&lf, plf, sizeof(LOGFONT));
		lf.lfHeight = -nFontSize;
		
		SAutoRefPtr<IFontS> pNewFont;
		GETRENDERFACTORY->CreateFont(&pNewFont, &lf);
		pRT->SelectObject(pNewFont, (IRenderObj**)&pOldFont);
	}
	
	// 应用透明度到颜色
	COLORREF crText = bIsCurrent ? m_crHighlight : m_crText;
	BYTE r = GetRValue(crText);
	BYTE g = GetGValue(crText);
	BYTE b = GetBValue(crText);
	COLORREF crFinal = RGBA(r, g, b, byAlpha);
	
	pRT->SetTextColor(crFinal);
	// 计算文本实际尺寸
	CRect rcMeasure = rc;
	pRT->DrawText(strText, strText.GetLength(), &rcMeasure, DT_CALCRECT|DT_WORDBREAK);
	// 调整垂直位置以保持居中
	int nOffsetY = (rc.Height() - rcMeasure.Height()) / 2;
	int nOffsetX = 0;
	switch(GetStyle().GetAlign()&SwndStyle::Align_MaskX){
		case SwndStyle::Align_Center:
		nOffsetX = (rc.Width() - rcMeasure.Width()) / 2;
		break;
		case SwndStyle::Align_Right:
		nOffsetX = rc.Width() - rcMeasure.Width();
		break;
	}
	CRect rcText = rc;
	rcText.OffsetRect(nOffsetX, nOffsetY);
	rcText.right = rcText.left + rcMeasure.Width();
	rcText.bottom = rcText.top + rcMeasure.Height();

	// 如果是当前行且正在逐字高亮
	if (bIsCurrent && !bHead && !m_animator->isRunning())
	{
		float ratio = m_provider->getWordRatio(iLine, m_timeMs);
		int width = rcMeasure.Width() * ratio;
		
		// 绘制已唱部分（高亮色）
		CRect rcHighlighted = rcText;
		rcHighlighted.right = rcHighlighted.left + width;
		pRT->PushClipRect(rcHighlighted, RGN_AND);
		pRT->SetTextColor(m_crHighlight);
		pRT->DrawText(strText, strText.GetLength(), &rcText, DT_WORDBREAK);
		pRT->PopClip();
		
		// 绘制未唱部分（普通色带透明度）
		CRect rcUnhighlighted = rcText;
		rcUnhighlighted.left = rcHighlighted.right;
		rcUnhighlighted.right = rcText.right;
		pRT->PushClipRect(rcUnhighlighted, RGN_AND);
        COLORREF crUnhighlighted = RGBA(r, g, b, byAlpha / 2); // 未唱部分更淡
        pRT->SetTextColor(crUnhighlighted);
		pRT->DrawText(strText, strText.GetLength(), &rcText, DT_WORDBREAK);
		pRT->PopClip();
	}
	else
	{
		// 普通绘制
		pRT->DrawText(strText, strText.GetLength(), &rcText, DT_WORDBREAK);
	}
	
	// 恢复旧字体
	if (pOldFont)
	{
		pRT->SelectObject(pOldFont, NULL);
	}
}

void SLrcView::OnPaint(IRenderTarget* pRT)
{
	SPainter painter;
	BeforePaint(pRT, painter);
	
	CRect rc = GetClientRect();
	if (m_provider && m_provider->getLineCount() > 0)
	{
		int nLines = m_provider->getLineCount();
		int nLineHei = m_lineHei.toPixelSize(GetScale());
		
		// 获取当前播放进度（行索引，可能带小数）
		float fCurrentProgress = m_provider->getLineIndexFromTimeMs(
			m_animator->isRunning() ? m_timeAniMs : m_timeMs);
		
		// 计算可见行数
		int visibleLines = rc.Height() / nLineHei + 2;  // 多显示几行以确保平滑滚动
		
		// 计算第一行和最后一行的索引，确保当前行始终在视图正中间
		// 关键：使用 fCurrentProgress 作为中心点，上下各扩展 visibleLines/2
		float fFirstLine = fCurrentProgress - (float)visibleLines / 2.0f;
		float fLastLine = fCurrentProgress + (float)visibleLines / 2.0f;
		
		int iFirstLine = (int)floorf(fFirstLine);
		int iLastLine = (int)ceilf(fLastLine);
		
		// 边界检查
		iFirstLine = smax(0, iFirstLine);
		iLastLine = smin(nLines - 1, iLastLine);
		
		// 计算垂直偏移以实现平滑滚动
		// 关键：让当前行的精确位置（包括小数部分）对齐到视图中心
		float fCenterOffset = fCurrentProgress - (float)iFirstLine;
		int nCenterPixel = (int)(fCenterOffset * nLineHei);
		int nViewCenter = rc.Height() / 2;
		int nOffset = nCenterPixel - nViewCenter;
		
		// 绘制每一行
		CRect rcLine(rc.left, rc.top, rc.right, rc.top + nLineHei);
		rcLine.OffsetRect(0, -nOffset);
		
		for (int i = iFirstLine; i <= iLastLine; i++)
		{
			// 判断是否是当前播放行
			bool bIsCurrent = (i == (int)fCurrentProgress && !m_animator->isRunning());
			
			drawLine(pRT, rcLine, i, fCurrentProgress, bIsCurrent);
			
			rcLine.OffsetRect(0, nLineHei);
		}
	}
	else
	{
		// 无歌词时显示提示
		pRT->SetTextColor(m_crText);
		pRT->DrawText(_T("No Lyrics"), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	
	AfterPaint(pRT, painter);
}

SNSEND
