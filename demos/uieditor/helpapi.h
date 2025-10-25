#ifndef _FILE_HELPER_API_H_
#define _FILE_HELPER_API_H_

#ifdef _WIN32
#define SLASH '\\'
#else
#define SLASH '/'
#endif//_WIN32

bool FileIsExist(const SStringT &filepath);
SStringT GetFileExtname(const SStringT& filepath);
SStringT GetFilename(const SStringT& filepath);

bool SortSString(const SStringT &v1, const SStringT &v2);
bool SortSStringNoCase(const SStringT &v1, const SStringT &v2);
bool SortSStringNoCaseW(const SStringW &v1, const SStringW &v2);

#endif//_FILE_HELPER_API_H_