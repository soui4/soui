/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SObjectFactory.h
 * @brief      Object Factory Management
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Description: Manages object factories for creating and managing objects in SOUI.
 */

#ifndef __SOBJECTFACTORY__H__
#define __SOBJECTFACTORY__H__

#include <core/SCmnMap.h>
#include <helper/obj-ref-impl.hpp>
#include <interface/SObjFactory-i.h>

/**
 * @brief Creates a new object information structure.
 * @param name Name of the object.
 * @param type Type of the object.
 * @param alise Alias for the object (optional).
 * @return New object information structure.
 */
EXTERN_C SOUI_EXP void ObjInfo_New(SObjectInfo *info,LPCWSTR name, int type, LPCWSTR alise = NULL);

/**
 * @brief Checks if the object information is valid.
 * @param pObjInfo Pointer to the object information structure.
 * @return TRUE if valid, FALSE otherwise.
 */
EXTERN_C SOUI_EXP BOOL ObjInfo_IsValid(const SObjectInfo *pObjInfo);

SNSBEGIN

/**
 * @class     CElementTraits< SObjectInfo >
 * @brief     SObjectInfo Hash and Comparison Traits
 *
 * Description: Provides hash and comparison functions for SObjectInfo used in the factory map.
 */
template <>
class CElementTraits<SObjectInfo> : public CElementTraitsBase<SObjectInfo> {
  public:
    /**
     * @brief Computes the hash value for an SObjectInfo.
     * @param objInfo Object information structure.
     * @return Hash value.
     */
    static ULONG Hash(INARGTYPE objInfo)
    {
        ULONG uRet = CElementTraits<SStringW>::Hash(objInfo.szName);
        uRet = (uRet << 5) + objInfo.nType;
        return uRet;
    }

    /**
     * @brief Compares two SObjectInfo structures.
     * @param obj1 First object information structure.
     * @param obj2 Second object information structure.
     * @return TRUE if the structures are equal, FALSE otherwise.
     */
    static bool CompareElements(INARGTYPE obj1, INARGTYPE obj2)
    {
        return obj1.nType == obj2.nType && _wcsicmp(obj1.szName, obj2.szName) == 0;
    }
};

/**
 * @class     TplSObjectFactory
 * @brief     Template-based object factory.
 * @tparam    T Type of the object to be created.
 *
 * Description: Implements the IObjectFactory interface for creating objects of type T.
 */
template <typename T>
class TplSObjectFactory : public TObjRefImpl<IObjectFactory> {
  public:
    /**
     * @brief Default constructor.
     */
    TplSObjectFactory()
    {
    }

    /**
     * @brief Called when the final reference is released.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS)
    {
        delete this;
    }

    /**
     * @brief Gets the base class name of the object.
     * @return Base class name.
     */
    STDMETHOD_(LPCWSTR, BaseClassName)(CTHIS) SCONST OVERRIDE
    {
        return T::BaseClassName();
    }

    /**
     * @brief Creates a new object of type T.
     * @return Pointer to the new object.
     */
    STDMETHOD_(IObject *, NewObject)(CTHIS) SCONST OVERRIDE
    {
        return new T;
    }

    /**
     * @brief Clones the object factory.
     * @return Pointer to the cloned factory.
     */
    STDMETHOD_(IObjectFactory *, Clone)(CTHIS) SCONST OVERRIDE
    {
        return new TplSObjectFactory<T>();
    }

    /**
     * @brief Gets the object information for the factory.
     * @return Object information structure.
     */
    STDMETHOD_(void, GetObjectInfo)(CTHIS_ SObjectInfo *info) SCONST OVERRIDE
    {
        ObjInfo_New(info, T::GetClassName(), T::GetClassType(), T::GetClassAlise());
    }
};

typedef IObjectFactory *SObjectFactoryPtr;

/**
 * @class     SObjectFactoryMgr
 * @brief     Manager for object factories.
 *
 * Description: Manages the registration, unregistration, and creation of objects using factories.
 */
class SOUI_EXP SObjectFactoryMgr : public SCmnMap<SObjectFactoryPtr, SObjectInfo> {
  public:
    /**
     * @brief Constructor for SObjectFactoryMgr.
     */
    SObjectFactoryMgr(void);

    /**
     * @brief Registers an object factory.
     * @param objFactory Pointer to the object factory.
     * @param bReplace Flag to replace existing factory if it exists.
     * @return TRUE if registration is successful, FALSE otherwise.
     */
    BOOL RegisterFactory(const IObjectFactory *objFactory, BOOL bReplace = false);

    /**
     * @brief Unregisters an object factory.
     * @param objInfo Object information structure.
     * @return TRUE if unregistration is successful, FALSE otherwise.
     */
    BOOL UnregisterFactory(const SObjectInfo &objInfo);

    /**
     * @brief Sets default attributes for a given object.
     * @param pObject Pointer to the object.
     */
    void SetSwndDefAttr(IObject *pObject) const;

    /**
     * @brief Gets the base object information from a given object information.
     * @param objInfo Object information structure.
     * @return Base object information structure.
     */
    SObjectInfo BaseObjectInfoFromObjectInfo(const SObjectInfo &objInfo) const;

    /**
     * @brief Template method to register a factory for a specific object type.
     * @tparam T Type of the object.
     * @return TRUE if registration is successful, FALSE otherwise.
     */
    template <class T>
    BOOL TplRegisterFactory()
    {
        TplSObjectFactory<T> fac;
        return RegisterFactory(&fac);
    }

    /**
     * @brief Template method to unregister a factory for a specific object type.
     * @tparam T Type of the object.
     * @return TRUE if unregistration is successful, FALSE otherwise.
     */
    template <class T>
    BOOL TplUnregisterFactory()
    {
        SObjectInfo info;
        ObjInfo_New(&info, T::GetClassName(), T::GetClassType());
        return UnregisterFactory(info);
    }

  protected:
    /**
     * @brief Creates an object based on the given object information.
     * @param objInfo Object information structure.
     * @return Pointer to the created object.
     */
    virtual IObject *CreateObject(const SObjectInfo &objInfo) const;

    /**
     * @brief Handles the creation of an unknown object.
     * @param objInfo Object information structure.
     * @return Pointer to the created object.
     */
    virtual IObject *OnCreateUnknownObject(const SObjectInfo &objInfo) const;

    /**
     * @brief Static callback for when a factory is removed.
     * @param obj Pointer to the object factory.
     */
    static void OnFactoryRemoved(const SObjectFactoryPtr &obj);
};

SNSEND

#endif // __SOBJECTFACTORY__H__