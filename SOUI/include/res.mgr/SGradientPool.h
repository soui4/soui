/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 *
 * @file       SStylePool.h
 * @brief      SkinObj Pool
 * @version    v1.0
 * @author     soui
 * @date       2014-05-28
 *
 * Describe    管理Style
 */

#ifndef __SGRADIENTPOOL__H__
#define __SGRADIENTPOOL__H__
#include <core/SCmnMap.h>
#include <interface/obj-ref-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN
/**
 * @class      SGradientPool
 * @brief      name和IGradient的映射表
 *
 * Describe
 */
class SOUI_EXP SGradientPool
    : public SCmnMap<SAutoRefPtr<IGradient>, SStringW>
    , public TObjRefImpl<IObjRef> {
  public:
    /**
     * GetStyle
     * @brief    Get IGradient from pool by name
     * @param    LPCWSTR lpszName --  name of gradient
     * @return   IGradient -- gradient object
     * Describe
     */
    IGradient *GetGradient(const SStringW &strName);

    /**
     * Init
     * @brief    Load Style table from xml node
     * @param    SXmlNode xmlNode --  xml node that describe style list
     * @return   BOOL -- TRUE: loaded; FALSE:failed;
     * Describe
     */
    BOOL Init(SXmlNode xmlNode);
};

SNSEND
#endif // __SGRADIENTPOOL__H__