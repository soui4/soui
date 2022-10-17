#pragma once


#ifdef __cplusplus
#define SCONST const
#define SNSBEGIN	namespace SOUI{
#define SNSEND		}//end of namespace SOUI
#define OVERRIDE override
#define CTHIS_  THIS_
#define CTHIS  THIS
#else
#define SCONST
#define SNSBEGIN
#define SNSEND
#define OVERRIDE
#define CTHIS_ const THIS_
#define CTHIS const THIS
#endif

