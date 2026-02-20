#ifndef _SCHECKGROUP_H_
#define _SCHECKGROUP_H_

SNSBEGIN

#ifndef EVT_GROUPCHECK_BEGIN
#define EVT_GROUPCHECK_BEGIN EVT_EXTERNAL_BEGIN + 500
#endif
enum
{
    EVT_GROUP_CHECK = EVT_GROUPCHECK_BEGIN,
};

DEF_EVT_EXT(EventGroupCheck, EVT_GROUP_CHECK, { int iItem; });

namespace SCheckGroup_style
{
    static const LPCWSTR kStyle_template = L"template";
    static const LPCWSTR kStyle_wndclass = L"wndclass";
}

struct SCheckItem
{
    BOOL bChecked;
    SStringT strText;
};

class SCheckGroup : public SWindow {
    DEF_SOBJECT(SWindow, L"checkgroup")
  public:
    SCheckGroup(void);
    ~SCheckGroup(void);

    void SetOptions(const SCheckItem * vOptions, size_t nCount);

  protected:
    BOOL CreateChildren(SXmlNode xmlNode) override;
    BOOL OnCheckChange(IEvtArgs *e);
  protected:
    SXmlDoc m_xmlTemplate;
    int     m_checkId;
};
SNSEND
#endif //_SCHECKGROUP_H_
