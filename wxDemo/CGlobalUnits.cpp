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

std::string CGlobalUnits::GenerateUUID()
{
	char szbuf[100];
	GUID guid;
	::CoCreateGuid(&guid);
	sprintf(szbuf,
		"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid.Data1,
		guid.Data2,
		guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	std::string strUUID = szbuf;
	return strUUID;
}

void CGlobalUnits::OperateEmojis()
{
	/*	2024-12-11	yangjinpeng
	*	处理本地目录下的emoji图片
	*/
	std::string strPath = "emojis\\*.png";
	SStringT sstrPath = S_CA2T(strPath.c_str());

	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	hFind = FindFirstFile(sstrPath, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("No files found.\n");
		return;
	}
	else {
		do {
			SStringT sstrName = findFileData.cFileName;
			SStringA tmp = S_CT2A(sstrName);
			std::string strName(tmp.c_str(), tmp.GetLength());
			std::string strUUID = GenerateUUID();
			m_mapEmojisIndex.insert(std::make_pair(strUUID, strName));

			std::string strTempPath;
			strTempPath.append("emojis\\");
			strTempPath.append(strName);
			SStringT sstrPath = S_CA2T(strTempPath.c_str());
			IBitmap* pRes = SResLoadFromFile::LoadImage(sstrPath);
			if (pRes)
				m_mapFace.insert(std::make_pair(strUUID, pRes));
		} while (FindNextFile(hFind, &findFileData) != 0);
		FindClose(hFind);
	}
}