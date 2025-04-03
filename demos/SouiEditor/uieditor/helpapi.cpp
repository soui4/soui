#include "stdafx.h"
#include "helpapi.h"

bool FileIsExist(const SStringT &filepath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(filepath, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		return false;
	}
	else {
		FindClose(hFind);
		return true;
	}
}

SStringT GetFileExtname(const SStringT& filepath)
{
	int pos = filepath.ReverseFind(_T('.'));
	if(pos==-1)
		return _T("");
	else{
		if(pos>0 && filepath[pos-1]==_T(SLASH))
			return _T("");
		return filepath.Right(filepath.GetLength()-pos-1);
	}
}

SStringT GetFilename(const SStringT& filepath)
{
	#ifdef _WIN32
	TCHAR szName[MAX_PATH] = { 0 };
	_tsplitpath(filepath, NULL, NULL, szName, NULL);
	return szName;
	#else
	int pos1 = filepath.ReverseFind(_T('/'));
	if(pos1==-1){
		return _T("");
	}
	int pos2 = filepath.ReverseFind(_T('.'));
	if(pos2 < pos1) pos2 = filepath.GetLength();
	return filepath.Mid(pos1+1,pos2-pos1-1);
	#endif//_WIN32
}

//自定义排序函数  
bool SortSString(const SStringT &v1, const SStringT &v2)
{
	return v1 < v2;		//升序排列  
}

bool SortSStringNoCase(const SStringT &v1, const SStringT &v2)
{
	return v1.CompareNoCase(v2) < 0;		//升序排列  
}

bool SortSStringNoCaseW(const SStringW &v1, const SStringW &v2)
{
	return v1.CompareNoCase(v2) < 0;		//升序排列  
}