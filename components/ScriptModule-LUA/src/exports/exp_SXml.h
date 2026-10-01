#ifndef __EXP_SXML_H__
#define __EXP_SXML_H__

#include <interface/sxml-i.h>
#include <xml/SXml.h>
#include "toobj.h"

using namespace pugi;

// ---------------------------------------------------------------------------
// SXmlAttr  (concrete, value based)
// ---------------------------------------------------------------------------
static void ExpLua_SXmlAttr(lua_State *L)
{
    lua_tinker::class_add<SXmlAttr>(L, "SXmlAttr");
    lua_tinker::class_inh<SXmlAttr, IXmlAttr>(L);
    lua_tinker::class_con<SXmlAttr>(L, lua_tinker::constructor<SXmlAttr>);
    lua_tinker::class_con<SXmlAttr>(L, lua_tinker::constructor<SXmlAttr, const SXmlAttr &>);

    lua_tinker::class_def<SXmlAttr>(L, "Next", &SXmlAttr::next_attribute);
    lua_tinker::class_def<SXmlAttr>(L, "Prev", &SXmlAttr::previous_attribute);

    // convenience value readers (inherited from IXmlAttr, need explicit arg)
    lua_tinker::class_def<SXmlAttr>(L, "AsInt", (int (IXmlAttr::*)(int)) & IXmlAttr::AsInt);
    lua_tinker::class_def<SXmlAttr>(L, "AsUint", (unsigned int (IXmlAttr::*)(int)) & IXmlAttr::AsUint);
    lua_tinker::class_def<SXmlAttr>(L, "AsFloat", (float (IXmlAttr::*)(float)) & IXmlAttr::AsFloat);
    lua_tinker::class_def<SXmlAttr>(L, "AsDouble", (double (IXmlAttr::*)(double)) & IXmlAttr::AsDouble);
    lua_tinker::class_def<SXmlAttr>(L, "AsBool", (BOOL (IXmlAttr::*)(BOOL)) & IXmlAttr::AsBool);

    DEF_CAST_PVOID(L, SXmlAttr);
}

// ---------------------------------------------------------------------------
// SXmlNode  (concrete, value based)
// ---------------------------------------------------------------------------
static void ExpLua_SXmlNode(lua_State *L)
{
    lua_tinker::class_add<SXmlNode>(L, "SXmlNode");
    lua_tinker::class_inh<SXmlNode, IXmlNode>(L);
    lua_tinker::class_con<SXmlNode>(L, lua_tinker::constructor<SXmlNode>);
    lua_tinker::class_con<SXmlNode>(L, lua_tinker::constructor<SXmlNode, const SXmlNode &>);

    lua_tinker::class_def<SXmlNode>(L, "Attribute", (SXmlAttr (SXmlNode::*)(const wchar_t *, bool) const) & SXmlNode::attribute);
    lua_tinker::class_def<SXmlNode>(L, "Attribute2", (SXmlAttr (SXmlNode::*)(const wchar_t *, bool)) & SXmlNode::attribute2);
    lua_tinker::class_def<SXmlNode>(L, "FirstAttribute", (SXmlAttr (SXmlNode::*)() const) & SXmlNode::first_attribute);
    lua_tinker::class_def<SXmlNode>(L, "LastAttribute", (SXmlAttr (SXmlNode::*)() const) & SXmlNode::last_attribute);

    lua_tinker::class_def<SXmlNode>(L, "Child", (SXmlNode (SXmlNode::*)(const wchar_t *, bool) const) & SXmlNode::child);
    lua_tinker::class_def<SXmlNode>(L, "FirstChild", (SXmlNode (SXmlNode::*)() const) & SXmlNode::first_child);
    lua_tinker::class_def<SXmlNode>(L, "LastChild", (SXmlNode (SXmlNode::*)() const) & SXmlNode::last_child);
    lua_tinker::class_def<SXmlNode>(L, "NextSibling", (SXmlNode (SXmlNode::*)() const) & SXmlNode::next_sibling);
    lua_tinker::class_def<SXmlNode>(L, "NextSibling2", (SXmlNode (SXmlNode::*)(const wchar_t *, bool) const) & SXmlNode::next_sibling);
    lua_tinker::class_def<SXmlNode>(L, "PrevSibling", (SXmlNode (SXmlNode::*)() const) & SXmlNode::previous_sibling);
    lua_tinker::class_def<SXmlNode>(L, "PrevSibling2", (SXmlNode (SXmlNode::*)(const wchar_t *, bool) const) & SXmlNode::previous_sibling);
    lua_tinker::class_def<SXmlNode>(L, "Parent", (SXmlNode (SXmlNode::*)() const) & SXmlNode::parent);
    lua_tinker::class_def<SXmlNode>(L, "ChildValue", (const wchar_t * (SXmlNode::*)() const) & SXmlNode::child_value);

    DEF_CAST_PVOID(L, SXmlNode);
}

// ---------------------------------------------------------------------------
// SXmlDoc  (concrete, ref counted)
// ---------------------------------------------------------------------------
static void ExpLua_SXmlDoc(lua_State *L)
{
    lua_tinker::class_add<SXmlDoc>(L, "SXmlDoc");
    lua_tinker::class_inh<SXmlDoc, IXmlDoc>(L);
    lua_tinker::class_con<SXmlDoc>(L, lua_tinker::constructor<SXmlDoc>);

    lua_tinker::class_def<SXmlDoc>(L, "Root", &SXmlDoc::root);

    // LoadFileA(path[, options[, encoding]]) - wrapper with optional args
    class_set_cfun<SXmlDoc>(L, "LoadFileA", [](lua_State *L) -> int {
        SXmlDoc *_this = lua_tinker_toobj<SXmlDoc>(L, 1);
        if (!_this)
            return luaL_error(L, "expect SXmlDoc at arg 1");
        const char *path = luaL_checkstring(L, 2);
        unsigned int options = (unsigned int)(lua_isnoneornil(L, 3) ? pugi::parse_default : lua_tointeger(L, 3));
        int encoding = (int)(lua_isnoneornil(L, 4) ? pugi::encoding_auto : lua_tointeger(L, 4));
        BOOL bRet = _this->LoadFileA(path, options, (XmlEncoding)encoding);
        lua_pushboolean(L, bRet);
        return 1;
    });

    // LoadStringU8(utf8 contents[, options])
    class_set_cfun<SXmlDoc>(L, "LoadStringU8", [](lua_State *L) -> int {
        SXmlDoc *_this = lua_tinker_toobj<SXmlDoc>(L, 1);
        if (!_this)
            return luaL_error(L, "expect SXmlDoc at arg 1");
        const char *contents = luaL_checkstring(L, 2);
        unsigned int options = (unsigned int)(lua_isnoneornil(L, 3) ? pugi::parse_default : lua_tointeger(L, 3));
        SStringW wstr = S_CA2W(contents, CP_UTF8);
        BOOL bRet = _this->LoadString(wstr.c_str(), options);
        lua_pushboolean(L, bRet);
        return 1;
    });

    DEF_CAST_OBJREF(L, SXmlDoc);
}

static BOOL ExpLua_SXml(lua_State *L)
{
    try
    {
        ExpLua_SXmlAttr(L);
        ExpLua_SXmlNode(L);
        ExpLua_SXmlDoc(L);
        return TRUE;
    }
    catch (...)
    {
        return FALSE;
    }
}

#endif // __EXP_SXML_H__
