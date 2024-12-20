#include "stdafx.h"
#include "CGlobalUnits.h"
#include <sstream>

CGlobalUnits::CGlobalUnits(void)
{
	GenerateShamDate();
}

CGlobalUnits::~CGlobalUnits(void)
{
}


CGlobalUnits* CGlobalUnits::instance()
{
	static CGlobalUnits _Instance;
	return &_Instance;
}

void CGlobalUnits::SetEmojiPath(const SStringT& emojiPath) {
	m_sstrEmojiFolder = emojiPath;
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
	SStringT sstrPath = m_sstrEmojiFolder + _T("*.png");

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

			SStringT strPath = m_sstrEmojiFolder + sstrName;
			IBitmap* pRes = SResLoadFromFile::LoadImage(strPath);
			if (pRes)
				m_mapFace.insert(std::make_pair(strUUID, pRes));
		} while (FindNextFile(hFind, &findFileData) != 0);
		FindClose(hFind);
	}
}

void CGlobalUnits::OperatePinyinMap()
{
	//构建汉字关联表
    m_PinyinMap.clear();
    SStringT sstrPath = m_sstrAppPath + _T("default_res\\pymap.txt");

#ifdef _WIN32
    std::string strPath = S_CT2A(sstrPath);
    std::ifstream fin(strPath.c_str(), std::ios::in);
    char line[1024] = { 0 };
	while (fin.getline(line, sizeof(line)))
	{
        std::string strLine = line;
        std::string strKey, strValue;
        std::string::size_type pos;
        pos = strLine.find_first_of(' ');
        if (std::string::npos != pos)
        {
            strKey = strLine.substr(0, pos);
            strValue = strLine.substr(pos + 1);
            m_PinyinMap.insert(std::make_pair(S_CA2T(strKey.c_str()), S_CA2T(strValue.c_str())));
        }
	}
#endif
}

SStringT CGlobalUnits::EncodeChinese(const SStringT &sstrSrc)
{
    SStringT sstrTemp, sstrRet;
    int cur;
	for (int i = 0; i < sstrSrc.GetLength(); i++)
	{
        cur = sstrSrc.GetAt(i);
        sstrTemp.Format(_T("%04X"), cur);
        sstrRet = sstrRet + sstrTemp;
	}

    return sstrRet;
}

SStringT CGlobalUnits::DecodeChinese(const SStringT &sstrSrc)
{
    SStringT sstrTemp, sstrRet;
    for (int i = 0; i < sstrSrc.GetLength(); i += 4)
    {
        SStringT sstr = sstrSrc.Mid(i, 4);
        TCHAR *str1 = sstr.GetBuffer(sstr.GetLength() + 2);
        int x = (int)_tcstol(sstr, &str1, 16); //十六进制
        sstrTemp.Format(_T("%c"), x);
        sstrRet = sstrRet + sstrTemp;
    }
    return sstrRet;
}

bool CGlobalUnits::GetSimpleAndFull(SStringT &sstrSrc, SStringT &sstrSimple, SStringT &sstrFull)
{
	//获取简拼跟全拼
    if (sstrSrc.IsEmpty())
        return false;
	
	SStringT sstrTemp;
	for (int i = 0; i < sstrSrc.GetLength(); i += 4)
    {
        SStringT sstr = sstrSrc.Mid(i, 4);
		std::multimap<SStringT, SStringT>::iterator iter = m_PinyinMap.find(sstr);
        if (iter != m_PinyinMap.end())
            sstrTemp = iter->second.c_str();

        sstrFull = sstrFull + sstrTemp;
        sstrSimple = sstrSimple + sstrTemp.Left(1);
    }
 
	return true;
}

bool CGlobalUnits::IsIncludeChinese(SStringT &sstrSrc)
{
    std::string strSrc = S_CT2A(sstrSrc).c_str();
    char *str = (char *)strSrc.c_str();
    char c;
    while (1)
    {
        c = *str++;
        if (c == 0)
            break;    //如果到字符串尾则说明该字符串没有中文字符
        if (c & 0x80) //如果字符高位为1且下一字符高位也是1则有中文字符
            if (*str & 0x80)
                return true;
    }
    return false;
}

void CGlobalUnits::GenerateShamDate()
{
	const char* shamAreas[] = {
		"湖南",
		"湖北",
		"广东",
		"广西壮族自治区",
		"河南",
		"河北",
		"山东",
		"山西",
		"北京市",
		"重庆市",
		"天津市",
		"上海市",
		"江苏",
		"江西",
		"黑龙江",
		"浙江",
		"新疆维吾尔自治区",
		"宁夏回族自治区",
		"辽宁",
		"青海",
		"陕西",
		"甘肃",
		"云南",
		"贵州",
		"西藏自治区",
		"四川",
		"内蒙古自治区",
		"台湾",
		"海南",
		"福建",
		"吉林",
		"安徽",
		"香港特别行政区",
		"澳门特别行政区",
	};

	const char* shamNames[] = {
		"张三",
		"李四",
		"王五",
		"赵六",
		"三英战吕布",
		"三打白骨精",
		"西游记",
		"水浒传",
		"三国演义",
		"红楼梦",
		"金瓶梅",
		"真香定律"
	};

	const char* shamGroupNames[] = {
		"群聊测试1",
		"群聊测试2",
		"群聊测试3",
		"群聊测试4",
		"群聊测试5",
		"群聊测试6",
		"群聊测试7",
		"群聊测试8",
		"群聊测试9",
		"群聊测试10",
		"群聊测试11",
		"群聊测试12"
	};

	//添加联系人的假数据
	{
		for (int i = 0; i < 50; i++)
		{
			std::string strUUID = GenerateUUID();
			int nNameIndex = rand() % 11;
			std::string strTempName = shamNames[nNameIndex];

			std::ostringstream os;
			os.str("");
			os << strTempName << i;
			std::string strName = os.str();
			int nAreaIndex = rand() % 33;
			std::string strArea = shamAreas[nAreaIndex];

			m_mapPersonals.insert(std::make_pair(strUUID,
				PERSONAL_INFO(strUUID, strName, "", strArea, "鬼知道", "鸟随鸾凤腾飞远，人随贤良品自高。")));
		}
	}

	//添加群的假数据
	{
		for (int i = 0; i < 12; i++)
		{
			std::string strUUID = GenerateUUID();
			//int nNameIndex = rand() % 11;
			std::string strTempName = shamGroupNames[i];

			std::ostringstream os;
			os.str("");
			os << strTempName << i;
			std::string strName = os.str();

			m_mapGroups.insert(std::make_pair(strUUID, GROUP_INFO(strUUID, strName, "五一四天假")));
		}
	}
}
