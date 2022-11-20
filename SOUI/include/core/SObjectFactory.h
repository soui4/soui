/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SwndFactoryMgr.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Describe    SOUI窗口类厂管理器
 */

#pragma once
#include <core/SCmnMap.h>
#include <helper/obj-ref-impl.hpp>
SNSBEGIN

SOUI_EXP SObjectInfo  ObjInfo_New(LPCWSTR name,int type);

SOUI_EXP BOOL ObjInfo_IsValid(const SObjectInfo* pObjInfo);

/**
 * @class     CElementTraits< SObjectInfo >
 * @brief      SObjectInfo的Hash及比较模板
 *
 * Describe    用于实现一个SObjectInfo SObjectFactroy map
 */
template <>
class CElementTraits<SObjectInfo> : public CElementTraitsBase<SObjectInfo> {
  public:
    static ULONG Hash(INARGTYPE objInfo)
    {
		ULONG uRet = CElementTraits<SStringW>::Hash(objInfo.szName);
		uRet = (uRet << 5) + objInfo.nType;
		return uRet;
    }

    static bool CompareElements(INARGTYPE obj1, INARGTYPE obj2)
    {
        return obj1.nType == obj2.nType && wcsicmp(obj1.szName,obj2.szName)== 0;
    }
};

template <typename T>
class TplSObjectFactory : public TObjRefImpl<IObjectFactory>{
  public:
    //! Default constructor.
    TplSObjectFactory()
    {
    }
	
	STDMETHOD_(void,OnFinalRelease)(THIS){
		delete this;
	}

	STDMETHOD_(LPCWSTR,BaseClassName)(CTHIS) SCONST OVERRIDE
    {
        return T::BaseClassName();
    }

    STDMETHOD_(IObject *,NewObject)(CTHIS) SCONST OVERRIDE
    {
        return new T;
    }

    STDMETHOD_(IObjectFactory *,Clone)(CTHIS) SCONST OVERRIDE
    {
        return new TplSObjectFactory<T>();
    }

    // 通过 SObjectFactory 继承
    STDMETHOD_(SObjectInfo,GetObjectInfo)(CTHIS) SCONST OVERRIDE
    {
        return ObjInfo_New(T::GetClassName(), T::GetClassType());
    }
};

typedef IObjectFactory * SObjectFactoryPtr;

class SOUI_EXP SObjectFactoryMgr : public SCmnMap<SObjectFactoryPtr, SObjectInfo> {
  public:
    SObjectFactoryMgr(void);

    //************************************
    // Method:    RegisterFactory,注册APP自定义的窗口类
    // Access:    public
    // Returns:   bool
    // Qualifier:
    // Parameter: SObjectFactory * pWndFactory:窗口工厂指针
    // Parameter: bool bReplace:强制替换原有工厂标志
    //************************************
    BOOL RegisterFactory(const IObjectFactory *objFactory, BOOL bReplace = false);

    //************************************
    // Method:    UnregisterFactor,反注册APP自定义的窗口类
    // Access:    public
    // Returns:   bool
    // Qualifier:
    // Parameter: SWindowFactory * pWndFactory
    //************************************
    BOOL UnregisterFactory(const SObjectInfo &objInfo);

    void SetSwndDefAttr(IObject *pObject) const;

    SObjectInfo BaseObjectInfoFromObjectInfo(const SObjectInfo &objInfo);

    template <class T>
    BOOL TplRegisterFactory()
    {
        return RegisterFactory(&TplSObjectFactory<T>());
    }

    template <class T>
    BOOL TplUnregisterFactory()
    {
        return UnregisterFactory(ObjInfo_New(T::GetClassName(), T::GetClassType()));
    }

  protected:
	virtual IObject *CreateObject(const SObjectInfo &objInfo) const;
    virtual IObject *OnCreateUnknownObject(const SObjectInfo &objInfo) const;

    static void OnFactoryRemoved(const SObjectFactoryPtr &obj);
};

SNSEND