#ifndef SGroupList_H_
#define SGroupList_H_

SNSBEGIN

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

#ifndef EVT_GROUPLIST_BEGIN
#define EVT_GROUPLIST_BEGIN EVT_EXTERNAL_BEGIN + 3000
#endif
enum
{
    EVT_GROUPLIST_INIT_GROUP = EVT_GROUPLIST_BEGIN,
    EVT_GROUPLIST_INIT_ITEM,
    EVT_GROUPLIST_ITEM_CHECK,
    EVT_GROUPLIST_GROUP_STATE_CHANGED,
};

DEF_EVT_EXT(EventGroupListInitGroup, EVT_GROUPLIST_INIT_GROUP, {
    SWindow *pItem;
    GroupListStuct::GroupInfo *pGroupInfo;
});

DEF_EVT_EXT(EventGroupListInitItem, EVT_GROUPLIST_INIT_ITEM, {
    SWindow *pItem;
    GroupListStuct::ItemInfo *pItemInfo;
});

DEF_EVT_EXT(EventGroupListItemCheck, EVT_GROUPLIST_ITEM_CHECK, {
    SWindow *pItem;
    GroupListStuct::ItemInfo *pItemInfo;
});

DEF_EVT_EXT(EventGroupStateChanged, EVT_GROUPLIST_GROUP_STATE_CHANGED, {
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
SNSEND


#endif // SGroupList_H_