#pragma once
#include "apppublic.h"
#include <map>

class CGlobalUnits
{
public:
	CGlobalUnits(void);
	~CGlobalUnits(void);

public:
	static CGlobalUnits* instance();

public:
	std::string m_strTest;

	//id-name
	std::map<std::string, std::string>			m_mapEmojisIndex;
	std::map<std::string, IBitmap*>				m_mapFace;

	SStringT m_sstrAppPath;
	SStringT m_sstrEmojiFolder;

	PersonalsMap	m_mapPersonals;
	GroupsMap		m_mapGroups;
	GroupMembersMap	m_mapGrpmbrs;
	GHsMap			m_mapGhs;

	std::multimap<SStringT, SStringT> m_PinyinMap;

	//实际项目中只需根据通讯录构建一个就可以了，在demo中添加数据时随意添加的就分开定义了
	std::multimap<SStringT, SEARCH_INFO> m_mapMessagePinyinSearch;
    std::multimap<SStringT, SEARCH_INFO> m_mapMessageChineseSearch;

    std::multimap<SStringT, SEARCH_INFO> m_mapContactPinyinSearch;
    std::multimap<SStringT, SEARCH_INFO> m_mapContactChineseSearch;

  public:
	void SetEmojiPath(const SStringT& emojiPath);

	std::string GenerateUUID();
	void OperateEmojis();

    void OperatePinyinMap();
    SStringT EncodeChinese(const SStringT& sstrSrc);
    SStringT DecodeChinese(const SStringT& sstrSrc);
    bool GetSimpleAndFull(SStringT& sstrSrc, SStringT& sstrSimple, SStringT& sstrFull);
    bool IsIncludeChinese(SStringT &sstrSrc);

	void GenerateShamDate();		//处理假数据
};

#define GlobalUnits  CGlobalUnits::instance();