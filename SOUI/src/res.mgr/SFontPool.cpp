//////////////////////////////////////////////////////////////////////////
//  Class Name: SFontPool
// Description: Font Pool
//////////////////////////////////////////////////////////////////////////
#include "souistd.h"
#include "res.mgr/SFontPool.h"
#include "helper/SplitString.h"
#include "layout/SLayoutSize.h"
#include "helper/SHostMgr.h"

SNSBEGIN

int CALLBACK DefFontsEnumProc(CONST LOGFONT *lplf,    // logical-font data
                              CONST TEXTMETRIC *lptm, // physical-font data
                              DWORD dwType,           // font type
                              LPARAM lpData           // application-defined data
)
{
    BOOL *pbValidFont = (BOOL *)lpData;
    *pbValidFont = TRUE;
    return 0; // stop enum.
}

static BOOL DefFontCheck(const SStringW &strFontName)
{
    return HasFont(S_CW2T(strFontName));
}

FunFontCheck SFontPool::s_funFontCheck = DefFontCheck;

void SFontPool::SetFontChecker(FunFontCheck fontCheck)
{
    s_funFontCheck = fontCheck;
}

BOOL SFontPool::CheckFont(const SStringW &strFontName)
{
    return s_funFontCheck(strFontName);
}

SFontPool::SFontPool(IRenderFactory *fac)
    : m_RenderFactory(fac)
{
    m_pFunOnKeyRemoved = OnKeyRemoved;
    FontInfo emptyInfo;
    m_defFontInfo = FontInfoFromString(L"face:宋体,size:16", emptyInfo);
}

void SFontPool::SetRenderFactory(IRenderFactory *fac)
{
    m_RenderFactory = fac;
}

void SFontPool::OnKeyRemoved(const IFontPtr &obj)
{
    obj->Release();
}

static const WCHAR KFontPropSeprator = L','; //字体属性之间的分隔符，不再支持其它符号。
static const WCHAR KPropSeprator = L':';     //一个属性name:value对之间的分隔符
static const WCHAR kExPropKey[] = L"exprop";
static const WCHAR KFontFace[] = L"face";
static const WCHAR KFontBold[] = L"bold";
static const WCHAR KFontUnderline[] = L"underline";
static const WCHAR KFontItalic[] = L"italic";
static const WCHAR KFontStrike[] = L"strike";
static const WCHAR KFontAdding[] = L"adding";
static const WCHAR KFontSize[] = L"size";
static const WCHAR KFontCharset[] = L"charset";
static const WCHAR KFontWeight[] = L"weight";
static const WCHAR KFontEscapement[] = L"escapement";
const static WCHAR KDefFontFace[] = L"宋体";

IFontPtr SFontPool::_GetFont(const SStringW &strFont, int scale)
{
    SStringW strFontDesc = GETUIDEF->GetFontDesc(strFont);
    FontInfo info = FontInfoFromString(strFontDesc, m_defFontInfo);
    info.scale = scale;
    IFontPtr hftRet = NULL;
    if (HasKey(info))
    {
        hftRet = GetKeyObject(info);
    }
    else
    {
        hftRet = _CreateFont(info);
        AddKeyObject(info, hftRet);
    }
    return hftRet;
}

IFontPtr SFontPool::_CreateFont(const FontInfo &fontInfo)
{
    LOGFONT lfNew = { 0 };

    lfNew.lfCharSet = fontInfo.style.attr.byCharset;

    //优先使用weigth属性.
    lfNew.lfWeight = (long)(fontInfo.style.attr.byWeight * 4);
    if (lfNew.lfWeight == 0) //没有weight属性时检查bold属性.
        lfNew.lfWeight = (fontInfo.style.attr.fBold ? FW_BOLD : FW_NORMAL);
    lfNew.lfUnderline = (FALSE != fontInfo.style.attr.fUnderline);
    lfNew.lfItalic = (FALSE != fontInfo.style.attr.fItalic);
    lfNew.lfStrikeOut = (FALSE != fontInfo.style.attr.fStrike);
    lfNew.lfEscapement = lfNew.lfOrientation = fontInfo.style.attr.fEscapement;
    if (fontInfo.style.attr.szIsAdding)
    {
        SLayoutSize defFontSize((float)(short)m_defFontInfo.style.attr.nSize, (SLayoutSize::Unit)m_defFontInfo.style.attr.szUnit);
        lfNew.lfHeight = -defFontSize.toPixelSize(fontInfo.scale);
        SLayoutSize layoutSize((float)(short)fontInfo.style.attr.nSize, (SLayoutSize::Unit)fontInfo.style.attr.szUnit);
        lfNew.lfHeight -= layoutSize.toPixelSize(fontInfo.scale);
    }
    else
    {
        SLayoutSize layoutSize((float)(int)fontInfo.style.attr.nSize, (SLayoutSize::Unit)fontInfo.style.attr.szUnit);
        lfNew.lfHeight = -layoutSize.toPixelSize(fontInfo.scale);
    }
    lfNew.lfQuality = CLEARTYPE_QUALITY;

    _tcscpy_s(lfNew.lfFaceName, _countof(lfNew.lfFaceName), S_CW2T(fontInfo.strFaceName));
    IFontPtr ret = NULL;
    SASSERT(m_RenderFactory);
    m_RenderFactory->CreateFont(&ret, &lfNew);

    if (!fontInfo.strPropEx.IsEmpty())
    {
        SXmlDoc xmlDoc;
        xmlDoc.load_string(fontInfo.strPropEx.c_str());
        SXmlNode xmlExProp = xmlDoc.root().child(kExPropKey);
        ret->SetProp(&xmlExProp);
    }
    return ret;
}

const FontInfo &SFontPool::_GetDefFontInfo() const
{
    return m_defFontInfo;
}

void SFontPool::_SetDefFontInfo(const FontInfo &fontInfo)
{
    m_defFontInfo = fontInfo;
    RemoveAll();
    SHostMgr::getSingletonPtr()->DispatchMessage(UM_UPDATEFONT);
}

void SFontPool::_SetDefFontInfo(const SStringW &strFontInfo)
{
    if (strFontInfo.IsEmpty())
        return;
    FontInfo emptyDefFont;
    emptyDefFont.scale = 100;
    FontInfo fi = FontInfoFromString(strFontInfo, emptyDefFont);
    if (!CheckFont(fi.strFaceName))
    { // make sure default font face is valid.
        fi.strFaceName = KDefFontFace;
    }
    _SetDefFontInfo(fi);
}

FontInfo SFontPool::FontInfoFromString(const SStringW &strFontDesc, const FontInfo &defFontInfo)
{
    FontInfo fi = defFontInfo;
    SXmlDoc xmlExProp;
    SXmlNode nodeExp;
    if (fi.strPropEx.IsEmpty())
        nodeExp = xmlExProp.root().append_child(kExPropKey);
    else
    {
        xmlExProp.load_string(fi.strPropEx.c_str());
        nodeExp = xmlExProp.root().child(kExPropKey);
    }
    SStringW fontDesc2 = strFontDesc;
    SArray<SStringW> strLst;
    size_t nSeg = SplitString(fontDesc2, KFontPropSeprator, strLst);
    for (size_t i = 0; i < nSeg; i++)
    {
        SArray<SStringW> kv;
        size_t n = SplitString(strLst[i], KPropSeprator, kv);
        if (n != 2)
            continue;
        kv[0].MakeLower();
        if (kv[0] == KFontFace)
        {
            if (kv[1][0] == L'\'' || kv[1][0] == L'\"')
                fi.strFaceName = kv[1].Mid(1, kv[1].GetLength() - 2);
            else
                fi.strFaceName = kv[1];
        }
        else if (kv[0] == KFontSize)
        {
            SLayoutSize layoutSize;
            layoutSize.parseString(kv[1]);
            fi.style.attr.nSize = (short)abs(layoutSize.fSize);
            fi.style.attr.szUnit = (int)layoutSize.unit;
            fi.style.attr.szIsAdding = 0;
        }
        else if (kv[0] == KFontAdding)
        {
            SLayoutSize layoutSize;
            layoutSize.parseString(kv[1]);
            fi.style.attr.nSize = (short)layoutSize.fSize;
            fi.style.attr.szUnit = (int)layoutSize.unit;
            fi.style.attr.szIsAdding = 1;
        }
        else if (kv[0] == KFontCharset)
        {
            fi.style.attr.byCharset = _wtoi(kv[1]);
        }
        else if (kv[0] == KFontWeight)
        {
            fi.style.attr.byWeight = (_wtoi(kv[1]) + 2) / 4; //+2 for 四舍五入. /4是为了把weight scale到0-250.
        }
        else if (kv[0] == KFontBold)
        {
            fi.style.attr.fBold = _wtoi(kv[1]);
        }
        else if (kv[0] == KFontItalic)
        {
            fi.style.attr.fItalic = _wtoi(kv[1]);
        }
        else if (kv[0] == KFontStrike)
        {
            fi.style.attr.fStrike = _wtoi(kv[1]);
        }
        else if (kv[0] == KFontUnderline)
        {
            fi.style.attr.fUnderline = _wtoi(kv[1]);
        }
        else if (kv[0] == KFontEscapement)
        {
            int fescapement = (int)(_wtof(kv[1]) * 10);
            fescapement %= 3600;
            if (fescapement < 0)
                fescapement += 3600;
            // make sure fescapement is between [0,3600)
            fi.style.attr.fEscapement = fescapement;
        }
        else
        { // extend props
            nodeExp.append_attribute(kv[0].c_str()).set_value(kv[1].c_str());
        }
    }
    if (nodeExp.first_attribute())
    {
        nodeExp.ToString(&fi.strPropEx);
    }
    return fi;
}

SStringW SFontPool::FontInfoToString(const FontInfo &fi)
{
    char szBuf[200];
    SLogStream s(szBuf, 200);
    s << KFontFace << KPropSeprator << "\'" << fi.strFaceName.c_str() << "\'"
      << ",";
    s << KFontSize << KPropSeprator << (short)fi.style.attr.nSize << ",";
    s << KFontCharset << KPropSeprator << fi.style.attr.byCharset << ",";
    s << KFontWeight << KPropSeprator << fi.style.attr.byWeight * 4 << ",";
    s << KFontBold << KPropSeprator << (fi.style.attr.fBold ? "1" : "0") << ",";
    s << KFontItalic << KPropSeprator << (fi.style.attr.fItalic ? "1" : "0") << ",";
    s << KFontStrike << KPropSeprator << (fi.style.attr.fStrike ? "1" : "0") << ",";
    s << KFontUnderline << KPropSeprator << (fi.style.attr.fUnderline ? "1" : "0");
    s << KFontEscapement << KPropSeprator << fi.style.attr.fEscapement;

    return S_CA2W(szBuf);
}

IFontPtr SFontPool::GetFont(const SStringW &strFont, int scale)
{
    return GETUIDEF->GetFont(strFont, scale);
}

void SFontPool::SetDefFontInfo(const SStringW &strFontInfo)
{
    return GETUIDEF->SetDefFontInfo(strFontInfo);
}

FontInfo SFontPool::GetDefFontInfo()
{
    return GETUIDEF->GetDefFontInfo();
}

SNSEND