//#pragma once
#ifndef COMMON_DEFS
#define COMMON_DEFS
#include <sdef.h>

SNSBEGIN

#define LOG_INFO LOG_LEVEL_INFO
#define LOG_DEBUG LOG_LEVEL_DEBUG
#define LOG_WARN LOG_LEVEL_WARN
#define LOG_ERROR LOG_LEVEL_ERROR
#define LOG_FATAL LOG_LEVEL_FATAL

static const int RETURN_OK = 0;
static const int RETURN_TIMEOUT = 1;
static const int RETURN_ERROR = -1;

SNSEND

#endif // COMMON_DEFS

