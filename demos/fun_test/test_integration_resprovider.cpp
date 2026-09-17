#include <souistd.h>
#include <commgr2.h>
#include <xml/SXml.h>
#include <resprovider-zip/zipresprovider-param.h>
#include <gtest/gtest.h>
#include "common.h"

using namespace SOUI;

TEST(soui_component, resprovider_zip_load_and_read)
{
    SStringT srcDir = getSourceDir();
    SComMgr2 comMgr;
    SAutoRefPtr<IResProvider> zipProvider;
    ASSERT_TRUE(comMgr.CreateResProvider_ZIP((IObjRef **)&zipProvider));
    ZIPRES_PARAM param;
    SStringT strZip = srcDir + _T("/uires.zip");
    ZipFile(&param, NULL, strZip.c_str(), "souizip");
    ASSERT_TRUE(zipProvider->Init((WPARAM)&param, 0));

    size_t szBuf = zipProvider->GetRawBufferSize(NULL, _T("uires.idx"));
    EXPECT_TRUE(szBuf > 0);
    SAutoBuf buf;
    char *pBuf = buf.Allocate(szBuf);
    zipProvider->GetRawBuffer(NULL, _T("uires.idx"), pBuf, szBuf);
    SXmlDoc xmlDoc;
    EXPECT_TRUE(xmlDoc.LoadBuffer(pBuf, szBuf, 0, enc_utf8));

    HICON hIcon = zipProvider->LoadIcon(_T("ICON_LOGO"), 64, 64);
    EXPECT_TRUE(hIcon != NULL);
    if (hIcon)
        DestroyIcon(hIcon);
}
