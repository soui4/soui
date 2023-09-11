#ifndef _SSKINPOOL_I_H__
#define _SSKINPOOL_I_H__

#include <interface/SSkinobj-i.h>
#include <interface/sxml-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE ISkinPool
DECLARE_INTERFACE_(ISkinPool, IObjRef)
{
    //!添加引用
    /*!
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //!释放引用
    /*!
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //!释放对象
    /*!
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * GetSkin
     * @brief    获得与指定name匹配的SkinObj
     * @param    LPCWSTR strSkinName --    Name of Skin Object
     * @return   ISkinObj*  -- 找到的Skin Object
     * Describe
     */
    STDMETHOD_(ISkinObj *, GetSkin)(THIS_ LPCWSTR strSkinName, int nScale) PURE;

    /**
     * LoadSkins
     * @brief    从XML中加载Skin列表
     * @param    SXmlNode xmlNode --  描述SkinObj的XML表
     * @return   int -- 成功加载的SkinObj数量
     * Describe
     */
    STDMETHOD_(int, LoadSkins)(THIS_ IXmlNode * xmlNode) PURE;

    /**
     * AddSkin
     * @brief    增加一个skinObj对象到SkinPool
     * @param    ISkinObj* --  SkinObj
     * @return   BOOL -- 加入是否成功,重名加入失败
     * Describe
     */
    STDMETHOD_(BOOL, AddSkin)(THIS_ ISkinObj * skin) PURE;

    /**
     * RemoveSkin
     * @brief    删除一个skinObj对象
     * @param    ISkinObj* --  SkinObj
     * @return   BOOL -- TRUE-成功
     * Describe
     */
    STDMETHOD_(BOOL, RemoveSkin)(THIS_ ISkinObj * skin) PURE;

    /**
     * RemoveAll
     * @brief    删除全部skin
     * @return   void
     * Describe
     */
    STDMETHOD_(void, RemoveAll)(THIS) PURE;
};

SNSEND

#endif //_SSKINPOOL_I_H__