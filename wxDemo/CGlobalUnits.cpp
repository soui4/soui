#include "stdafx.h"
#include "CGlobalUnits.h"


CGlobalUnits::CGlobalUnits(void)
{
}

CGlobalUnits::~CGlobalUnits(void)
{
}


CGlobalUnits* CGlobalUnits::instance()
{
	static CGlobalUnits _Instance;
	return &_Instance;
}