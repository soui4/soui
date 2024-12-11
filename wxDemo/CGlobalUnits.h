#pragma once
#include "apppublic.h"

class CGlobalUnits
{
public:
	CGlobalUnits(void);
	~CGlobalUnits(void);

public:
	static CGlobalUnits* instance();

public:
	std::string m_strTest;
};

#define GlobalUnits  CGlobalUnits::instance();