#include <stdio.h>
#include <gtest/gtest.h>
#include <xml/SXml.h>
#include <string/sstringa.h>
#include <string/sstringw.h>
#include <string/strcpcvt.h>
#include <helper/SRwLock.h>

using namespace SOUI;
TEST(MathTest, Addition) {
  EXPECT_EQ(2 + 2, 4);
}

TEST(XmlTest,Load){
    SXmlDoc xml;
    EXPECT_EQ(xml.load_file("/home/flyhigh/work/soui4/demo2/uires/xml/dlg_main.xml"),true);
    SXmlNode node=xml.root().first_child();
    while(node){
        const wchar_t * name = node.name();
        wprintf(L"node name=%s\n",name);
        node = node.next_sibling();
    }
}

TEST(Charset,Cvt){
    setlocale(LC_ALL, "");
    SStringA src = "再跌几个月得6.0了";
    SStringW wstr = S_CA2W(src,CP_UTF8);
    
    SStringA src2 = S_CW2A(wstr,CP_UTF8);
    bool beq = src == src2;
    
    EXPECT_EQ(beq,true);
}

TEST(Util, RwLock) {
    SRwLock lock;
    lock.LockShared();
    lock.UnlockShared();

    lock.LockExclusive();
    lock.UnlockExclusive();

    lock.LockShared();
    lock.LockShared();
    lock.UnlockShared();
    lock.UnlockShared();
    EXPECT_EQ(1, 1);
}