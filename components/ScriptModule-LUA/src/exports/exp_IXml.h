#include <interface/sxml-i.h>
using namespace pugi;

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

        
		lua_tinker::class_add<IXmlNode>(L,"IXmlNode");
		lua_tinker::class_inh<IXmlNode,IObjRef>(L);
		lua_tinker::class_def<IXmlNode>(L,"ToString",&IXmlNode::ToString);
		lua_tinker::class_def<IXmlNode>(L,"GetPrivPtr",&IXmlNode::GetPrivPtr);
		lua_tinker::class_def<IXmlNode>(L,"Empty",&IXmlNode::Empty);
		lua_tinker::class_def<IXmlNode>(L,"Name",&IXmlNode::Name);
		lua_tinker::class_def<IXmlNode>(L,"Value",&IXmlNode::Value);
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