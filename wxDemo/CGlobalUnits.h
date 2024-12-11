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
public:
	std::string GenerateUUID();
	void OperateEmojis();
};

#define GlobalUnits  CGlobalUnits::instance();