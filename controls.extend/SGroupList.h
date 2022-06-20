#pragma once

namespace SOUI
{
namespace GroupListStuct
{
struct ItemInfo
{
    SStringT strText;
    int iIcon;
    SStringW strName;
    int id;
};

struct GroupInfo : ItemInfo
{
    bool bCollapsed;
};
} // namespace GroupListStuct

DEF_EVT_EXT(EventGroupListInitGroup, EVT_EXTERNAL_BEGIN + 3000, {
    SWindow *pItem;
    GroupListStuct::GroupInfo *pGroupInfo;
});

DEF_EVT_EXT(EventGroupListInitItem, EVT_EXTERNAL_BEGIN + 3001, {
    SWindow *pItem;
    GroupListStuct::ItemInfo *pItemInfo;
});

DEF_EVT_EXT(EventGroupListItemCheck, EVT_EXTERNAL_BEGIN + 3002, {
    SWindow *pItem;
    GroupListStuct::ItemInfo *pItemInfo;
});

DEF_EVT_EXT(EventGroupStateChanged, EVT_EXTERNAL_BEGIN + 3003, {
    SWindow *pItem;
    GroupListStuct::GroupInfo *pGroupInfo;
});

class SGroupList : public SScrollView {
    DEF_SOBJECT(SScrollView, L"groupList")
  public:
    SGroupList();
    ~SGroupList();

    bool SelectPage(int nID);
    SWindow *InsertGroup(int iGroup,
                         const SStringW &name,
                         const SStringT &text,
                         int iIcon,
                         int nID,
                         const SStringW &strTip);
    SWindow *InsertItem(SWindow *pGroup,
                        int iItem,
                        const SStringW &name,
                        const SStringT &text,
                        int iIcon,
                        int nID,
                        const SStringW &strTip);

  protected:
    BOOL OnGroupClick(IEvtArgs *e);
    BOOL OnItemClick(IEvtArgs *e);

    virtual BOOL CreateChildren(SXmlNode xmlNode);

  protected:
    void OnDestroy();
    SOUI_MSG_MAP_BEGIN()
    MSG_WM_DESTROY(OnDestroy)
    SOUI_MSG_MAP_END()

  protected:
    SXmlDoc m_xmlDoc;
    SXmlNode m_groupTemplate;
    SXmlNode m_itemTemplate;

    SWindow *m_pSelectedItem;
};
} // namespace SOUI
