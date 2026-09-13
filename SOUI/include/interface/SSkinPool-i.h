#ifndef _SSKINPOOL_I_H__
#define _SSKINPOOL_I_H__

#include <interface/SSkinobj-i.h>
#include <interface/sxml-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ISkinPool
DECLARE_INTERFACE_(ISkinPool, IObjRef)
{
    /**
     * @brief Add reference
     * @return long -- reference count
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Release reference
     * @return long -- reference count
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Release object
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
    ///////////////////////////////////////////////////////////////////////

    /**
     * @brief Get the SkinObj matching the specified name
     * @param strSkinName LPCWSTR -- Name of the Skin object
     * @param nScale int -- Scale factor
     * @return ISkinObj* -- Found Skin object
     */
    STDMETHOD_(ISkinObj *, GetSkin)(THIS_ LPCWSTR strSkinName, int nScale) PURE;

    /**
     * @brief Load Skin list from XML
     * @param xmlNode IXmlNode* -- XML node describing the SkinObj
     * @return int -- Number of SkinObj loaded successfully
     */
    STDMETHOD_(int, LoadSkins)(THIS_ IXmlNode * xmlNode) PURE;

    /**
     * @brief Add a skinObj object to the SkinPool
     * @param skin ISkinObj* -- Skin object
     * @return BOOL -- Whether the addition succeeded; adding a duplicate name fails and returns FALSE
     */
    STDMETHOD_(BOOL, AddSkin)(THIS_ ISkinObj * skin) PURE;

    /**
     * @brief Remove a skinObj object
     * @param skin ISkinObj* -- Skin object
     * @return BOOL -- TRUE: success, FALSE: failure
     */
    STDMETHOD_(BOOL, RemoveSkin)(THIS_ ISkinObj * skin) PURE;

    /**
     * @brief Delete all skins
     * @return void
     */
    STDMETHOD_(void, RemoveAll)(THIS) PURE;
};

SNSEND

#endif /**< _SSKINPOOL_I_H__ */
