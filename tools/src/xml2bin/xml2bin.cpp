// dui-demo.cpp : main source file
//

#include "stdafx.h"
using namespace pugi;
using namespace SNS;

void convertDir(LPCTSTR pszDir,bool toBin)
{
	WIN32_FIND_DATA wfd;
	TCHAR szFilter[MAX_PATH];
	_stprintf(szFilter,_T("%s\\*.*"),pszDir);
	HANDLE hFind = FindFirstFile(szFilter,&wfd);
	if(hFind)
	{
		do 
		{
			SOUI::SStringT strName(wfd.cFileName);
			strName.MakeLower();
			SOUI::SStringT strPath=SOUI::SStringT().Format(_T("%s\\%s"),pszDir,wfd.cFileName);
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(strName != _T(".") && strName!=_T("..") )
				{
					convertDir(strPath,toBin);
				}
			}else if(strName.EndsWith(_T("xml")) || strName==_T("uires.idx"))
			{
				pugi::xml_document xmlDoc;
				if(xmlDoc.load_file(strPath))
				{
					bool bOk=xmlDoc.save_file(strPath,L"\t",0,toBin?encoding_bin:encoding_utf8);
					_tprintf(_T("convert %s to %s return %s!\n"),strPath.c_str(),toBin?_T("bin"):_T("xml"),bOk?_T("succeed"):_T("failed"));
				}else
				{
					_tprintf(_T("load %s failed!\n"),strPath.c_str());
				}
			}

		} while (FindNextFile(hFind,&wfd));
		FindClose(hFind);
	}
}

int _tmain(int argc,TCHAR **argv)
{
	if(argc <2)
	{
		_tprintf(_T("usage: xml2bin destFolder [tobin|toxml]\n"));
	}else
	{
		bool bCvt2Bin = true;

		if(argc==3)
		{
			SOUI::SStringT str(argv[2]);
			bCvt2Bin = str.CompareNoCase(_T("toxml"))!=0;
		}
		convertDir(argv[1],bCvt2Bin);
	}
	return 0;
}

#ifndef _WIN32
int main(int argc, char* argv[])
{
	return _tmain(argc, argv); 
}
#endif//_WIN32
