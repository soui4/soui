#include <souistd.h>
#include <core/SGradient.h>

SNSBEGIN
SGradient::SGradient()
{
}

const GradientItem *SGradient::GetGradientData(CTHIS) SCONST
{
    return m_arrGradient.GetData();
}

int SGradient::GetGradientLength(CTHIS) SCONST
{
    return (int)m_arrGradient.GetCount();
}

static int GradientItemCmp(const void *_p1, const void *_p2)
{
    const GradientItem *p1 = (const GradientItem *)_p1;
    const GradientItem *p2 = (const GradientItem *)_p2;
    float diff = p1->pos - p2->pos;
    if (diff > 0.0f)
        return 1;
    else if (diff < 0.0f)
        return -1;
    else
        return 0;
}

static BOOL ParseGradientColors(const SStringW &value, SArray<GradientItem> &output)
{
    SStringWList lstInfo;
    SplitString(value, ',', lstInfo);

    for (UINT i = 0; i < lstInfo.GetCount(); i++)
    {
        SStringWList lstColorInfo;
        SplitString(lstInfo[i], '|', lstColorInfo);
        GradientItem gradient;
        if (lstColorInfo.GetCount() == 2)
        {
            gradient.cr = GETCOLOR(lstColorInfo[0]);
            gradient.pos = (float)_wtof(lstColorInfo[1]);
        }
        else
        {
            gradient.cr = GETCOLOR(lstColorInfo[0]);
            gradient.pos = 1.0f * i / (lstInfo.GetCount() - 1);
        }
        output.Add(gradient);
    }
    qsort(output.GetData(), output.GetCount(), sizeof(GradientItem), GradientItemCmp);
    if (output.GetCount() < 2)
        return FALSE;
    output[0].pos = 0.0f;
    output[output.GetCount() - 1].pos = 1.0f;
    return TRUE;
}

HRESULT SGradient::OnAttrColors(const SStringW &value, BOOL bLoading)
{
    m_arrGradient.RemoveAll();
    return ParseGradientColors(value, m_arrGradient) ? S_FALSE : E_INVALIDARG;
}

void SGradient::OnInitFinished(IXmlNode *pNode)
{
    LoadColorTable(pNode);
}

int SGradient::LoadColorTable(IXmlNode *xmlNode)
{
    IXmlNode *xmlColor = xmlNode->Child(L"item", FALSE);
    if (!xmlColor)
    {
        return 0;
    }

    m_arrGradient.RemoveAll();
    SXmlNode color(xmlColor);
    float pos = -1.f;
    while (color)
    {
        GradientItem gradient;
        gradient.cr = GETCOLOR(color.attribute(L"color").as_string());
        gradient.pos = color.attribute(L"pos").as_float();
        if (gradient.pos < pos || gradient.pos < 0.f || gradient.pos > 1.0f)
        {
            SSLOGW() << "invalid gradient item pos. pos must between 0 and 1";
        }
        else
        {
            m_arrGradient.Add(gradient);
            pos = gradient.pos;
        }
        color = color.next_sibling();
    }
    if (m_arrGradient.GetCount() < 2)
    {
        m_arrGradient.RemoveAll();
        // invalid
        SSLOGW() << "gradient color table is empty";
    }
    return (int)m_arrGradient.GetCount();
}
SNSEND
