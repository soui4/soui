#include "stdafx.h"
#include "myprofile.h"

template<>
MyProfile * SSingleton<MyProfile>::ms_Singleton = NULL;

MyProfile::MyProfile(void)
{
    m_nTableId = -1;
    m_nSeatIndex = -1;
    m_nAvatarId = BuiltinAvatar::EXTERNAL;
    m_pUserInfo = std::make_shared<GS_USERINFO>();
    m_pUserInfo->uid = 0;
    m_pUserInfo->nAvatarId = BuiltinAvatar::EXTERNAL;
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
    // 首次设置性别时，自动匹配默认内置头像
    if (m_nAvatarId == BuiltinAvatar::EXTERNAL && m_strAvatarPath.IsEmpty() && !m_avatar)
    {
        if (nSex == SEX_FEMALE)
            SetAvatarId(BuiltinAvatar::FEMALE);
        else
            SetAvatarId(BuiltinAvatar::MALE);
    }
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

void MyProfile::SetAvatarId(int nId)
{
    m_nAvatarId = nId;
    m_pUserInfo->nAvatarId = nId;
    if (nId >= 0)
    {
        // 选中内置头像，清除外部路径和二进制数据
        m_strAvatarPath.Empty();
        m_avatar.reset();
    }
}

void MyProfile::SetAvatarPath(const SStringT &strPath)
{
    m_strAvatarPath = strPath;
    if (strPath.IsEmpty())
        return;
    // 使用外部图片：标记头像ID为EXTERNAL，并读取文件内容到二进制（用于上传）
    m_nAvatarId = BuiltinAvatar::EXTERNAL;
    m_pUserInfo->nAvatarId = BuiltinAvatar::EXTERNAL;
    FILE *f = _tfopen(strPath, _T("rb"));
    if (!f)
    {
        m_avatar.reset();
        return;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz <= 0 || sz > 4*1024*1024) // 限制4MB
    {
        fclose(f);
        m_avatar.reset();
        return;
    }
    fseek(f, 0, SEEK_SET);
    m_avatar = std::make_shared<std::vector<BYTE> >((size_t)sz);
    size_t rd = fread(m_avatar->data(), 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz)
        m_avatar.reset();
}


uint32_t MyProfile::GetUID() const
{
    return m_pUserInfo->uid;
}

void MyProfile::SetUID(uint32_t uid)
{
    m_pUserInfo->uid = uid;
}