#include <interface/sxml-i.h>
#include "toobj.h"
using namespace pugi;

// interface As* methods have DEF_VAL default args; lua_tinker cannot fill
// them, so wrap with lua_CFunctions that apply the defaults.
static int IXmlAttr_AsInt(lua_State *L)
{
    IXmlAttr *_this = lua_tinker_toobj<IXmlAttr>(L, 1);
    if (!_this) return 0;
    int def = lua_isnoneornil(L, 2) ? 0 : (int)lua_tointeger(L, 2);
    lua_pushinteger(L, _this->AsInt(def));
    return 1;
}
static int IXmlAttr_AsUint(lua_State *L)
{
    IXmlAttr *_this = lua_tinker_toobj<IXmlAttr>(L, 1);
    if (!_this) return 0;
    int def = lua_isnoneornil(L, 2) ? 0 : (int)lua_tointeger(L, 2);
    lua_pushinteger(L, (lua_Integer)_this->AsUint(def));
    return 1;
}
static int IXmlAttr_AsFloat(lua_State *L)
{
    IXmlAttr *_this = lua_tinker_toobj<IXmlAttr>(L, 1);
    if (!_this) return 0;
    float def = lua_isnoneornil(L, 2) ? 0.0f : (float)lua_tonumber(L, 2);
    lua_pushnumber(L, _this->AsFloat(def));
    return 1;
}
static int IXmlAttr_AsDouble(lua_State *L)
{
    IXmlAttr *_this = lua_tinker_toobj<IXmlAttr>(L, 1);
    if (!_this) return 0;
    double def = lua_isnoneornil(L, 2) ? 0.0 : lua_tonumber(L, 2);
    lua_pushnumber(L, _this->AsDouble(def));
    return 1;
}
static int IXmlAttr_AsBool(lua_State *L)
{
    IXmlAttr *_this = lua_tinker_toobj<IXmlAttr>(L, 1);
    if (!_this) return 0;
    BOOL def = lua_isnoneornil(L, 2) ? FALSE : (lua_toboolean(L, 2) ? TRUE : FALSE);
    lua_pushboolean(L, _this->AsBool(def) ? 1 : 0);
    return 1;
}

BOOL ExpLua_IXml(lua_State *L)
{
    try{
        lua_tinker::class_add<IXmlAttr>(L,"IXmlAttr");
		lua_tinker::class_inh<IXmlAttr,IObjRef>(L);
		lua_tinker::class_def<IXmlAttr>(L,"GetPrivPtr",&IXmlAttr::GetPrivPtr);
		lua_tinker::class_def<IXmlAttr>(L,"Empty",&IXmlAttr::Empty);
		lua_tinker::class_def<IXmlAttr>(L,"Name",&IXmlAttr::Name);
		lua_tinker::class_def<IXmlAttr>(L,"Value",&IXmlAttr::Value);
		lua_tinker::class_def<IXmlAttr>(L,"set_userdata",&IXmlAttr::set_userdata);
		lua_tinker::class_def<IXmlAttr>(L,"get_userdata",&IXmlAttr::get_userdata);
		lua_tinker::class_def<IXmlAttr>(L,"Next",&IXmlAttr::Next);
		lua_tinker::class_def<IXmlAttr>(L,"Prev",&IXmlAttr::Prev);
		class_set_cfun<IXmlAttr>(L,"AsInt", IXmlAttr_AsInt);
		class_set_cfun<IXmlAttr>(L,"AsUint", IXmlAttr_AsUint);
		class_set_cfun<IXmlAttr>(L,"AsFloat", IXmlAttr_AsFloat);
		class_set_cfun<IXmlAttr>(L,"AsDouble", IXmlAttr_AsDouble);
		class_set_cfun<IXmlAttr>(L,"AsBool", IXmlAttr_AsBool);


		lua_tinker::class_add<IXmlNode>(L,"IXmlNode");
		lua_tinker::class_inh<IXmlNode,IObjRef>(L);
		lua_tinker::class_def<IXmlNode>(L,"ToString",&IXmlNode::ToString);
		lua_tinker::class_def<IXmlNode>(L,"GetPrivPtr",&IXmlNode::GetPrivPtr);
		lua_tinker::class_def<IXmlNode>(L,"Empty",&IXmlNode::Empty);
		lua_tinker::class_def<IXmlNode>(L,"Name",&IXmlNode::Name);
		lua_tinker::class_def<IXmlNode>(L,"Value",&IXmlNode::Value);
		lua_tinker::class_def<IXmlNode>(L,"Text",&IXmlNode::Text);
		lua_tinker::class_def<IXmlNode>(L,"SetText",&IXmlNode::SetText);
		lua_tinker::class_def<IXmlNode>(L,"set_userdata",&IXmlNode::set_userdata);
		lua_tinker::class_def<IXmlNode>(L,"get_userdata",&IXmlNode::get_userdata);

		lua_tinker::class_def<IXmlNode>(L,"Attribute",&IXmlNode::Attribute);
		lua_tinker::class_def<IXmlNode>(L,"FirstAttribute",&IXmlNode::FirstAttribute);
		lua_tinker::class_def<IXmlNode>(L,"LastAttribute",&IXmlNode::LastAttribute);

		lua_tinker::class_def<IXmlNode>(L,"Child",&IXmlNode::Child);
		lua_tinker::class_def<IXmlNode>(L,"FirstChild",&IXmlNode::FirstChild);
		lua_tinker::class_def<IXmlNode>(L,"LastChild",&IXmlNode::LastChild);

		lua_tinker::class_def<IXmlNode>(L,"NextSibling",&IXmlNode::NextSibling);
		lua_tinker::class_def<IXmlNode>(L,"PrevSibling",&IXmlNode::PrevSibling);

		lua_tinker::class_def<IXmlNode>(L,"NextSibling2",&IXmlNode::NextSibling2);
		lua_tinker::class_def<IXmlNode>(L,"PrevSibling2",&IXmlNode::PrevSibling2);

		// mutation methods (DOM building from script)
		lua_tinker::class_def<IXmlNode>(L,"AppendChild",&IXmlNode::AppendChild);
		lua_tinker::class_def<IXmlNode>(L,"PrependChild",&IXmlNode::PrependChild);
		lua_tinker::class_def<IXmlNode>(L,"AppendCopyNode",&IXmlNode::AppendCopyNode);
		lua_tinker::class_def<IXmlNode>(L,"PrependCopyNode",&IXmlNode::PrependCopyNode);
		lua_tinker::class_def<IXmlNode>(L,"AppendAttribute",&IXmlNode::AppendAttribute);
		lua_tinker::class_def<IXmlNode>(L,"PrependAttribute",&IXmlNode::PrependAttribute);
		lua_tinker::class_def<IXmlNode>(L,"AppendCopyAttribute",&IXmlNode::AppendCopyAttribute);
		lua_tinker::class_def<IXmlNode>(L,"PrependCopyAttribute",&IXmlNode::PrependCopyAttribute);
		lua_tinker::class_def<IXmlNode>(L,"RemoveAttribute",&IXmlNode::RemoveAttribute);
		lua_tinker::class_def<IXmlNode>(L,"RemoveChild",&IXmlNode::RemoveChild);
		lua_tinker::class_def<IXmlNode>(L,"RemoveAllChilden",&IXmlNode::RemoveAllChilden);


		lua_tinker::class_add<IXmlDoc>(L,"IXmlDoc");
		lua_tinker::class_inh<IXmlDoc,IObjRef>(L);
		lua_tinker::class_def<IXmlDoc>(L,"GetPrivPtr",&IXmlDoc::GetPrivPtr);

		lua_tinker::class_def<IXmlDoc>(L,"Reset",&IXmlDoc::Reset);
		lua_tinker::class_def<IXmlDoc>(L,"Copy",&IXmlDoc::Copy);
		lua_tinker::class_def<IXmlDoc>(L,"LoadString",&IXmlDoc::LoadString);
		lua_tinker::class_def<IXmlDoc>(L,"LoadFileA",&IXmlDoc::LoadFileA);
		lua_tinker::class_def<IXmlDoc>(L,"LoadFileW",&IXmlDoc::LoadFileW);
		lua_tinker::class_def<IXmlDoc>(L,"LoadBuffer",&IXmlDoc::LoadBuffer);
		lua_tinker::class_def<IXmlDoc>(L,"LoadBufferInplace",&IXmlDoc::LoadBufferInplace);

		lua_tinker::class_def<IXmlDoc>(L,"LoadBufferInplaceOwn",&IXmlDoc::LoadBufferInplaceOwn);
		lua_tinker::class_def<IXmlDoc>(L,"GetParseResult",&IXmlDoc::GetParseResult);
		lua_tinker::class_def<IXmlDoc>(L,"SaveBinary",&IXmlDoc::SaveBinary);
		lua_tinker::class_def<IXmlDoc>(L,"SaveFileA",&IXmlDoc::SaveFileA);
		lua_tinker::class_def<IXmlDoc>(L,"SaveFileW",&IXmlDoc::SaveFileW);

		lua_tinker::class_def<IXmlDoc>(L,"Root",&IXmlDoc::Root);


        return TRUE;
    }catch(...)
    {
        return FALSE;
    }

}