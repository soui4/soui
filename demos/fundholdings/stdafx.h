// stdafx.h : 预编译头（与 demos/uieditor 保持一致）
//
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define DLL_SOUI

// ---- 系统 / 标准库（公共、稳定） ----
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>

// ---- SOUI 框架（公共、稳定） ----
#include <souistd.h>
#include <SouiFactory.h>
#include <com-cfg.h>
#include <commgr2.h>
#include <core/SPanel.h>
#include <control/SEdit.h>
#include <control/SRichEdit.h>
#include <event/SNotifyCenter.h>
#include <helper/SDpiHelper.hpp>
#include <helper/SFunctor.hpp>
#include <helper/slog.h>
#include <interface/SHttpClient-i.h>
#include <interface/STaskLoop-i.h>
#include <string/sstringa.h>

// ---- 工程公共基础设施 ----
#include "compat.h"
#include "events.h"
#include "resource.h"
#include "res/resource.h"

using namespace SOUI;
