#include "stdafx.h"
#include "XmlParser.h"
using namespace spugi;

CXmlParser::CXmlParser(void):m_utf8Loading(NULL),m_utf8Len(0),m_tsLoad(0)
{
	set_parse_listener(this);
}

CXmlParser::~CXmlParser(void)
{
}


bool CXmlParser::loadUtf8(const char *utf8,int nLen)
{
	m_utf8Loading = utf8;
	m_utf8Len = nLen;
	xml_parse_result res= load_buffer(utf8,nLen,parse_default,encoding_utf8);
	
	m_utf8Loading = NULL;
	m_utf8Len = 0;
	if(res)
	{
		m_tsLoad = GetTickCount();
	}
	return res;
}

NodeRange CXmlParser::getNodePos(int *nodePos,int nLen)
{
	xml_node selNode = root();
	for(int i=0;i<nLen;i++)
	{
		xml_node child = selNode.first_child();
		while(child && child.type()!=spugi::node_element){
			child = child.next_sibling();
		}
		for(int j=0;j<nodePos[i];j++)
		{
			do{
				child = child.next_sibling();
			}while(child && child.type()!=spugi::node_element);
		}
		selNode = child;
		if(!selNode)
			break;
	}
	NodeRange nRet={-1,-1,-1};
	if(selNode)
	{
		NodeRange *pRange = (NodeRange*)selNode.get_userdata();
		if(pRange)
		{
			nRet = *pRange;
		}
	}
	return nRet;
}

void CXmlParser::on_node_begin(spugi::xml_node node,int pos)
{
	if(node.type() == spugi::node_element)
	{
		NodeRange *pRange = new NodeRange;
		pRange->begin = pos;
		pRange->_break = pRange->end = -1;
		node.set_userdata((ULONG_PTR)pRange);
	}
}

void CXmlParser::on_node_break(xml_node node,int pos)
{
	if(node.type() == spugi::node_element)
	{
		NodeRange *pRange =(NodeRange *)node.get_userdata();
		pRange->_break = pos;
		SStringA strA = SStringA(m_utf8Loading+pRange->begin,pRange->_break-pRange->begin);
		SStringW strW = S_CA2W(strA,CP_UTF8);
		//SLOG_INFO("node:"<<strW.c_str());
	}
}

void CXmlParser::on_node_end(spugi::xml_node node,int pos)
{
	if(node.type() == spugi::node_element)
	{
		NodeRange *pRange =(NodeRange *)node.get_userdata();
		pRange->end = pos;
		SStringA strA = SStringA(m_utf8Loading+pRange->begin,pRange->end-pRange->begin);
		SStringW strW = S_CA2W(strA,CP_UTF8);
		//SLOG_INFO("node:"<<strW.c_str());
	}
}

void CXmlParser::on_node_free(spugi::xml_node node)
{
	if(node.type() == spugi::node_element)
	{
		NodeRange *pRange = (NodeRange *)node.get_userdata();
		if(pRange) delete pRange;
	}
}

bool CXmlParser::isRootLayout() const
{
	if(!first_child())
		return false;
	return stricmp(first_child().name(),"soui")==0;
}

SOUI::SStringA CXmlParser::getRootName() const
{
	SStringA ret;
	if(first_child()) ret = first_child().name();
	ret.MakeLower();
	return ret;
}
