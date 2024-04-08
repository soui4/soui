#include "include/souistd.h"
#include "res.mgr/SUiDef.h"
#include "helper/SplitString.h"
#include "helper/SAutoBuf.h"

SNSBEGIN

const static WCHAR KNodeUidef[] = L"uidef";
const static WCHAR KNodeDefUnit[] = L"unit";
const static WCHAR KNodeCaret[] = L"caret";
const static WCHAR KNodeDefFont[] = L"deffont";
const static WCHAR KNodeFont[] = L"font";
const static WCHAR KNodeColor[] = L"color";
const static WCHAR KNodeDim[] = L"dim";
const static WCHAR KNodeSkin[] = L"skin";
const static WCHAR KNodeString[] = L"string";
const static WCHAR KNodeStyle[] = L"style";
const static WCHAR KNodeTemplate[] = L"template";
const static WCHAR KNodeObjAttr[] = L"objattr";
const static WCHAR KNodeGradient[] = L"gradient";

static SXmlNode GetSourceXmlNode(SXmlNode nodeRoot, SXmlDoc &docInit, IResProvider *pResProvider, const wchar_t *pszName)
{
    SXmlNode nodeData = nodeRoot.child(pszName, false);
    if (nodeData)
    {
        SXmlAttr attrSrc = nodeData.attribute(L"src", false);
        if (attrSrc)
        { //优先从src属性里获取数据
            if (SApplication::getSingletonPtr()->LoadXmlDocment(docInit, S_CW2T(attrSrc.as_string()), pResProvider))
            {
                nodeData = docInit.root().child(pszName);
            }
        }
    }
    return nodeData;
}

class SUiDefInfo : public TObjRefImpl<IUiDefInfo> {
    friend class SUiDef;

  private:
    SUiDefInfo()
    {
    }

  public:
    UINT Init(IResProvider *pResProvide, LPCTSTR pszUidef) override;
    UINT Init2(IXmlNode *pNode, BOOL bGlobalDomain, IResProvider *pResProvider = NULL) override;
    SSkinPool *GetSkinPool() override;
    SStylePool *GetStylePool() override;
    STemplatePool *GetTemplatePool() override;
    SGradientPool *GetGradientPool() override;
    SObjDefAttr *GetObjDefAttr() override;
    SNamedColor &GetNamedColor() override;
    SNamedString &GetNamedString() override;
    SNamedDimension &GetNamedDimension() override;
    SNamedFont &GetNamedFont() override;
    SStringW GetDefFontInfo() override;
    SXmlNode GetCaretInfo() override;

  protected:
    SAutoRefPtr<SSkinPool> pSkinPool;
    SAutoRefPtr<SStylePool> pStylePool;
    SAutoRefPtr<SObjDefAttr> objDefAttr;
    SAutoRefPtr<STemplatePool> templatePool;
    SAutoRefPtr<SGradientPool> gradientPool;
    SNamedColor namedColor;
    SNamedString namedString;
    SNamedDimension namedDim;
    SNamedFont namedFont;

    SStringW defFontInfo;
    SXmlDoc xmlCaret;
};

UINT SUiDefInfo::Init(IResProvider *pResProvider, LPCTSTR pszUidef)
{
    UINT bRet = 0;
    SStringTList strUiDef;
    if (2 != ParseResID(pszUidef, strUiDef))
    {
        SSLOGW() << "warning!!!! Add ResProvider Error.";
    }

    size_t dwSize = pResProvider->GetRawBufferSize(strUiDef[0], strUiDef[1]);
    if (dwSize == 0)
    {
        SSLOGW() << "warning!!!! uidef was not found in the specified resprovider";
    }
    else
    {
        SXmlDoc docInit;
        SAutoBuf strXml;
        strXml.Allocate(dwSize);

        pResProvider->GetRawBuffer(strUiDef[0], strUiDef[1], strXml, dwSize);

        bool bLoad = docInit.load_buffer(strXml, strXml.size(), xml_parse_default, enc_auto);

        if (!bLoad)
        { // load xml failed
            SSLOGD() << "warning!!! load uidef as xml document failed";
        }
        else
        { // init named objects
            SXmlNode root = docInit.root().child(KNodeUidef, false);
            if (!root)
            {
                SSLOGD() << "warning!!! \"uidef\" element is not the root element of uidef xml";
            }
            else
            {
                bRet = Init2(&root, TRUE, pResProvider);
            }
        }
    }
    return bRet;
}

UINT SUiDefInfo::Init2(IXmlNode *pNode, BOOL bGlobalDomain, IResProvider *pResProvider)
{
    UINT uRet = 0;
    GETUIDEF->PushUiDefInfo(this, TRUE); // make it possible for a uidef element to ref other uidef element in the same package.

    SXmlNode root(pNode);
    if (bGlobalDomain)
    {
        // parse default Unit
        SXmlNode xmlUnit;
        xmlUnit = root.child(KNodeDefUnit);
        if (xmlUnit)
        {
            SStringW strUnit = xmlUnit.attribute(L"defUnit").as_string(L"px");
            SLayoutSize::Unit unit = SLayoutSize::unitFromString(strUnit);
            if (unit != SLayoutSize::unknow)
                SLayoutSize::defUnit = unit;
        }

        xmlCaret.Reset();
        SXmlNode xmlCaretNode = root.child(KNodeCaret);
        if (xmlCaretNode)
            xmlCaret.root().append_copy(xmlCaretNode);

        // parse default font
        SXmlNode xmlFont;
        xmlFont = root.child(KNodeDefFont);
        if (xmlFont)
        {
            defFontInfo = xmlFont.attribute(L"value").as_string();
        }
        else
        {
            xmlFont = root.child(KNodeFont); // compatible to 4.4
            SXmlAttr attr = xmlFont.first_attribute();
            while (attr)
            {
                if (!defFontInfo.IsEmpty())
                    defFontInfo += L",";
                if (SStringW(attr.name()).CompareNoCase(L"face") == 0)
                {
                    defFontInfo += SStringW().Format(L"face:\'%s\'", attr.value());
                }
                else
                    defFontInfo += SStringW().Format(L"%s:%s", attr.name(), attr.value());
                attr = attr.next_attribute();
            }
        }

        // load SWindow default attribute
        SXmlDoc docData;
        SXmlNode nodeData = GetSourceXmlNode(root, docData, pResProvider, KNodeObjAttr);
        if (nodeData)
        {
            objDefAttr.Attach(new SObjDefAttr);
            objDefAttr->Init(nodeData);
            nodeData.set_userdata(1);
        }

        uRet |= UDI_GLOBAL;
    }
    // load named string
    {
        SXmlDoc docData;
        SXmlNode nodeData = GetSourceXmlNode(root, docData, pResProvider, KNodeString);
        if (nodeData)
        {
            namedString.Init(nodeData);
            nodeData.set_userdata(1);
            uRet |= UDI_STRING;
        }
    }

    // load named color
    {
        SXmlDoc docData;
        SXmlNode nodeData = GetSourceXmlNode(root, docData, pResProvider, KNodeColor);
        if (nodeData)
        {
            namedColor.Init(nodeData);
            nodeData.set_userdata(1);
            uRet |= UDI_COLOR;
        }
    }

    // load named color
    {
        SXmlDoc docData;
        SXmlNode nodeData = GetSourceXmlNode(root, docData, pResProvider, KNodeGradient);
        if (nodeData)
        {
            gradientPool.Attach(new SGradientPool);
            gradientPool->Init(nodeData);
            nodeData.set_userdata(1);
            uRet |= UDI_GRADIENT;
        }
    }
    // load named dimension
    {
        SXmlDoc docData;
        SXmlNode nodeData = GetSourceXmlNode(root, docData, pResProvider, KNodeDim);
        if (nodeData)
        {
            namedDim.Init(nodeData);
            nodeData.set_userdata(1);
            uRet |= UDI_DIMENSION;
        }
    }
    // load named font
    {
        SXmlDoc docData;
        SXmlNode nodeData = GetSourceXmlNode(root, docData, pResProvider, KNodeFont);
        if (nodeData)
        {
            namedFont.Init(nodeData);
            nodeData.set_userdata(1);
            uRet |= UDI_FONT;
        }
    }
    // load named skin
    {
        SXmlDoc docData;
        SXmlNode nodeData = GetSourceXmlNode(root, docData, pResProvider, KNodeSkin);
        if (nodeData)
        {
            pSkinPool.Attach(new SSkinPool(TRUE));
            pSkinPool->LoadSkins(&nodeData);
            nodeData.set_userdata(1);
            uRet |= UDI_SKIN;
        }
    }
    // load named style
    {
        SXmlDoc docData;
        SXmlNode nodeData = GetSourceXmlNode(root, docData, pResProvider, KNodeStyle);
        if (nodeData)
        {
            pStylePool.Attach(new SStylePool);
            pStylePool->Init(nodeData);
            nodeData.set_userdata(1);
            uRet |= UDI_STYLE;
        }
    }
    // load named template
    {
        SXmlDoc docData;
        SXmlNode nodeData = GetSourceXmlNode(root, docData, pResProvider, KNodeTemplate);
        if (nodeData)
        {
            templatePool.Attach(new STemplatePool);
            templatePool->Init(nodeData);
            nodeData.set_userdata(1);
            uRet |= UDI_TEMPLATE;
        }
    }
    GETUIDEF->PopUiDefInfo(this, TRUE);

    return uRet;
}

SSkinPool *SUiDefInfo::GetSkinPool()
{
    return pSkinPool;
}
SStylePool *SUiDefInfo::GetStylePool()
{
    return pStylePool;
}
SNamedColor &SUiDefInfo::GetNamedColor()
{
    return namedColor;
}
SNamedString &SUiDefInfo::GetNamedString()
{
    return namedString;
}
SNamedDimension &SUiDefInfo::GetNamedDimension()
{
    return namedDim;
}

SNamedFont &SUiDefInfo::GetNamedFont()
{
    return namedFont;
}

SObjDefAttr *SUiDefInfo::GetObjDefAttr()
{
    return objDefAttr;
}
SStringW SUiDefInfo::GetDefFontInfo()
{
    return defFontInfo;
}

SXmlNode SUiDefInfo::GetCaretInfo()
{
    return xmlCaret.root().child(L"caret");
}

STemplatePool *SUiDefInfo::GetTemplatePool()
{
    return templatePool;
}

SGradientPool *SUiDefInfo::GetGradientPool()
{
    return gradientPool;
}
//////////////////////////////////////////////////////////////////////////
SUiDef::SUiDef(IRenderFactory *fac)
    : SFontPool(fac)
{
    SAutoLock autolock(m_cs);
    IUiDefInfo *emptyUiInfo = CreateUiDefInfo();
    SetUiDef(emptyUiInfo, false); // set empty uiinfo
    emptyUiInfo->Release();

    m_bulitinSkinPool.Attach(new SSkinPool(TRUE));
    m_lstSkinPools.AddTail(m_bulitinSkinPool);
    m_bulitinSkinPool->AddRef();
}

SUiDef::~SUiDef(void)
{
    SAutoLock autolock(m_cs);
    {
        SPOSITION pos = m_lstUiDefInfo.GetHeadPosition();
        while (pos)
        {
            IObjRef *p = m_lstUiDefInfo.GetNext(pos);
            p->Release();
        }
        m_lstUiDefInfo.RemoveAll();
        m_defUiDefInfo = NULL;
    }
    {
        SPOSITION pos = m_lstSkinPools.GetHeadPosition();
        while (pos)
        {
            ISkinPool *p = m_lstSkinPools.GetNext(pos);
            p->Release();
        }
        m_lstSkinPools.RemoveAll();
        m_bulitinSkinPool = NULL;
    }
}

BOOL SUiDef::InitDefUiDef(IResProvider *pResProvider, LPCTSTR pszUiDef)
{
    IUiDefInfo *pRet = CreateUiDefInfo();
    UINT uRet = pRet->Init(pResProvider, pszUiDef);
    if (uRet == 0)
    {
        pRet->Release();
        return FALSE;
    }
    SetUiDef(pRet, true);
    pRet->Release();
    return TRUE;
}

IUiDefInfo *SUiDef::GetUiDef()
{
    SAutoLock autolock(m_cs);
    return m_defUiDefInfo;
}

void SUiDef::SetUiDef(IUiDefInfo *pUiDefInfo, bool bUpdateDefFont)
{
    SAutoLock autolock(m_cs);
    SASSERT(pUiDefInfo);
    if (m_defUiDefInfo)
    {
        SASSERT(m_lstUiDefInfo.GetHead() == m_defUiDefInfo);
        IUiDefInfo *defUI = m_lstUiDefInfo.RemoveHead();
        defUI->Release();
        m_defUiDefInfo = NULL;
    }
    m_defUiDefInfo = pUiDefInfo;
    // add to uidef list.
    m_lstUiDefInfo.AddHead(pUiDefInfo);
    pUiDefInfo->AddRef();

    SetDefFontInfo(m_defUiDefInfo->GetDefFontInfo());
}

void SUiDef::PushUiDefInfo(IUiDefInfo *pUiDefInfo, BOOL bPreivate)
{
    SAutoLock autolock(m_cs);
    m_lstUiDefInfo.AddTail(pUiDefInfo);
    pUiDefInfo->AddRef();
    if (bPreivate)
        m_cs.Enter();
}

BOOL SUiDef::PopUiDefInfo(IUiDefInfo *pUiDefInfo, BOOL bPreivate)
{
    SAutoLock autolock(m_cs);
    if (bPreivate)
        m_cs.Leave();
    if (!pUiDefInfo)
    {
        if (m_lstUiDefInfo.IsEmpty())
            return FALSE;
        pUiDefInfo = m_lstUiDefInfo.RemoveTail();
        pUiDefInfo->Release();
        return TRUE;
    }
    else
    {
        SPOSITION pos = m_lstUiDefInfo.Find(pUiDefInfo);
        if (!pos)
            return FALSE;
        m_lstUiDefInfo.RemoveAt(pos);
        pUiDefInfo->Release();
        return TRUE;
    }
}

void SUiDef::PushSkinPool(ISkinPool *pSkinPool)
{
    SAutoLock autolock(m_cs);
    if (pSkinPool == m_bulitinSkinPool)
        return;
    m_lstSkinPools.AddTail(pSkinPool);
    pSkinPool->AddRef();
}

BOOL SUiDef::PopSkinPool(ISkinPool *pSkinPool)
{
    SAutoLock autolock(m_cs);
    if (pSkinPool == m_bulitinSkinPool)
        return FALSE;
    SPOSITION pos = m_lstSkinPools.Find(pSkinPool);
    if (!pos)
        return FALSE;

    m_lstSkinPools.RemoveAt(pos);
    pSkinPool->Release();
    return TRUE;
}

ISkinObj *SUiDef::GetSkin(const SStringW &strSkinName, int nScale)
{
    SAutoLock autolock(m_cs);
    {
        SPOSITION pos = m_lstSkinPools.GetTailPosition();
        while (pos)
        {
            ISkinPool *pSkinPool = m_lstSkinPools.GetPrev(pos);
            if (ISkinObj *pSkin = pSkinPool->GetSkin(strSkinName, nScale))
            {
                return pSkin;
            }
        }
    }
    {
        SPOSITION pos = m_lstUiDefInfo.GetTailPosition();
        while (pos)
        {
            IUiDefInfo *pUiInfo = m_lstUiDefInfo.GetPrev(pos);
            SSkinPool *pSkinPool = pUiInfo->GetSkinPool();
            if (!pSkinPool)
                continue;
            if (ISkinObj *pSkin = pSkinPool->GetSkin(strSkinName, nScale))
            {
                return pSkin;
            }
        }
    }

    if (wcscmp(strSkinName, L"") != 0)
    {
        SASSERT_FMTW(FALSE, L"GetSkin[%s] Failed!", strSkinName.c_str());
    }
    return NULL;
}

static const wchar_t *BUILDIN_SKIN_NAMES[] = {
    L"_skin.sys.checkbox", L"_skin.sys.radio", L"_skin.sys.focuscheckbox", L"_skin.sys.focusradio", L"_skin.sys.btn.normal", L"_skin.sys.scrollbar", L"_skin.sys.border", L"_skin.sys.dropbtn", L"_skin.sys.tree.toggle", L"_skin.sys.tree.checkbox", L"_skin.sys.tree.lines", L"_skin.sys.tab.page", L"_skin.sys.header", L"_skin.sys.split.vert", L"_skin.sys.split.horz", L"_skin.sys.prog.bkgnd", L"_skin.sys.prog.bar", L"_skin.sys.vert.prog.bkgnd", L"_skin.sys.vert.prog.bar", L"_skin.sys.slider.thumb", L"_skin.sys.btn.close", L"_skin.sys.btn.minimize", L"_skin.sys.btn.maxmize", L"_skin.sys.btn.restore", L"_skin.sys.menu.check", L"_skin.sys.menu.sep", L"_skin.sys.menu.arrow", L"_skin.sys.menu.border", L"_skin.sys.menu.skin", L"_skin.sys.icons", L"_skin.sys.wnd.bkgnd", L"_skin.sys.btn.prev", L"_skin.sys.btn.next", L"_skin.sys.spin.down", L"_skin.sys.spin.up",
};

ISkinObj *SUiDef::GetBuiltinSkin(SYS_SKIN uID, int nScale)
{
    SAutoLock autolock(m_cs);
    return GetBuiltinSkinPool()->GetSkin(BUILDIN_SKIN_NAMES[uID], nScale);
}

ISkinPool *SUiDef::GetBuiltinSkinPool()
{
    SAutoLock autolock(m_cs);
    return m_bulitinSkinPool;
}

SXmlNode SUiDef::GetStyle(const SStringW &strName)
{
    SAutoLock autolock(m_cs);
    SPOSITION pos = m_lstUiDefInfo.GetTailPosition();
    while (pos)
    {
        IUiDefInfo *pUiInfo = m_lstUiDefInfo.GetPrev(pos);
        SStylePool *pStylePool = pUiInfo->GetStylePool();
        if (!pStylePool)
            continue;
        SXmlNode style = pStylePool->GetStyle(strName);
        if (style)
            return style;
    }
    return SXmlNode();
}

SStringW SUiDef::GetTemplateString(const SStringW &strName)
{
    SAutoLock autolock(m_cs);
    SPOSITION pos = m_lstUiDefInfo.GetTailPosition();
    while (pos)
    {
        IUiDefInfo *pUiInfo = m_lstUiDefInfo.GetPrev(pos);
        STemplatePool *pPool = pUiInfo->GetTemplatePool();
        if (!pPool)
            continue;
        SStringW strRet = pPool->GetTemplateString(strName);
        if (!strRet.IsEmpty())
            return strRet;
    }
    return SStringW();
}

IGradient *SUiDef::GetGradient(const SStringW &strName)
{
    SAutoLock autolock(m_cs);
    SPOSITION pos = m_lstUiDefInfo.GetTailPosition();
    while (pos)
    {
        IUiDefInfo *pUiInfo = m_lstUiDefInfo.GetPrev(pos);
        SGradientPool *pPool = pUiInfo->GetGradientPool();
        if (!pPool)
            continue;
        IGradient *pRet = pPool->GetGradient(strName);
        if (pRet)
            return pRet;
    }
    return NULL;
}

COLORREF SUiDef::GetColor(const SStringW &strColor)
{
    SAutoLock autolock(m_cs);
    COLORREF cr = CR_INVALID;
    SPOSITION pos = m_lstUiDefInfo.GetTailPosition();
    while (pos)
    {
        IUiDefInfo *pUiDefInfo = m_lstUiDefInfo.GetPrev(pos);
        if (pUiDefInfo->GetNamedColor().Get(strColor, cr))
            return cr;
    }
    return cr;
}

COLORREF SUiDef::GetColor(int idx)
{
    SAutoLock autolock(m_cs);
    return GetUiDef()->GetNamedColor().Get(idx);
}

SStringW SUiDef::GetString(const SStringW &strString)
{
    SAutoLock autolock(m_cs);
    SStringW ret;
    SPOSITION pos = m_lstUiDefInfo.GetTailPosition();
    while (pos)
    {
        IUiDefInfo *pUiDefInfo = m_lstUiDefInfo.GetPrev(pos);
        if (pUiDefInfo->GetNamedString().Get(strString, ret))
            return ret;
    }
    ret = strString;
    return ret;
}

SStringW SUiDef::GetString(int idx)
{
    SAutoLock autolock(m_cs);
    return GetUiDef()->GetNamedString().Get(idx);
}

SLayoutSize SUiDef::GetLayoutSize(const SStringW &strSize)
{
    SAutoLock autolock(m_cs);
    SLayoutSize ret;
    SPOSITION pos = m_lstUiDefInfo.GetTailPosition();
    while (pos)
    {
        IUiDefInfo *pUiDefInfo = m_lstUiDefInfo.GetPrev(pos);
        if (pUiDefInfo->GetNamedDimension().Get(strSize, ret))
            return ret;
    }
    return ret;
}

SLayoutSize SUiDef::GetLayoutSize(int idx)
{
    SAutoLock autolock(m_cs);
    return GetUiDef()->GetNamedDimension().Get(idx);
}

SStringW SUiDef::GetFontDesc(const SStringW &strFont)
{
    SAutoLock autolock(m_cs);
    SStringW ret;
    SPOSITION pos = m_lstUiDefInfo.GetTailPosition();
    while (pos)
    {
        IUiDefInfo *pUiDefInfo = m_lstUiDefInfo.GetPrev(pos);
        if (pUiDefInfo->GetNamedFont().Get(strFont, ret))
            return ret;
    }
    ret = strFont;
    return ret;
}

SStringW SUiDef::GetFontDesc(int idx)
{
    SAutoLock autolock(m_cs);
    return GetUiDef()->GetNamedFont().Get(idx);
}

IUiDefInfo *SUiDef::CreateUiDefInfo()
{
    return new SUiDefInfo();
}

ISkinPool *SUiDef::CreateSkinPool(BOOL bAutoScale)
{
    return new SSkinPool(bAutoScale);
}

IFontPtr SUiDef::GetFont(const SStringW &strFont, int scale)
{
    SAutoLock lock(m_cs);
    return SFontPool::_GetFont(strFont, scale);
}

void SUiDef::SetDefFontInfo(const SStringW &strFontInfo)
{
    SAutoLock lock(m_cs);
    SFontPool::_SetDefFontInfo(strFontInfo);
}

FontInfo SUiDef::GetDefFontInfo() const
{
    SAutoLock lock(m_cs);
    return SFontPool::_GetDefFontInfo();
}

SNSEND
