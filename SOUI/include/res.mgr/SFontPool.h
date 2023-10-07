/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SFontPool.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Describe    SOUI字体管理模块
 */

#ifndef __SFONTPOOL__H__
#define __SFONTPOOL__H__

#include <core/ssingletonmap.h>
#include <interface/SRender-i.h>
#include <res.mgr/SFontInfo.h>
#include <helper/obj-ref-impl.hpp>

#define FF_DEFAULTFONT L""

/**
 * @class     FontKey
 * @brief      一个FONT的KEY
 *
 * Describe    用于实现一个font map
 */
SNSBEGIN
/**
 * @class     CElementTraits< FontKey >
 * @brief      FontKey的Hash及比较模板
 *
 * Describe    用于实现一个font map
 */
template <>
class CElementTraits<FontInfo> : public CElementTraitsBase<FontInfo> {
  public:
    static ULONG Hash(INARGTYPE fontKey)
    {
        ULONG uRet = SOUI::CElementTraits<SStringW>::Hash(fontKey.strFaceName);
        uRet = (uRet << 5) + SOUI::CElementTraits<SStringW>::Hash(fontKey.strPropEx);
        uRet = (uRet << 5) + (UINT)fontKey.style.syle + 1 + fontKey.scale;
        return uRet;
    }

    static bool CompareElements(INARGTYPE element1, INARGTYPE element2)
    {
        return element1.strFaceName == element2.strFaceName && element1.strPropEx == element2.strPropEx && element1.style.syle == element2.style.syle && element1.scale == element2.scale;
    }

    static int CompareElementsOrdered(INARGTYPE element1, INARGTYPE element2)
    {
        int nRet = element1.strFaceName.Compare(element2.strFaceName);
        if (nRet == 0)
            nRet = element1.strPropEx.Compare(element2.strPropEx);
        if (nRet == 0)
            nRet = (int)(element1.style.syle - element2.style.syle);
        if (nRet == 0)
            nRet = (int)(element1.scale - element2.scale);

        return nRet;
    }
};

typedef IFontS *IFontPtr;
typedef BOOL (*FunFontCheck)(const SStringW &strFontName);

/**
 * @class      SFontPool
 * @brief      font pool
 *
 * Describe
 */
class SOUI_EXP SFontPool : public SCmnMap<IFontPtr, FontInfo> {
  public:
    /**
     * @brief 设置检查字体的回调函数
     * @param fontCheck  检查字体的回调函数
     */
    static void SetFontChecker(FunFontCheck fontCheck);

    /**
     * @brief 检查一个字体是否有效
     * @param strFontName 字体名
     * @return TRUE-有效
     */
    static BOOL CheckFont(const SStringW &strFontName);

    /**
     * @brief 将字体描述转换为FontInfo
     * @param strFontInfo 字体描述
     * @param defFontInfo 默认字体的FontInfo
     * @return FontInfo
     */
    static FontInfo FontInfoFromString(const SStringW &strFontInfo, const FontInfo &defFontInfo);

    /**
     * @brief 将FontInfo转换为字体描述
     * @param fi FontInfo
     * @return 字体描述
     */
    static SStringW FontInfoToString(const FontInfo &fi);

    /**
     * GetFont
     * @brief    获得与指定的strFont对应的IFontPtr
     * @param    const SStringW & strFont --  font描述字符串
     * @param scale 放大倍数
     * @return   IFontPtr -- font对象
     *
     * Describe  描述字符串格式如：face:宋体,bold:0,italic:1,underline:1,strike:1,adding:10
     */
    static IFontPtr GetFont(const SStringW &strFont, int scale);

    /**
     * @brief 设置默认字体
     * @param strFontInfo 默认字体描述
     */
    static void SetDefFontInfo(const SStringW &strFontInfo);

    /**
     * @brief 获取默认字体信息
     * @return FontInfo 字体信息
     */
    static FontInfo GetDefFontInfo();

  protected:
    static void OnKeyRemoved(const IFontPtr &obj);

  protected:
    SFontPool(IRenderFactory *fac);

  protected:
    const FontInfo &_GetDefFontInfo() const;
    void SetRenderFactory(IRenderFactory *fac);

    IFontPtr _GetFont(const SStringW &strFont, int scale);
    void _SetDefFontInfo(const SStringW &strFontInfo);

    IFontPtr _CreateFont(const FontInfo &fontInfo);
    void _SetDefFontInfo(const FontInfo &fontInfo);

    SAutoRefPtr<IRenderFactory> m_RenderFactory;
    FontInfo m_defFontInfo;

    static FunFontCheck s_funFontCheck;
};

SNSEND

#endif // __SFONTPOOL__H__