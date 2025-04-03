#pragma once

#include "spugixml/pugixml.hpp"

struct NodeRange
{
	int begin;
	int _break;
	int end;
};

class CXmlParser :  public spugi::xml_document, spugi::xml_parse_listener
{
public:
	CXmlParser(void);
	~CXmlParser(void);

	bool loadUtf8(const char *utf8,int nLen);

	NodeRange getNodePos(int *nodePos,int nLen);

	bool isRootLayout() const;
	SStringA getRootName() const;
protected:
	virtual void on_node_begin(spugi::xml_node node,int pos);

	virtual void on_node_end(spugi::xml_node node,int pos);

	virtual void on_node_free(spugi::xml_node node);

	virtual void on_node_break(xml_node node,int pos);

	const char *m_utf8Loading;
	int			m_utf8Len;
	DWORD		m_tsLoad;
};

