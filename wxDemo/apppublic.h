#pragma once

#include <vector>
#include <string>
#include <map>

typedef struct _tagCustomizeTime
{
	int Year;
	int Mon;
	int Day;
	int Hour;
	int Min;
	int Second;
}CustomizeTime;

enum EcPosType
{
	Null = -1,
	TopLeft,
	TopCenter,
	TopRight,
	RightCenter,
	BottomRight,
	BottomCenter,
	BottomLeft,
	LeftCenter,
	SelectRect,
};

typedef struct _tagPersonalInfo
{
	std::string m_strID;
	std::string m_strName;
	std::string m_strRemark;
	std::string m_strArea;
	std::string m_strOrigin;
	std::string m_strSign;

	_tagPersonalInfo() {
		m_strID = "";
		m_strName = "";
		m_strRemark = "";
		m_strArea = "";
		m_strOrigin = "";
		m_strSign = "";
	}
	_tagPersonalInfo(const std::string& strID, const std::string& strName, const std::string& strRemark,
		const std::string& strArea, const std::string& strOrigin, const std::string& strSign) {
		m_strID = strID;
		m_strName = strName;
		m_strRemark = strRemark;
		m_strArea = strArea;
		m_strOrigin = strOrigin;
		m_strSign = strSign;
	}
}PERSONAL_INFO;
typedef std::map<std::string, PERSONAL_INFO>	PersonalsMap;

typedef struct _tagGroupInfo
{
	std::string m_strGroupID;
	std::string m_strGroupName;
	std::string m_strGroupNotice;

	_tagGroupInfo() {
		m_strGroupID = "";
		m_strGroupName = "";
		m_strGroupNotice = "";
	}
	_tagGroupInfo(const std::string& strGroupID, const std::string& strGroupName,
		const std::string& strGroupNotice) {
		m_strGroupID = strGroupID;
		m_strGroupName = strGroupName;
		m_strGroupNotice = strGroupNotice;
	}
	std::vector<std::string> m_vecGroupMembers;
}GROUP_INFO;
typedef std::map<std::string, GROUP_INFO>		GroupsMap;

typedef struct _tagGrpmbrInfo
{
	std::string m_strGrpmbrID;
	std::string m_strGrpmbrName;
	std::string m_strGrpmbrArea;
	_tagGrpmbrInfo() {
		m_strGrpmbrID = "";
		m_strGrpmbrName = "";
		m_strGrpmbrArea = "";
	}
	_tagGrpmbrInfo(const std::string& strGrpmbrID, const std::string& strGrpmbrName,
		const std::string& strGrpmbrArea) {
		m_strGrpmbrID = strGrpmbrID;
		m_strGrpmbrName = strGrpmbrName;
		m_strGrpmbrArea = strGrpmbrArea;
	}
}GRPMBR_INFO;
typedef std::map<std::string, GRPMBR_INFO>		GroupMembersMap;

typedef struct _tagGHInfo
{
	std::string m_strGhID;
	std::string m_strName;
	std::string m_strSummary;
	_tagGHInfo() {
		m_strGhID = "";
		m_strName = "";
		m_strSummary = "";
	}
	_tagGHInfo(const std::string& strID, const std::string& strName, const std::string& strSummary) {
		m_strGhID = strID;
		m_strName = strName;
		m_strSummary = strSummary;
	}
}GH_INFO;
typedef std::map<std::string, GH_INFO>			GHsMap;