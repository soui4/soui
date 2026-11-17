#pragma once

#include <algorithm>
#include <vector>

#include <souistd.h>
#include <control/SComboView.h>
#include <control/SListCtrl.h>
#include <control/SListView.h>
#include <control/SMCListView.h>
#include <control/STileView.h>
#include <control/STreeView.h>
#include <helper/SAdapterBase.h>

namespace Gallery
{
namespace AdapterDetail
{
struct Row
{
    SOUI::SStringW title;
    SOUI::SStringW detail;
    SOUI::SStringW status;

    Row()
    {
    }

    Row(LPCWSTR titleText, LPCWSTR detailText, LPCWSTR statusText)
        : title(titleText)
        , detail(detailText)
        , status(statusText)
    {
    }

    const SOUI::SStringW &Column(int column) const
    {
        return column == 0 ? title : (column == 1 ? detail : status);
    }
};

inline const Row *Choices()
{
    static const Row rows[] = {
        Row(L"ListCtrl report", L"SListCtrl: text rows and a resizable header", L"Ready"), Row(L"Virtual list", L"SListView: recycled rows for large data sets", L"Ready"), Row(L"Multi-column view", L"SMCListView: three columns with sortable data", L"Review"), Row(L"Tile gallery", L"STileView: fixed-size cards with automatic wrapping", L"Ready"), Row(L"Grouped tree", L"STreeView: expandable parent and child nodes", L"Updated"), Row(L"Rich selection", L"SComboView: templated drop-down choices", L"Ready"), Row(L"Search suggestions", L"Keyword filtering over eight local entries", L"Draft"), Row(L"Resource browser", L"Named XML templates and Fluent skin resources", L"Synced"),
    };
    return rows;
}

inline int ChoiceCount()
{
    return 8;
}

inline Row NumberedRow(int index, LPCWSTR kind)
{
    Row row = Choices()[index % ChoiceCount()];
    row.title = SOUI::SStringW(kind) + SOUI::SStringW().Format(L" %03d - ", index + 1) + row.title;
    return row;
}

inline SOUI::SXmlNode ItemTemplate(SOUI::SXmlNode xmlTemplate)
{
    // Accept both the core demo's direct template and Gallery's <template><item> wrapper.
    SOUI::SXmlNode item = xmlTemplate.child(L"item");
    return item ? item : xmlTemplate;
}

inline void BindRow(SOUI::SItemPanel *item, SOUI::SXmlNode xmlTemplate, const Row &row)
{
    if (!item)
        return;
    if (item->GetChildrenCount() == 0)
    {
        SOUI::SXmlNode rowTemplate = ItemTemplate(xmlTemplate);
        item->InitFromXml(&rowTemplate);
    }

    // Item panels are recycled: populate every field on every bind, not just at creation.
    for (int column = 0; column < 3; ++column)
    {
        SOUI::SWindow *field = item->FindChildByID(column + 1);
        if (field)
            field->SetWindowText(SOUI::S_CW2T(row.Column(column)));
    }
}

class ListAdapter : public SOUI::SAdapterBase {
  public:
    explicit ListAdapter(int count, LPCWSTR kind = NULL)
    {
        for (int i = 0; i < count; ++i)
            m_rows.push_back(kind ? NumberedRow(i, kind) : Choices()[i % ChoiceCount()]);
    }

    explicit ListAdapter(const SOUI::SStringT &query)
    {
        SOUI::SStringW needle = SOUI::S_CT2W(query);
        needle.TrimBlank();
        needle.MakeLower();
        for (int i = 0; i < ChoiceCount(); ++i)
        {
            const Row &row = Choices()[i];
            SOUI::SStringW searchable = row.title + L" " + row.detail + L" " + row.status;
            searchable.MakeLower();
            if (needle.IsEmpty() || searchable.Find(needle) >= 0)
                m_rows.push_back(row);
        }
    }

    int WINAPI getCount()
    {
        return static_cast<int>(m_rows.size());
    }

    void WINAPI getView(int position, SOUI::SItemPanel *item, SOUI::SXmlNode xmlTemplate)
    {
        const Row empty;
        BindRow(item, xmlTemplate, position >= 0 && position < getCount() ? m_rows[position] : empty);
    }

    SOUI::SStringW WINAPI getItemDesc(int position)
    {
        // SComboView obtains its collapsed text here, not from a child control ID.
        return position >= 0 && position < getCount() ? m_rows[position].title : SOUI::SStringW();
    }

  private:
    std::vector<Row> m_rows;
};

struct RowColumnOrder
{
    RowColumnOrder(int column, bool ascending)
        : m_column(column)
        , m_ascending(ascending)
    {
    }

    bool operator()(const Row &left, const Row &right) const
    {
        const int comparison = left.Column(m_column).CompareNoCase(right.Column(m_column));
        return m_ascending ? comparison < 0 : comparison > 0;
    }

    int m_column;
    bool m_ascending;
};

class McAdapter : public SOUI::SMcAdapterBase {
  public:
    McAdapter()
    {
        m_columnNames[0] = L"gallery_column_title";
        m_columnNames[1] = L"gallery_column_detail";
        m_columnNames[2] = L"gallery_column_status";
        for (int i = 0; i < 80; ++i)
            m_rows.push_back(NumberedRow(i, L"Record"));
    }

    int WINAPI getCount()
    {
        return static_cast<int>(m_rows.size());
    }

    void WINAPI InitByTemplate(SOUI::SXmlNode xmlTemplate)
    {
        // MC positions columns by name, not ID. Keep supplied names; fill missing names
        // only in the view's private template copy. IDs 1/2/3 identify the column roots.
        for (SOUI::SXmlNode column = ItemTemplate(xmlTemplate).first_child(); column; column = column.next_sibling())
        {
            const int id = column.attribute(L"id").as_int();
            if (id < 1 || id > 3)
                continue;
            SOUI::SXmlAttr name = column.attribute(L"name");
            if (SOUI::SStringW(name.as_string()).IsEmpty())
            {
                if (!name)
                    name = column.append_attribute(L"name");
                name.set_value(m_columnNames[id - 1]);
            }
            else
            {
                m_columnNames[id - 1] = name.as_string();
            }
        }
    }

    void WINAPI getView(int position, SOUI::SItemPanel *item, SOUI::SXmlNode xmlTemplate)
    {
        const Row empty;
        BindRow(item, xmlTemplate, position >= 0 && position < getCount() ? m_rows[position] : empty);
    }

    SOUI::SStringW WINAPI getItemDesc(int position)
    {
        return position >= 0 && position < getCount() ? m_rows[position].title : SOUI::SStringW();
    }

    SOUI::SStringW WINAPI GetColumnName(int column) const
    {
        return column >= 0 && column < 3 ? m_columnNames[column] : SOUI::SStringW();
    }

    BOOL WINAPI OnSort(int column, UINT *formats, int columnCount)
    {
        if (!formats || column < 0 || column >= 3 || columnCount < 3)
            return FALSE;
        const bool ascending = (formats[column] & HDF_SORTUP) == 0;
        std::stable_sort(m_rows.begin(), m_rows.end(), RowColumnOrder(column, ascending));
        for (int i = 0; i < columnCount; ++i)
            formats[i] &= ~(HDF_SORTUP | HDF_SORTDOWN);
        formats[column] |= ascending ? HDF_SORTUP : HDF_SORTDOWN;
        // SMCListView::OnHeaderClick refreshes the view after OnSort returns TRUE.
        return TRUE;
    }

  private:
    std::vector<Row> m_rows;
    SOUI::SStringW m_columnNames[3];
};

class TreeAdapter : public SOUI::STreeAdapterBase<Row> {
  public:
    TreeAdapter()
    {
        const wchar_t *groups[] = { L"Workspace", L"Control samples", L"Resources" };
        for (int group = 0; group < 3; ++group)
        {
            const HSTREEITEM parent = InsertItem(Row(groups[group], L"4 child items", L"Group"));
            for (int child = 0; child < 4; ++child)
                InsertItem(NumberedRow(group * 4 + child, L"Item"), parent);
            SetItemExpanded(parent, TRUE);
        }
    }

    void WINAPI getView(HSTREEITEM node, SOUI::SItemPanel *item, SOUI::SXmlNode xmlTemplate)
    {
        if (!item)
            return;
        const bool firstBind = item->GetChildrenCount() == 0;
        BindRow(item, xmlTemplate, GetItemData(node));
        SOUI::SToggle *toggle = SOUI::sobj_cast<SOUI::SToggle>(item->FindChildByID(10));
        if (toggle)
        {
            if (firstBind)
                toggle->GetEventSet()->subscribeEvent(SOUI::EventCmd::EventID, SOUI::Subscriber(&TreeAdapter::OnToggle, this));
            const BOOL hasChildren = HasChildren(node);
            toggle->SetVisible(hasChildren, FALSE);
            toggle->SetToggle(hasChildren && IsItemExpanded(node), FALSE);
        }
    }

    BOOL OnToggle(SOUI::IEvtArgs *event)
    {
        SOUI::SToggle *toggle = SOUI::sobj_cast<SOUI::SToggle>(event->Sender());
        SOUI::SItemPanel *item = toggle ? SOUI::sobj_cast<SOUI::SItemPanel>(toggle->GetRoot()) : NULL;
        if (!item)
            return FALSE;
        const HSTREEITEM node = static_cast<HSTREEITEM>(item->GetItemIndex());
        if (!HasChildren(node))
            return FALSE;
        ExpandItem(node, toggle->GetToggle() ? SOUI::TVC_EXPAND : SOUI::TVC_COLLAPSE);
        return TRUE;
    }

    BOOL WINAPI isViewWidthMatchParent() const
    {
        return TRUE;
    }
};

template <class View, class Adapter>
inline bool InstallAdapter(View *view, Adapter *adapter)
{
    // SetAdapter retains an IObjRef; always drop the creator's reference, including failure.
    const bool installed = view && view->SetAdapter(adapter) != FALSE;
    adapter->Release();
    return installed;
}

inline bool InitializeReport(SOUI::SListCtrl *view)
{
    if (!view || view->GetColumnCount() > 3)
        return false;
    const wchar_t *titles[] = { L"Title", L"Detail", L"Status" };
    const int widths[] = { 220, 400, 100 };
    while (view->GetColumnCount() < 3)
    {
        const int column = view->GetColumnCount();
        const int width = (widths[column] * view->GetScale() + 50) / 100;
        if (view->InsertColumn(column, SOUI::S_CW2T(titles[column]), width, HDF_LEFT) < 0)
            return false;
    }
    view->DeleteAllItems();
    for (int i = 0; i < 12; ++i)
    {
        Row row = NumberedRow(i, L"Task");
        // Stored cells have no template/ellipsis hook; keep the Kind column concise.
        row.detail = Choices()[i % ChoiceCount()].title;
        const int item = view->InsertItem(i, SOUI::S_CW2T(row.title));
        if (item < 0)
            return false;
        view->SetItemData(item, static_cast<LPARAM>(i));
        if (!view->SetSubItemText(item, 1, SOUI::S_CW2T(row.detail)) || !view->SetSubItemText(item, 2, SOUI::S_CW2T(row.status)))
            return false;
    }
    return view->GetItemCount() == 12;
}
} // namespace AdapterDetail

// Call after XML initialization. A missing/wrongly typed sample or rejected adapter returns
// false, but does not prevent the other samples from being initialized. Safe to call again.
inline bool InitializeViews(SOUI::SWindow *root)
{
    if (!root)
        return false;
    bool initialized = AdapterDetail::InitializeReport(root->FindChildByName2<SOUI::SListCtrl>(L"sample_listctrl"));
    initialized = AdapterDetail::InstallAdapter(root->FindChildByName2<SOUI::SListView>(L"sample_listview"), new AdapterDetail::ListAdapter(200, L"Activity")) && initialized;
    initialized = AdapterDetail::InstallAdapter(root->FindChildByName2<SOUI::SMCListView>(L"sample_mclistview"), new AdapterDetail::McAdapter()) && initialized;
    initialized = AdapterDetail::InstallAdapter(root->FindChildByName2<SOUI::STileView>(L"sample_tileview"), new AdapterDetail::ListAdapter(30, L"Tile")) && initialized;
    initialized = AdapterDetail::InstallAdapter(root->FindChildByName2<SOUI::STreeView>(L"sample_treeview"), new AdapterDetail::TreeAdapter()) && initialized;

    SOUI::SComboView *combo = root->FindChildByName2<SOUI::SComboView>(L"sample_comboview");
    const bool comboInitialized = AdapterDetail::InstallAdapter(combo ? combo->GetListView() : NULL, new AdapterDetail::ListAdapter(AdapterDetail::ChoiceCount()));
    if (comboInitialized)
    {
        const BOOL wasMuted = combo->GetEventSet()->isMuted();
        combo->GetEventSet()->setMutedState(true);
        combo->SetCurSel(0);
        combo->GetEventSet()->setMutedState(wasMuted);
    }
    return comboInitialized && initialized;
}

// Empty/whitespace queries show all eight choices; a non-match produces an empty adapter.
// Call from the search edit's change handler; popup visibility remains the host's decision.
inline void FillSearch(SOUI::IListView *list, const SOUI::SStringT &query)
{
    if (!list)
        return;
    if (AdapterDetail::InstallAdapter(list, new AdapterDetail::ListAdapter(query)))
        list->SetSel(-1, FALSE);
}
} // namespace Gallery
