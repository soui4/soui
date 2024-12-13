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

	SStringT m_sstrEmojiFolder;

	PersonalsMap	m_mapPersonals;
	GroupsMap		m_mapGroups;
	GroupMembersMap	m_mapGrpmbrs;
	GHsMap			m_mapGhs;
public:
	void SetEmojiPath(const SStringT& emojiPath);

	std::string GenerateUUID();
	void OperateEmojis();

	void GenerateShamDate();		//处理假数据
};

#define GlobalUnits  CGlobalUnits::instance();