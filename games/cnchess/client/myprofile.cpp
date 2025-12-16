#include "stdafx.h"
#include "myprofile.h"

template<>
MyProfile * SSingleton<MyProfile>::ms_Singleton = NULL;

MyProfile::MyProfile(void)
{
    m_nTableId = -1;
    m_nSeatIndex = -1;
    m_pUserInfo = std::make_shared<GS_USERINFO>();
    m_pUserInfo->uid = 0;
}

MyProfile::~MyProfile(void)
{
}

void MyProfile::SetTableId(int nTableId)
{
    m_nTableId = nTableId;
}

int MyProfile::GetTableId()
{
    return m_nTableId;
}

void MyProfile::SetSeatIndex(int nSeatIndex)
{
    m_nSeatIndex = nSeatIndex;
}

int MyProfile::GetSeatIndex()
{
    return m_nSeatIndex;
}

void MyProfile::SetSex(int nSex)
{
    m_pUserInfo->nSex = nSex;
}

void MyProfile::SetName(const SStringT &strName)
{
    SStringA strNameA = S_CT2A(strName, CP_UTF8);
    strcpy_s(m_pUserInfo->szName, 50, strNameA.c_str());
}

SStringT MyProfile::GetName() const
{
    return S_CA2T(m_pUserInfo->szName,CP_UTF8); 
}

void MyProfile::SetAvatarData(const void *pData, int nSize)
{
    if(!pData){
        m_avatar.reset();
    }else{
        m_avatar = std::make_shared<std::vector<BYTE> >(nSize);
        memcpy(m_avatar->data(), pData, nSize);
    }
}

const std::shared_ptr<std::vector<BYTE> > MyProfile::GetAvatarData() const
{
    return m_avatar;
}


uint32_t MyProfile::GetUID() const
{
    return m_pUserInfo->uid;
}

void MyProfile::SetUID(uint32_t uid)
{
    m_pUserInfo->uid = uid;
}