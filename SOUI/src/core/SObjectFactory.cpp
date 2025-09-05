#include "souistd.h"
#include "core/SObjectFactory.h"
#include "res.mgr/SObjDefAttr.h"
#include "res.mgr/SUiDef.h"


void ObjInfo_New(SObjectInfo *ret, LPCWSTR name, int type, LPCWSTR alise)
{
    SNS::SStringW strName(name);
    strName.MakeLower();
    SASSERT(strName.GetLength() < MAX_OBJNAME);
    wcscpy(ret->szName, strName.c_str());
    ret->nType = type;
    ret->szAlise = alise;
}

BOOL ObjInfo_IsValid(const SObjectInfo *pObjInfo)
{
    return pObjInfo->nType >= SNS::Undef && pObjInfo->szName[0] != 0;
}

SNSBEGIN

SObjectFactoryMgr::SObjectFactoryMgr(void)
{
    m_pFunOnKeyRemoved = OnFactoryRemoved;
}

//************************************
// Method:    RegisterFactory,注册APP自定义的窗口类
// Access:    public
// Returns:   bool
// Qualifier:
// Parameter: SObjectFactory * pWndFactory:窗口工厂指针
// Parameter: bool bReplace:强制替换原有工厂标志
//************************************
BOOL SObjectFactoryMgr::RegisterFactory(const IObjectFactory *objFactory, BOOL bReplace)
{
    SObjectInfo objInfo;
    objFactory->GetObjectInfo(&objInfo);
    if (HasKey(objInfo))
    {
        if (!bReplace)
            return FALSE;
        RemoveKeyObject(objInfo);
    }
    AddKeyObject(objInfo, objFactory->Clone());
    if (objInfo.szAlise)
    {
        SStringWList aliseList;
        SplitString(SStringW(objInfo.szAlise), L'|', aliseList);
        for (size_t i = 0; i < aliseList.GetCount(); i++)
        {
            SObjectInfo objInfoAlise;
            ObjInfo_New(&objInfoAlise, aliseList[i], objInfo.nType);
            AddKeyObject(objInfoAlise, objFactory->Clone());
        }
    }
    return TRUE;
}

void SObjectFactoryMgr::OnFactoryRemoved(const SObjectFactoryPtr &obj)
{
    obj->Release();
}

//************************************
// Method:    UnregisterFactor,反注册APP自定义的窗口类
// Access:    public
// Returns:   bool
// Qualifier:
// Parameter: SWindowFactory * pWndFactory
//************************************

BOOL SObjectFactoryMgr::UnregisterFactory(const SObjectInfo &objInfo)
{
    return (BOOL)RemoveKeyObject(objInfo);
}

IObject *SObjectFactoryMgr::CreateObject(const SObjectInfo &objInfo) const
{
    if (!HasKey(objInfo))
    {
        return OnCreateUnknownObject(objInfo);
    }
    IObject *pRet = GetKeyObject(objInfo)->NewObject();
    SASSERT(pRet);
    SetSwndDefAttr(pRet);
    return pRet;
}

SObjectInfo SObjectFactoryMgr::BaseObjectInfoFromObjectInfo(const SObjectInfo &objInfo) const
{
    SObjectInfo ret = { L"", NULL, Invalid };
    if (!HasKey(objInfo))
    {
        return ret;
    }

    SStringW strBaseClass = GetKeyObject(objInfo)->BaseClassName();
    if (strBaseClass == objInfo.szName)
        return ret;
    ObjInfo_New(&ret,strBaseClass, objInfo.nType);
    return ret;
}

void SObjectFactoryMgr::SetSwndDefAttr(IObject *pObject) const
{
    LPCWSTR pszClassName = pObject->GetObjectClass();

    if (pObject->GetObjectType() != Window)
        return;

    //检索并设置类的默认属性
    SXmlNode defAttr;
    SObjDefAttr *pDefObjAttr = GETUIDEF->GetUiDef()->GetObjDefAttr();
    if (pDefObjAttr)
        defAttr = pDefObjAttr->GetDefAttribute(pszClassName);

    if (defAttr)
    {
        //优先处理"class"属性
        SXmlAttr attrClass = defAttr.attribute(L"class");
        if (attrClass)
        {
            attrClass.set_userdata(1);
            pObject->SetAttribute(attrClass.name(), attrClass.value(), TRUE);
        }
        for (SXmlAttr attr = defAttr.first_attribute(); attr; attr = attr.next_attribute())
        {
            if (attr.get_userdata())
                continue;
            pObject->SetAttribute(attr.name(), attr.value(), TRUE);
        }
        if (attrClass)
        {
            attrClass.set_userdata(0);
        }
    }
}

IObject *SObjectFactoryMgr::OnCreateUnknownObject(const SObjectInfo &objInfo) const
{
    SSLOGD() << "Warning: no object " << objInfo.szName << "of type:" << objInfo.nType << "in SOUI !!";
    return NULL;
}

SNSEND