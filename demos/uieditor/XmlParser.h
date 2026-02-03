#pragma once

#include "spugixml/pugixml.hpp"
#include <vector>
#include <sstream>

struct NodeRange
{
	int begin;
	int _break;
	int end;
};

class StreamWrite : public spugi::xml_writer {
  public:
    virtual ~StreamWrite()
    {
    }

    // Write memory chunk into stream/file/whatever
    virtual void write(const void *data, size_t size)
    {
        m_stream.write((const char *)data, size);
    }

  public:
    std::stringstream m_stream;
};

class CXmlParser :  public spugi::xml_document, spugi::xml_parse_listener
{
public:
	CXmlParser(void);
	~CXmlParser(void);

	bool loadUtf8(const char *utf8,int nLen);

	NodeRange getNodePos(const int *nodePos,int nLen);
	NodeRange findElementRange(int pos);
	int findElementOrder(int pos,std::vector<int> &order);
	bool isRootLayout() const;
	SStringA getRootName() const;
protected:
	spugi::xml_node _findNodeRange(spugi::xml_node node,int pos);

	virtual void on_node_begin(spugi::xml_node node,int pos);

	virtual void on_node_end(spugi::xml_node node,int pos);

	virtual void on_node_free(spugi::xml_node node);

	virtual void on_node_break(xml_node node,int pos);

	const char *m_utf8Loading;
	int			m_utf8Len;
	DWORD		m_tsLoad;
};

