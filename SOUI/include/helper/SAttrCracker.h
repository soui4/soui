/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SAttrCracker.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/15
 *
 * Describe    Defines the XML attribute parsing macros for SOUI
 */

#ifndef _SATTRCRACK_H
#define _SATTRCRACK_H

/**
 * @brief Heads the attribute table of a class.
 *
 * The attribute name is split once, right here, into its optional member namespace prefix and
 * the remaining bare name -- see the SAttrChainPrefix block below for what the prefix is for:
 *   - strAttribName -- the name exactly as written in XML. The attribute table of the object
 *     itself keeps comparing against this one, so a prefixed name such as L"layout:width" is
 *     never taken by the object that owns the prefixed member;
 *   - strAttrPrefix -- the prefix including the separator (L"layout:"), empty for a plain name;
 *   - strAttrName   -- the bare name (L"width"), which is what a chained member always reads.
 * Splitting once here keeps that logic in a single place and lets every ATTR_CHAIN* macro
 * below route and forward with one comparison, instead of re-parsing the name per member.
 *
 * Not every attribute table chains to a member, so the two locals stay unused in most of them;
 * no warning is expected (SNS::SStringW is a class with a real constructor, which compilers
 * exclude from "set but not used" diagnostics).
 */
#define SOUI_ATTRS_BEGIN()                                                                                                 \
  public:                                                                                                                  \
    virtual HRESULT SetAttribute(const SNS::SStringW &strAttribName, const SNS::SStringW &strValue, BOOL bLoading = FALSE) \
    {                                                                                                                      \
        HRESULT hRet = E_FAIL;                                                                                             \
        const int nPrefixEnd = strAttribName.Find(L":");                                                                   \
        const SNS::SStringW strAttrPrefix = (nPrefixEnd < 0) ? SNS::SStringW() : strAttribName.Left(nPrefixEnd + 1);       \
        const SNS::SStringW strAttrName = (nPrefixEnd < 0) ? strAttribName : strAttribName.Mid(nPrefixEnd + 1);

/** Classes derived from SObject mark the end of properties */
#define SOUI_ATTRS_END()                                                            \
    if (FAILED(hRet))                                                               \
        return __baseCls::SetAttribute(strAttribName, strValue, bLoading);          \
    return AfterAttribute(strAttribName.c_str(), strValue.c_str(), bLoading, hRet); \
    }

/** End of the property table not handled by SObject */
#define SOUI_ATTRS_BREAK() \
    hRet = E_NOTIMPL;      \
    return hRet;           \
    }

/**
 * @brief Attribute namespace prefixes of the members reached through ATTR_CHAIN*.
 *
 * ATTR_CHAIN* hands an attribute down to a member object, so a member shares the XML
 * attribute namespace of the object owning it and two members exposing the same
 * attribute name cannot be told apart. An attribute may therefore name its member
 * explicitly with a namespace prefix, e.g. layout:width="-1" or animator:duration="200".
 *
 * The prefix is a selector, not decoration. An attribute name is either
 *   - plain (L"width"): it belongs to the object itself and is offered to every chained
 *     member in chain order, which is how all pre-existing SOUI XML is written; or
 *   - prefixed (L"layout:width"): it belongs to the member declaring that very prefix and
 *     is offered to that member alone. Members declaring another prefix -- and members
 *     declaring none -- are skipped without being asked, so a prefixed attribute can never
 *     be swallowed by the wrong member.
 * SOUI_ATTRS_BEGIN splits the name once into strAttrPrefix (L"layout:", empty when the name is
 * plain) and strAttrName (the bare L"width" a member always reads), so the chain macros below
 * only compare the prefix they own against strAttrPrefix and forward strAttrName. A prefixed
 * name never reaches the object's own attribute table either: that table keeps comparing the
 * full strAttribName.
 * A prefix no member of the chain declares is dropped, exactly like an unknown plain
 * attribute: it is never retried as a bare name.
 *
 * Every prefix is declared here instead of next to the class using it: a prefix has to
 * mean the same thing wherever it appears. Keep them lowercase and terminated by the
 * separator ':'. Add a new one only when no existing prefix already names that kind of
 * member.
 */
#ifdef __cplusplus
namespace SAttrChainPrefix
{
static const wchar_t *const STYLE = L"style:";               /**< SWindow's window style member */
static const wchar_t *const LAYOUT = L"layout:";             /**< SWindow's layout and layout parameter members */
static const wchar_t *const ANIMATOR = L"animator:";         /**< chained value animator */
static const wchar_t *const INTERPOLATOR = L"interpolator:"; /**< chained interpolator */
static const wchar_t *const GRADIENT = L"gradient:";         /**< chained gradient object */
static const wchar_t *const HOVER = L"hover:";               /**< SButton's hover animator */
static const wchar_t *const FADE = L"fade:";                 /**< chained fade interpolator */
static const wchar_t *const SLIDER = L"slider:";             /**< STabCtrl's slider animator */
static const wchar_t *const THUMB = L"thumb:";               /**< SSliderBar's thumb animator */
static const wchar_t *const VALUE = L"value:";               /**< SSliderBar's value animator */
} // namespace SAttrChainPrefix
#endif /**< __cplusplus */

/**
 * @brief Tells whether the attribute name carries a member namespace prefix.
 *
 * Attribute names never carry the separator for any other reason: the L"t:" namespace of
 * SWindow_style applies to element names only, and no SOUI XML resource writes a colon in an
 * attribute name. The name is not parsed here any more -- SOUI_ATTRS_BEGIN already split the
 * prefix off and this only tests the piece it produced.
 *
 * @param strAttrPrefix SNS::SStringW -- prefix split off by SOUI_ATTRS_BEGIN, empty when plain
 */
#define ATTR_IS_PREFIXED(strAttrPrefix) (!(strAttrPrefix).IsEmpty())

/**
 * @brief Tells whether the chained member owning @p prefix is a target of the attribute.
 *
 * This is the routing rule of the prefixes: a plain name is offered to every member (chain
 * order decides which one takes it), a prefixed name only to the member declaring exactly
 * that prefix. It is what keeps an attribute addressed to one member from being handed to
 * its siblings. The comparison is case insensitive, like every other attribute name in SOUI
 * (attribute names are all matched with CompareNoCase downstream).
 *
 * @param strAttrPrefix SNS::SStringW -- prefix split off by SOUI_ATTRS_BEGIN, empty when plain
 * @param prefix        LPCWSTR -- member prefix, including the separator
 */
#define ATTR_PREFIX_MATCHES(strAttrPrefix, prefix) ((strAttrPrefix).IsEmpty() || 0 == (strAttrPrefix).CompareNoCase(prefix))

/**
 * @brief Forwards the attribute to a member object that owns no namespace prefix.
 *
 * The member only accepts plain attribute names: a prefixed name addresses the member
 * declaring that prefix, and this member declared none. Prefer ATTR_CHAIN_PREFIX /
 * ATTR_CHAIN_PTR_PREFIX when the member's attributes may be spelled with a prefix.
 */
#define ATTR_CHAIN(varname, flag)                                                                                                      \
    if (FAILED(hRet) && !ATTR_IS_PREFIXED(strAttrPrefix) && SUCCEEDED(hRet = (varname).SetAttribute(strAttrName, strValue, bLoading))) \
    {                                                                                                                                  \
        hRet |= flag;                                                                                                                  \
    }                                                                                                                                  \
    else

/**
 * @brief Forwards the attribute to a member object owning @p prefix.
 * @param prefix LPCWSTR -- member prefix including the separator; take it from the
 *                          SAttrChainPrefix table so that one member role keeps one name
 *
 * A plain attribute name is forwarded as it is written, so the member keeps taking part in
 * the chain the way it always did; a prefixed name reaches it only when the prefix is its own.
 * Either way the member reads the bare strAttrName SOUI_ATTRS_BEGIN left for it.
 */
#define ATTR_CHAIN_PREFIX(varname, flag, prefix)                                                                                                 \
    if (FAILED(hRet) && ATTR_PREFIX_MATCHES(strAttrPrefix, prefix) && SUCCEEDED(hRet = (varname).SetAttribute(strAttrName, strValue, bLoading))) \
    {                                                                                                                                            \
        hRet |= flag;                                                                                                                            \
    }                                                                                                                                            \
    else

/**
 * @brief Forwards the attribute to a member object referenced by a pointer that owns no namespace prefix.
 */
#define ATTR_CHAIN_PTR(varname, flag)                                                                                                                         \
    if (FAILED(hRet) && !ATTR_IS_PREFIXED(strAttrPrefix) && varname != NULL && SUCCEEDED(hRet = (varname)->ISetAttribute(&strAttrName, &strValue, bLoading))) \
    {                                                                                                                                                         \
        hRet |= flag;                                                                                                                                         \
    }                                                                                                                                                         \
    else

/**
 * @brief Forwards the attribute to a member object referenced by a pointer, owning @p prefix.
 * @param prefix LPCWSTR -- member prefix including the separator, e.g. L"layout:"
 *
 * Like ATTR_CHAIN_PREFIX, the member is only asked when the attribute is plain or carries this
 * very prefix, and it always reads the bare name. Because SOUI_ATTRS_BEGIN left that bare name
 * in a local, it is an lvalue and this variant calls the member exactly like a non-prefixed
 * chained member does -- ISetAttribute, which is also how these members were reached before
 * prefixes existed. No IAttrAlias lookup is added: the alias of an XML attribute is resolved
 * once, by the SetAttribute(LPCWSTR) that started the chain, and resolving it again per member
 * would apply it with the member as the context instead of the object owning the attribute.
 */
#define ATTR_CHAIN_PTR_PREFIX(varname, flag, prefix)                                                                                                                    \
    if (FAILED(hRet) && ATTR_PREFIX_MATCHES(strAttrPrefix, prefix) && varname != NULL && SUCCEEDED(hRet = (varname)->ISetAttribute(&strAttrName, &strValue, bLoading))) \
    {                                                                                                                                                                   \
        hRet |= flag;                                                                                                                                                   \
    }                                                                                                                                                                   \
    else

/**
 * @brief Forwards the attribute to the base class implementation of SetAttribute.
 *
 * The base class is not a member: it shares this object's own attribute table and may chain
 * to members of its own, so it has to receive prefixed names as well -- the routing rule is
 * applied again inside its SetAttribute. Filtering here would hide L"animator:duration" from
 * a base class whose animator owns that prefix, so this macro stays unconditional.
 */
#define ATTR_CHAIN_CLASS(cls) \
    if (FAILED(hRet))         \
        hRet = cls::SetAttribute(strAttribName, strValue, bLoading);

#define ATTR_CUSTOM(attribname, func)                 \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        hRet = func(strValue, bLoading);              \
    }                                                 \
    else

#define STRINGASBOOL(strValue) (strValue).CompareNoCase(L"0") != 0 && (strValue).CompareNoCase(L"false") != 0

#define ATTR_BOOL(attribname, varname, allredraw)     \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        varname = STRINGASBOOL(strValue);             \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** Int = %d StringA */
#define ATTR_INT(attribname, varname, allredraw)      \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        int nRet = Str2IntW(strValue, TRUE);          \
        varname = nRet;                               \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

#define ATTR_LAYOUTSIZE(attribname, varname, allredraw) \
    if (0 == strAttribName.CompareNoCase(attribname))   \
    {                                                   \
        varname = GETLAYOUTSIZE(strValue);              \
        hRet = allredraw ? S_OK : S_FALSE;              \
    }                                                   \
    else

#define ATTR_LAYOUTSIZE2(attribname, varname, allredraw) \
    if (0 == strAttribName.CompareNoCase(attribname))    \
    {                                                    \
        SStringWList values;                             \
        if (SplitString(strValue, L',', values) != 2)    \
            return E_INVALIDARG;                         \
        varname[0] = GETLAYOUTSIZE(values[0]);           \
        varname[1] = GETLAYOUTSIZE(values[1]);           \
        hRet = allredraw ? S_OK : S_FALSE;               \
    }                                                    \
    else

#define ATTR_LAYOUTSIZE4(attribname, varname, allredraw) \
    if (0 == strAttribName.CompareNoCase(attribname))    \
    {                                                    \
        SStringWList values;                             \
        if (SplitString(strValue, L',', values) != 4)    \
            return E_INVALIDARG;                         \
        varname[0] = GETLAYOUTSIZE(values[0]);           \
        varname[1] = GETLAYOUTSIZE(values[1]);           \
        varname[2] = GETLAYOUTSIZE(values[2]);           \
        varname[3] = GETLAYOUTSIZE(values[3]);           \
        hRet = allredraw ? S_OK : S_FALSE;               \
    }                                                    \
    else

#define ATTR_MARGIN(attribname, varname, allredraw)                 \
    if (0 == strAttribName.CompareNoCase(attribname))               \
    {                                                               \
        SStringWList values;                                        \
        if (SplitString(strValue, L',', values) != 4)               \
            return E_INVALIDARG;                                    \
        varname.left = GETLAYOUTSIZE(values[0]).toPixelSize(100);   \
        varname.top = GETLAYOUTSIZE(values[1]).toPixelSize(100);    \
        varname.right = GETLAYOUTSIZE(values[2]).toPixelSize(100);  \
        varname.bottom = GETLAYOUTSIZE(values[3]).toPixelSize(100); \
        hRet = allredraw ? S_OK : S_FALSE;                          \
    }                                                               \
    else

/** Rect = %d,%d,%d,%d StringA */
#define ATTR_RECT(attribname, varname, allredraw)                                                          \
    if (0 == strAttribName.CompareNoCase(attribname))                                                      \
    {                                                                                                      \
        swscanf_s(strValue, L"%d,%d,%d,%d", &varname.left, &varname.top, &varname.right, &varname.bottom); \
        hRet = allredraw ? S_OK : S_FALSE;                                                                 \
    }                                                                                                      \
    else

/** Size = %d,%d StringA */
#define ATTR_SIZE(attribname, varname, allredraw)                \
    if (0 == strAttribName.CompareNoCase(attribname))            \
    {                                                            \
        swscanf_s(strValue, L"%d,%d", &varname.cx, &varname.cy); \
        hRet = allredraw ? S_OK : S_FALSE;                       \
    }                                                            \
    else

/** Point = %d,%d StringA */
#define ATTR_POINT(attribname, varname, allredraw)             \
    if (0 == strAttribName.CompareNoCase(attribname))          \
    {                                                          \
        swscanf_s(strValue, L"%d,%d", &varname.x, &varname.y); \
        hRet = allredraw ? S_OK : S_FALSE;                     \
    }                                                          \
    else

/** Point = %d,%d StringA */
#define ATTR_SPOINT(attribname, varname, allredraw)              \
    if (0 == strAttribName.CompareNoCase(attribname))            \
    {                                                            \
        swscanf_s(strValue, L"%f,%f", &varname.fX, &varname.fY); \
        hRet = allredraw ? S_OK : S_FALSE;                       \
    }                                                            \
    else

/** Float = %f StringA */
#define ATTR_FLOAT(attribname, varname, allredraw)    \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        float v;                                      \
        swscanf_s(strValue, L"%f", &v);               \
        varname = v;                                  \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** UInt = %u StringA */
#define ATTR_UINT(attribname, varname, allredraw)     \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        int nRet = Str2IntW(strValue, TRUE);          \
        varname = (UINT)nRet;                         \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** DWORD = %u StringA */
#define ATTR_DWORD(attribname, varname, allredraw)    \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        int nRet = Str2IntW(strValue, TRUE);          \
        varname = (DWORD)nRet;                        \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** WORD = %u StringA */
#define ATTR_WORD(attribname, varname, allredraw)     \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        int nRet = Str2IntW(strValue, TRUE);          \
        varname = (WORD)nRet;                         \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** bool = 0 or 1 StringA */
#define ATTR_BIT(attribname, varname, maskbit, allredraw) \
    if (0 == strAttribName.CompareNoCase(attribname))     \
    {                                                     \
        bool bSet = STRINGASBOOL(strValue);               \
        if (bSet)                                         \
            varname |= maskbit;                           \
        else                                              \
            varname &= ~(maskbit);                        \
        hRet = allredraw ? S_OK : S_FALSE;                \
    }                                                     \
    else

/** StringA = StringA */
#define ATTR_STRINGA(attribname, varname, allredraw)  \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        SNS::SStringW strTmp = GETSTRING(strValue);   \
        varname = S_CW2A(strTmp);                     \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** StringW = StringA */
#define ATTR_STRINGW(attribname, varname, allredraw)  \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        varname = GETSTRING(strValue);                \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** StringT = StringA */
#define ATTR_STRINGT(attribname, varname, allredraw)  \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        varname = S_CW2T(GETSTRING(strValue));        \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** StringA = StringA */
#define ATTR_I18NSTRA(attribname, varname, allredraw)   \
    if (0 == strAttribName.CompareNoCase(attribname))   \
    {                                                   \
        SNS::SStringW strTmp = tr(GETSTRING(strValue)); \
        varname = S_CW2A(strTmp);                       \
        hRet = allredraw ? S_OK : S_FALSE;              \
    }                                                   \
    else

/** STrText = StringA */
#define ATTR_I18NSTRT(attribname, varname, allredraw) \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        SNS::SStringW strTmp = GETSTRING(strValue);   \
        varname.SetText(S_CW2T(strTmp));              \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** DWORD = 0x08x StringA */
#define ATTR_HEX(attribname, varname, allredraw)      \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        int nRet = Str2IntW(strValue, TRUE);          \
        varname = nRet;                               \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** COLORREF = #06X or #08x or rgba(r,g,b,a) or rgb(r,g,b) */
#define ATTR_COLOR(attribname, varname, allredraw)    \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        if (!strValue.IsEmpty())                      \
        {                                             \
            varname = GETCOLOR(strValue);             \
            hRet = allredraw ? S_OK : S_FALSE;        \
        }                                             \
        else                                          \
        {                                             \
            hRet = E_FAIL;                            \
        }                                             \
    }                                                 \
    else

/** DpiAwareFont="face:宋体;bold:1;italic:1;underline:1;adding:10" */
#define ATTR_FONT(attribname, varname, allredraw)     \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        varname.SetFontDesc(strValue, GetScale());    \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** Value In {String1 : Value1, String2 : Value2 ...} */
#define ATTR_ENUM_BEGIN(attribname, vartype, allredraw) \
    if (0 == strAttribName.CompareNoCase(attribname))   \
    {                                                   \
        vartype varTemp;                                \
                                                        \
        hRet = allredraw ? S_OK : S_FALSE;

#define ATTR_ENUM_VALUE(enumstring, enumvalue) \
    if (strValue == enumstring)                \
        varTemp = enumvalue;                   \
    else

#define ATTR_ENUM_END(varname) \
    return E_FAIL;             \
                               \
    varname = varTemp;         \
    }                          \
    else

/** SwndStyle From StringA Key */
#define ATTR_STYLE(attribname, varname, allredraw)    \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        GETSTYLE(strValue, varname);                  \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** SSkinPool From StringA Key */
#define ATTR_SKIN(attribname, varname, allredraw)     \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        varname = GETSKIN(strValue, GetScale());      \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

/** SSkinPool From StringA Key */
#define ATTR_INTERPOLATOR(attribname, varname, allredraw) \
    if (0 == strAttribName.CompareNoCase(attribname))     \
    {                                                     \
        varname.Attach(CREATEINTERPOLATOR(strValue));     \
        hRet = allredraw ? S_OK : S_FALSE;                \
    }                                                     \
    else

/** ATTR_IMAGE: Directly use IResProvider::LoadImage to create an SNS::IBitmapS object; the reference count is 1 after successful creation */
/** No need to call AddRef, but Release must be called after use */
#define ATTR_IMAGE(attribname, varname, allredraw)    \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        SNS::IBitmapS *pImg = LOADIMAGE2(strValue);   \
        if (!pImg)                                    \
            hRet = E_FAIL;                            \
        else                                          \
        {                                             \
            if (varname)                              \
                varname->Release();                   \
            varname = pImg;                           \
            hRet = allredraw ? S_OK : S_FALSE;        \
        }                                             \
    }                                                 \
    else

/** ATTR_IMAGEAUTOREF: varname should be an SAutoRefPtr<SNS::IBitmapS> object */
#define ATTR_IMAGEAUTOREF(attribname, varname, allredraw) \
    if (0 == strAttribName.CompareNoCase(attribname))     \
    {                                                     \
        SNS::IBitmapS *pImg = LOADIMAGE2(strValue);       \
        if (!pImg)                                        \
            hRet = E_FAIL;                                \
        else                                              \
        {                                                 \
            varname = pImg;                               \
            pImg->Release();                              \
            hRet = allredraw ? S_OK : S_FALSE;            \
        }                                                 \
    }                                                     \
    else

#define ATTR_ICON(attribname, varname, allredraw)     \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        if (varname)                                  \
            DestroyIcon(varname);                     \
        varname = LOADICON2(strValue);                \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

#define ATTR_CHAR(attribname, varname, allredraw)     \
    if (0 == strAttribName.CompareNoCase(attribname)) \
    {                                                 \
        varname = *(LPCWSTR)strValue;                 \
        hRet = allredraw ? S_OK : S_FALSE;            \
    }                                                 \
    else

#define ATTR_ANIMATION(attribname, varname, allredraw)                                \
    if (0 == strAttribName.CompareNoCase(attribname))                                 \
    {                                                                                 \
        varname.Attach(SApplication::getSingleton().LoadAnimation(S_CW2T(strValue))); \
        hRet = allredraw ? S_OK : S_FALSE;                                            \
    }                                                                                 \
    else

#define ATTR_GRADIENT(attribname, varname, allredraw)                \
    if (0 == strAttribName.CompareNoCase(attribname))                \
    {                                                                \
        SNS::IGradient *pGradient = GETUIDEF->GetGradient(strValue); \
        if (!pGradient)                                              \
            hRet = E_FAIL;                                           \
        else                                                         \
        {                                                            \
            varname = pGradient;                                     \
            hRet = allredraw ? S_OK : S_FALSE;                       \
        }                                                            \
    }                                                                \
    else

#endif /**< _SATTRCRACK_H */