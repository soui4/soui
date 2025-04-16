/**
 * @file SFontPool.h
 * @brief SOUI字体管理模块
 * @version v1.0
 * @author SOUI group
 * @date 2014/08/02
 *
 * @details SOUI字体管理模块，用于管理和获取字体资源。
 */

#ifndef __SFONTPOOL__H__
#define __SFONTPOOL__H__

#include <core/SSingletonMap.h>
#include <interface/SRender-i.h>
#include <res.mgr/SFontInfo.h>
#include <helper/obj-ref-impl.hpp>

#define FF_DEFAULTFONT L""



SNSBEGIN

/**
 * @class FontKey
 * @brief 一个FONT的KEY
 *
 * @details 用于实现一个font map，标识字体的唯一性。
 */
typedef FontInfo FontKey;

/**
 * @class CElementTraits<FontInfo>
 * @brief FontInfo的Hash及比较模板
 *
 * @details 用于实现一个font map，提供Hash和比较函数。
 */
template <>
class CElementTraits<FontInfo> : public CElementTraitsBase<FontInfo> {
  public:
    /**
     * @brief 计算FontInfo的哈希值
     * @param fontKey FontInfo对象
     * @return 哈希值
     */
    static ULONG Hash(INARGTYPE fontKey)
    {
        ULONG uRet = SNS::CElementTraits<SStringW>::Hash(fontKey.strFaceName);
        uRet = (uRet << 5) + SNS::CElementTraits<SStringW>::Hash(fontKey.strPropEx);
        uRet = (uRet << 5) + (UINT)fontKey.style.syle + 1 + fontKey.scale;
        return uRet;
    }

    /**
     * @brief 比较两个FontInfo对象是否相等
     * @param element1 第一个FontInfo对象
     * @param element2 第二个FontInfo对象
     * @return 如果相等返回true，否则返回false
     */
    static bool CompareElements(INARGTYPE element1, INARGTYPE element2)
    {
        return element1.strFaceName == element2.strFaceName && element1.strPropEx == element2.strPropEx && element1.style.syle == element2.style.syle && element1.scale == element2.scale;
    }

    /**
     * @brief 按顺序比较两个FontInfo对象
     * @param element1 第一个FontInfo对象
     * @param element2 第二个FontInfo对象
     * @return 比较结果，0表示相等，负数表示element1小于element2，正数表示element1大于element2
     */
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
 * @class SFontPool
 * @brief 字体池管理类
 *
 * @details 管理和获取字体资源，支持字体的缓存和复用。
 */
class SOUI_EXP SFontPool : public SCmnMap<IFontPtr, FontInfo> {
  public:
    /**
     * @brief 设置检查字体的回调函数
     * @param fontCheck 检查字体的回调函数
     */
    static void SetFontChecker(FunFontCheck fontCheck);

    /**
     * @brief 检查一个字体是否有效
     * @param strFontName 字体名
     * @return TRUE-有效，FALSE-无效
     */
    static BOOL CheckFont(const SStringW &strFontName);

    /**
     * @brief 将字体描述转换为FontInfo
     * @param strFontInfo 字体描述字符串
     * @param defFontInfo 默认字体的FontInfo
     * @return FontInfo对象
     */
    static FontInfo FontInfoFromString(const SStringW &strFontInfo, const FontInfo &defFontInfo);

    /**
     * @brief 将FontInfo转换为字体描述
     * @param fi FontInfo对象
     * @return 字体描述字符串
     */
    static SStringW FontInfoToString(const FontInfo &fi);

    /**
     * @brief 获取与指定描述字符串对应的IFontPtr
     * @param strFont 字体描述字符串
     * @param scale 放大倍数
     * @return IFontPtr 字体对象指针
     *
     * @details 描述字符串格式如：face:宋体,bold:0,italic:1,underline:1,strike:1,adding:10
     */
    static IFontPtr GetFont(const SStringW &strFont, int scale);

    /**
     * @brief 设置默认字体
     * @param strFontInfo 默认字体描述字符串
     */
    static void SetDefFontInfo(const SStringW &strFontInfo);

    /**
     * @brief 获取默认字体信息
     * @return FontInfo 默认字体信息
     */
    static FontInfo GetDefFontInfo();

  protected:
    /**
     * @brief 当FontInfo被移除时的回调函数
     * @param obj 移除的IFontPtr对象
     */
    static void OnKeyRemoved(const IFontPtr &obj);

  protected:
    /**
     * @brief 构造函数
     * @param fac 渲染工厂对象指针
     */
    SFontPool(IRenderFactory *fac);

  protected:
    /**
     * @brief 获取默认字体信息的常量引用
     * @return FontInfo 默认字体信息的常量引用
     */
    const FontInfo &_GetDefFontInfo() const;

    /**
     * @brief 设置渲染工厂对象
     * @param fac 渲染工厂对象指针
     */
    void SetRenderFactory(IRenderFactory *fac);

    /**
     * @brief 获取与指定描述字符串对应的IFontPtr
     * @param strFont 字体描述字符串
     * @param scale 放大倍数
     * @return IFontPtr 字体对象指针
     */
    IFontPtr _GetFont(const SStringW &strFont, int scale);

    /**
     * @brief 设置默认字体描述字符串
     * @param strFontInfo 默认字体描述字符串
     */
    void _SetDefFontInfo(const SStringW &strFontInfo);

    /**
     * @brief 创建FontInfo对应的字体对象
     * @param fontInfo FontInfo对象
     * @return IFontPtr 字体对象指针
     */
    IFontPtr _CreateFont(const FontInfo &fontInfo);

    /**
     * @brief 设置默认字体信息
     * @param fontInfo FontInfo对象
     */
    void _SetDefFontInfo(const FontInfo &fontInfo);

    SAutoRefPtr<IRenderFactory> m_RenderFactory; // 渲染工厂对象指针
    FontInfo m_defFontInfo;                      // 默认字体信息

    static FunFontCheck s_funFontCheck; // 字体检查回调函数
};

SNSEND

#endif // __SFONTPOOL__H__