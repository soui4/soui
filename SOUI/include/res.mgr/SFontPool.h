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

#pragma once

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
        return element1.strFaceName == element2.strFaceName 
			&& element1.strPropEx == element2.strPropEx 
			&& element1.style.syle == element2.style.syle
			&& element1.scale == element2.scale;
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
class SOUI_EXP SFontPool : public SSingletonMap<SFontPool, IFontPtr, FontInfo> {
    SINGLETON2_TYPE(SINGLETON_FONTPOOL)
  public:
    SFontPool(IRenderFactory *pRendFactory);

public:
	static void SetFontChecker(FunFontCheck fontCheck);
	static BOOL CheckFont(const SStringW &strFontName);
public:
    /**
     * GetFont
     * @brief    获得与指定的strFont对应的IFontPtr
     * @param    const SStringW & strFont --  font描述字符串
     * @return   IFontPtr -- font对象
     *
     * Describe  描述字符串格式如：face:宋体,bold:0,italic:1,underline:1,strike:1,adding:10
     */
    IFontPtr GetFont(const SStringW &strFont, int scale);

    void SetDefFontInfo(const SStringW &strFontInfo);

public:
	static FontInfo FontInfoFromString(const SStringW &strFontInfo, const FontInfo & defFontInfo);
	static SStringW FontInfoToString(const FontInfo & fi);
  protected:

    const FontInfo &GetDefFontInfo() const;

    static void OnKeyRemoved(const IFontPtr &obj);

    IFontPtr _CreateFont(const FontInfo &fontInfo);
	void _SetDefFontInfo(const FontInfo &fontInfo);

    SAutoRefPtr<IRenderFactory> m_RenderFactory;
    FontInfo m_defFontInfo;
    SCriticalSection m_cs;

	static FunFontCheck s_funFontCheck;
};

SNSEND
