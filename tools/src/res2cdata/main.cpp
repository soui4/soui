#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <string>
#include "XGetopt.h"
#include <Windows.h>

using namespace std;
#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

bool convertResFile2CData(const tstring & filename,FILE *output){
	FILE *fin = _tfopen(filename.c_str(),_T("rb"));
	if(!fin)
		return false;

	TCHAR szTitle[MAX_PATH],szExt[30]={0};
	_tsplitpath(filename.c_str(),NULL,NULL,szTitle,szExt);
	tstring strVar = tstring(szTitle)+_T('_')+(szExt+1);
#ifdef _UNICODE
	char szVar[MAX_PATH];
	WideCharToMultiByte(CP_ACP,0,strVar.c_str(),-1,szVar,MAX_PATH,NULL,NULL);
	fprintf(output,"static const unsigned char  %s[]={\n",szVar);
#else
	fprintf(output,"static const unsigned char  %s[]={\n",strVar.c_str());
#endif
	unsigned char buf[16];
	bool bFirst = true;
	for(;;){
		int nReaded = fread(buf ,1,16,fin);
		if(!nReaded)
			break;
		int i=0;
		if(bFirst){
			fprintf(output,"0x%02x",buf[i]);
			i++;
			bFirst = false;
		}
		for(;i<nReaded;i++){
			fprintf(output,",0x%02x",buf[i]);
		}
		fprintf(output,"\n");
	}	
	fclose(fin);
	fprintf(output,"};\n\n");
	return true;
}

int _tmain(int argc, _TCHAR* argv[]){

	int c;
	tstring strInput;	//皮肤路径,相对于程序的.rc文件
	tstring strOutput;
	tstring strFilter = _T("*.*");
	while ((c = getopt(argc, argv, _T("i:o:f:"))) != EOF || optarg!=NULL)
	{
		switch (c)
		{
		case 'i':strInput=optarg;break;
		case 'o':strOutput=optarg;break;
		case 'f':strFilter = optarg;break;
		}
	}
	
	if(strInput.empty() || strOutput.empty()){
		printf("usage: res2cdata.exe -i input -o output.c\n");
		return 1;
	}
	if(*strInput.rbegin() !=_T('\\'))
		strInput.push_back(_T('\\'));
	_tprintf(_T("input=%s,output=%s,filter=%s\n"),strInput.c_str(),strOutput.c_str(),strFilter.c_str());
	FILE *fout = _tfopen(strOutput.c_str(),_T("w"));
	if(!fout){
		_tprintf(_T("open output file failed! output = %s\n"),strOutput.c_str());
		return 2;
	}
//	fprintf(fout,"#include <stdio.h>\n");

	WIN32_FIND_DATA wfd;
	tstring filename = strInput+strFilter;
	HANDLE fd = FindFirstFile(filename.c_str(),&wfd);
	if(fd != INVALID_HANDLE_VALUE){
		BOOL next = TRUE;
		do{
			if(!(wfd.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN)))
			{
				_tprintf(_T("find file: %s\n"),wfd.cFileName);
				tstring filename = strInput + wfd.cFileName;
				bool bConvert = convertResFile2CData(filename,fout);
				if(!bConvert){
					_tprintf(_T("convert file: %s failed\n"),wfd.cFileName);
				}
			}
			next = FindNextFile(fd,&wfd);
		}while(next);
		FindClose(fd);
	}
	fclose(fout);

	return 0;
}