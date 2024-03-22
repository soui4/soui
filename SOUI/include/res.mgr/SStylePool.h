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

#ifndef __SSTYLEPOOL__H__
#define __SSTYLEPOOL__H__
#include <core/SCmnMap.h>
#include <interface/obj-ref-i.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN
/**
 * @class      SStylePool
 * @brief      name和SwndStyle的映射表
 *
 * Describe
 */
class SOUI_EXP SStylePool
    : public SCmnMap<SXmlNode, SStringW>
    , public TObjRefImpl<IObjRef> {
  public:
    /**
     * GetStyle
     * @brief    Get style object from pool by class name
     * @param    LPCWSTR lpszName --  name of style
     * @return   SXmlNode -- xml style
     * Describe
     */
    SXmlNode GetStyle(const SStringW &strName);

    /**
     * Init
     * @brief    Load Style table from xml node
     * @param    SXmlNode xmlNode --  xml node that describe style list
     * @return   BOOL -- TRUE: loaded; FALSE:failed;
     * Describe
     */
    BOOL Init(SXmlNode xmlNode);

  protected:
    SXmlDoc m_xmlDoc;
};

class SOUI_EXP STemplatePool
    : public SCmnMap<SStringW, SStringW>
    , public TObjRefImpl<IObjRef> {
  public:
    /**
     * Init
     * @brief    Load temlpate table from xml node
     * @param    SXmlNode xmlNode --  xml node that describe temlpate list
     * @return   BOOL -- TRUE: loaded; FALSE:failed;
     * Describe
     */
    BOOL Init(SXmlNode xmlNode);

    SStringW GetTemplateString(const SStringW &strName) const;
};

SNSEND
#endif // __SSTYLEPOOL__H__