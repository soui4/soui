// stdafx.h : Android 版本，替代 Windows 版本
// 移除了 <windows.h> 和 <tchar.h>，SOUI 的 souistd.h 已提供必要的兼容类型
#pragma once

#define WIN32_LEAN_AND_MEAN

#include <souistd.h>
#include <core/SHostWnd.h>
#include <core/SWnd.h>
#include <core/SHostDialog.h>
#include <helper/SMenuEx.h>
#include <gif/SSkinAni.h>

using namespace SOUI;
