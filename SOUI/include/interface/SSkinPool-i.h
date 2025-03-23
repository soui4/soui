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
     * @brief 添加引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief 释放引用
     * @return long -- 引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief 释放对象
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 获得与指定name匹配的SkinObj
     * @param strSkinName LPCWSTR -- Skin对象的名称
     * @param nScale int -- 缩放比例
     * @return ISkinObj* -- 找到的Skin对象
     */
    STDMETHOD_(ISkinObj *, GetSkin)(THIS_ LPCWSTR strSkinName, int nScale) PURE;

    /**
     * @brief 从XML中加载Skin列表
     * @param xmlNode IXmlNode* -- 描述SkinObj的XML节点
     * @return int -- 成功加载的SkinObj数量
     */
    STDMETHOD_(int, LoadSkins)(THIS_ IXmlNode * xmlNode) PURE;

    /**
     * @brief 增加一个skinObj对象到SkinPool
     * @param skin ISkinObj* -- Skin对象
     * @return BOOL -- 加入是否成功，重名加入失败返回FALSE
     */
    STDMETHOD_(BOOL, AddSkin)(THIS_ ISkinObj * skin) PURE;

    /**
     * @brief 删除一个skinObj对象
     * @param skin ISkinObj* -- Skin对象
     * @return BOOL -- TRUE: 成功，FALSE: 失败
     */
    STDMETHOD_(BOOL, RemoveSkin)(THIS_ ISkinObj * skin) PURE;

    /**
     * @brief 删除全部skin
     * @return void
     */
    STDMETHOD_(void, RemoveAll)(THIS) PURE;
};

SNSEND

#endif //_SSKINPOOL_I_H__


