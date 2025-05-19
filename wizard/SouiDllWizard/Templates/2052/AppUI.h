﻿#pragma once


#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/SouiCtrls.h>
#include <res.mgr/SObjDefAttr.h>
#include <com-cfg.h>
#include <event/SNotifyCenter.h>
#include "resource.h"
#define R_IN_CPP	//定义这个开关来
#include "res\resource.h"
using namespace SNS;


//////////////////////////////////////////////////////////////////////////
class UI_Engine
{
public:
	UI_Engine();
	~UI_Engine();

	static DWORD WINAPI RunUI(LPVOID lpParam);
	static int Init_UI(HMODULE hInstance, HWND parent);
public:
	static HMODULE m_hInstance;
	static HWND m_hParent;
};

extern UI_Engine g_UIEngine;