#include <stdio.h>
#include <gtest/gtest.h>
#include <xml/SXml.h>
using namespace SOUI;
TEST(MathTest, Addition) {
  EXPECT_EQ(2 + 2, 4);
}

TEST(XmlTest,Load){
    SXmlDoc xml;
    EXPECT_EQ(xml.load_file("./demo2/uires/xml/dlg_main.xml"),true);
    SXmlNode node=xml.root().first_child();
    while(node){
        const wchar_t * name = node.name();
        wprintf(L"node name=%s\n",name);
        node = node.next_sibling();
    }
}