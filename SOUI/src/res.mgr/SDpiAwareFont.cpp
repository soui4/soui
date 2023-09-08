#include "souistd.h"
#include "res.mgr/SDpiAwareFont.h"

SNSBEGIN
static const SStringW KNoneFont = L"none";
SDpiAwareFont::SDpiAwareFont(void)
    : m_strDesc(KNoneFont)
{
}

SDpiAwareFont::~SDpiAwareFont(void)
{
}

IFontS *SDpiAwareFont::GetFontPtr()
{
    return m_object;
}

void SDpiAwareFont::SetFontDesc(const SStringW &strDesc, int nScale)
{
    m_strDesc = strDesc;
    m_object = OnGetFont(m_strDesc, nScale);
}

void SDpiAwareFont::UpdateFont(int nScale)
{
    if (m_strDesc == KNoneFont)
        return;
    m_object = OnGetFont(m_strDesc, nScale);
}

IFontS *SDpiAwareFont::OnGetFont(const SStringW &strDesc, int nScale)
{
    return GETUIDEF->GetFont(strDesc, nScale);
}
SNSEND
