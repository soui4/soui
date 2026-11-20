#include "MainDlg.h"
#include "GalleryAdapters.h"
#include "GalleryCatalog.h"
#include "GalleryNative.h"
#include <control/SStackView.h>
#include <control/SCalendar.h>
#include <control/SDateTimePicker.h>
#include <control/SSearchDropdownList.h>
#include <core/SModalViewSession.h>
#include <core/SWindowMgr.h>
#include "fluent/FluentWindowChrome.h"

namespace
{
const wchar_t *ProgressNames[] = { L"progress_standard", L"progress_success", L"progress_pill", L"progress_custom" };
const wchar_t *ProgressLabels[] = { L"progress_standard_label", L"progress_success_label", L"progress_pill_label", L"progress_custom_label" };
const int InitialProgress[] = { 25, 60, 90, 35 };
const UINT_PTR ProgressTimer = 1;
const unsigned AnimationDuration = 900;

class CDecisionDialog : public SHostDialog {
  public:
    CDecisionDialog()
        : SHostDialog(_T("layout:gallery_dialog"))
    {
    }
    BOOL OnInitDialog(HWND, LPARAM)
    {
        return m_chrome.Attach(*this);
    }
    void OnConfirm()
    {
        EndDialog(IDOK);
    }
    void OnCancel()
    {
        EndDialog(IDCANCEL);
    }

  private:
    FluentDemo::WindowChrome m_chrome;
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"dialog_confirm", OnConfirm)
        EVENT_NAME_COMMAND(L"dialog_cancel", OnCancel)
        EVENT_NAME_COMMAND(L"btn_close", OnCancel)
    EVENT_MAP_END2(SHostDialog)
    BEGIN_MSG_MAP_EX(CDecisionDialog)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnCancel)
        CHAIN_MSG_MAP(SHostDialog)
    END_MSG_MAP()
};
// Native child HWNDs need an opaque host. The gallery itself is layered.
class CNativePreviewDialog : public SHostDialog {
  public:
    CNativePreviewDialog()
        : SHostDialog(_T("layout:gallery_native"))
    {
    }
    BOOL OnInitDialog(HWND, LPARAM)
    {
        // Create the native child only after its SOUI layout is measurable.
        // Early init during XML loading otherwise starts it at 0 x 0.
        GetRoot()->UpdateLayout();
        SRealWnd *native = FindChildByName2<SRealWnd>(L"sample_realwnd");
        return native && ::IsWindow(native->GetRealHwnd(TRUE));
    }
    void OnDone()
    {
        EndDialog(IDOK);
    }
    BOOL OnCreateNative(IEvtArgs *event)
    {
        SRealWnd *native = sobj_cast<SRealWnd>(event->Sender());
        sobj_cast<EventRealWndCreate>(event)->hRet = Gallery::CreateNativePreview(m_hWnd, native->GetScale());
        return TRUE;
    }
    BOOL OnDestroyNative(IEvtArgs *event)
    {
        SRealWnd *native = sobj_cast<SRealWnd>(event->Sender());
        if (HWND window = native->GetRealHwnd(FALSE))
            ::DestroyWindow(window);
        return TRUE;
    }

  private:
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnDone)
        EVENT_NAME_COMMAND(L"native_done", OnDone)
        EVENT_NAME_HANDLER(L"sample_realwnd", EventRealWndCreate::EventID, OnCreateNative)
        EVENT_NAME_HANDLER(L"sample_realwnd", EventRealWndDestroy::EventID, OnDestroyNative)
    EVENT_MAP_END2(SHostDialog)
    BEGIN_MSG_MAP_EX(CNativePreviewDialog)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SHostDialog)
    END_MSG_MAP()
};
} // namespace

GalleryMainDialog::GalleryMainDialog()
    : SHostWnd(_T("layout:gallery"))
    , m_category(Gallery::InitialCategory)
    , m_initialized(false)
    , m_sourceVisible(false)
    , m_alternateTarget(false)
    , m_animationStart(0)
    , m_addedItems(0)
    , m_modalPreviousFocus(0)
{
    for (int index = 0; index < 4; ++index)
        m_animationFrom[index] = m_animationTo[index] = InitialProgress[index];
}

BOOL GalleryMainDialog::OnInitDialog(HWND, LPARAM)
{
    if (!Gallery::InitializeViews(GetRoot()))
        return FALSE;
    OnThemeChanged();
    m_initialized = true;
    Navigate(Gallery::InitialCategory);
    UpdateProgressLabels();
    return TRUE;
}

void GalleryMainDialog::OnClose()
{
    DestroyWindow();
}
void GalleryMainDialog::OnMinimize()
{
    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}
void GalleryMainDialog::OnMaximize()
{
    SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void GalleryMainDialog::OnRestore()
{
    SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void GalleryMainDialog::OnSize(UINT type, CSize)
{
    SetMsgHandled(FALSE);
    if (SWindow *button = FindChildByName(L"btn_max"))
        button->SetVisible(type != SIZE_MAXIMIZED, TRUE);
    if (SWindow *button = FindChildByName(L"btn_restore"))
        button->SetVisible(type == SIZE_MAXIMIZED, TRUE);
}

void GalleryMainDialog::OnDestroy()
{
    KillTimer(ProgressTimer);
    m_initialized = false;
    RestoreModalInput();
    SetMsgHandled(FALSE);
}

void GalleryMainDialog::OnNavigate(UINT id)
{
    Navigate(static_cast<int>(id) - Gallery::NavigationId);
}

void GalleryMainDialog::Navigate(int index)
{
    if (index < 0 || index >= Gallery::CategoryCount)
        return;
    m_category = index;
    FindChildByName2<STabCtrl>(L"gallery_pages")->SetCurSel(index);
    const Gallery::Category &category = Gallery::Categories()[index];
    FindChildByName(L"gallery_title")->SetWindowText(S_CW2T(category.title));
    FindChildByName(L"gallery_description")->SetWindowText(S_CW2T(category.description));
    SStringT api = S_CW2T(category.api);
    if (api.GetLength() > 70)
    {
        int types = 0;
        for (int i = 0; i < Gallery::RegisteredControlCount; ++i)
            types += Gallery::Controls()[i].category == index ? 1 : 0;
        api.Format(_T("%d registered types / native SOUI"), types);
    }
    FindChildByName(L"gallery_api")->SetWindowText(api);
    FindChildByName(L"gallery_breadcrumb")->SetWindowText(S_CW2T(category.title));
    for (int item = 0; item < Gallery::CategoryCount; ++item)
    {
        SWindow *navigation = FindChildByID(Gallery::NavigationId + item);
        navigation->SetAttribute(L"skin", item == index ? L"gallery_nav_selected" : L"gallery_nav");
    }
    FindChildByName(L"gallery_source")->SetWindowText(S_CW2T(Gallery::Sources()[index]));
    SStringT message;
    message.Format(_T("%s selected  /  Native SOUI examples"), S_CW2T(category.title).c_str());
    SetStatus(message);
}

void GalleryMainDialog::SetStatus(const SStringT &text)
{
    FindChildByName(L"gallery_status")->SetWindowText(text);
}

BOOL GalleryMainDialog::OnSearch(IEvtArgs *event)
{
    if (!m_initialized || sobj_cast<EventRENotify>(event)->iNotify != EN_CHANGE)
        return FALSE;
    const std::wstring query = S_CT2W(FindChildByName(L"gallery_search")->GetWindowText()).c_str();
    int matches = 0;
    for (int index = 0; index < Gallery::CategoryCount; ++index)
    {
        const bool visible = Gallery::Matches(index, query);
        FindChildByID(Gallery::NavigationId + index)->SetVisible(visible, TRUE);
        matches += visible ? 1 : 0;
    }
    FindChildByName(L"gallery_no_results")->SetVisible(!matches, TRUE);
    SStringT count;
    count.Format(_T("%d / %d categories"), matches, Gallery::CategoryCount);
    FindChildByName(L"gallery_category_count")->SetWindowText(count);
    return TRUE;
}

void GalleryMainDialog::UpdateProgressLabels()
{
    for (int index = 0; index < 4; ++index)
    {
        const int value = FindChildByName2<SProgress>(ProgressNames[index])->GetValue();
        SStringT label;
        label.Format(index == 3 ? _T("%d / 80") : _T("%d%%"), value);
        FindChildByName(ProgressLabels[index])->SetWindowText(label);
    }
}

void GalleryMainDialog::OnAnimate()
{
    KillTimer(ProgressTimer);
    m_alternateTarget = !m_alternateTarget;
    const int target[] = { 75, 100, 45, 70 };
    for (int index = 0; index < 4; ++index)
    {
        m_animationFrom[index] = FindChildByName2<SProgress>(ProgressNames[index])->GetValue();
        m_animationTo[index] = m_alternateTarget ? target[index] : InitialProgress[index];
    }
    m_animationStart = GetTickCount();
    SetTimer(ProgressTimer, 16, NULL);
    SetStatus(_T("Animating progress  /  Click again to retarget from the current value"));
}

void GalleryMainDialog::OnTimer(UINT_PTR timer)
{
    if (timer != ProgressTimer)
    {
        SetMsgHandled(FALSE);
        return;
    }
    const DWORD elapsed = GetTickCount() - m_animationStart;
    for (int index = 0; index < 4; ++index)
        FindChildByName2<SProgress>(ProgressNames[index])->SetValue(Gallery::AnimateValue(m_animationFrom[index], m_animationTo[index], elapsed, AnimationDuration));
    UpdateProgressLabels();
    if (elapsed >= AnimationDuration)
    {
        KillTimer(ProgressTimer);
        SetStatus(_T("Animation complete  /  All ranges remain bounded"));
    }
}

void GalleryMainDialog::OnReset()
{
    KillTimer(ProgressTimer);
    m_alternateTarget = false;
    for (int index = 0; index < 4; ++index)
        FindChildByName2<SProgress>(ProgressNames[index])->SetValue(InitialProgress[index]);
    UpdateProgressLabels();
    SetStatus(_T("Progress reset to 25%, 60%, 90%, and custom value 35"));
}

void GalleryMainDialog::OnSource()
{
    m_sourceVisible = !m_sourceVisible;
    FindChildByName(L"gallery_source_panel")->SetVisible(m_sourceVisible, TRUE);
    SetStatus(m_sourceVisible ? _T("XML example opened  /  Full examples live in uires/xml/gallery.xml") : _T("XML example closed"));
}

void GalleryMainDialog::OnAction(UINT id)
{
    if (id == 4304)
    {
        Navigate(5);
        return;
    }
    if (id == 4305)
    {
        Navigate(1);
        return;
    }
    if (id == 4306)
    {
        Navigate(11);
        return;
    }
    SStringT message;
    message.Format(_T("Action received: %s  /  %u"), FindChildByID(id)->GetWindowText().c_str(), id);
    SetStatus(message);
    if (SWindow *feedback = FindChildByName(L"button_feedback"))
        feedback->SetWindowText(message);
}

BOOL GalleryMainDialog::OnInput(IEvtArgs *event)
{
    if (!m_initialized || sobj_cast<EventRENotify>(event)->iNotify != EN_CHANGE)
        return FALSE;
    SStringT text = FindChildByName(L"sample_input")->GetWindowText();
    SStringT count;
    count.Format(_T("%d characters  /  Edits are preserved when you change theme or category"), text.GetLength());
    FindChildByName(L"input_feedback")->SetWindowText(count);
    return TRUE;
}

BOOL GalleryMainDialog::OnSlider(IEvtArgs *event)
{
    const int value = sobj_cast<EventSliderPos>(event)->nPos;
    SStringT label;
    label.Format(_T("%d%s"), value, event->IdFrom() == 4501 ? _T(" / 10") : _T("%"));
    FindChildByName(event->IdFrom() == 4501 ? L"slider_steps_value" : L"slider_value")->SetWindowText(label);
    SetStatus(_T("Slider value updated"));
    return TRUE;
}

BOOL GalleryMainDialog::OnSelection(IEvtArgs *event)
{
    SWindow *control = sobj_cast<SWindow>(event->Sender());
    SStringT message;
    message.Format(_T("%s: %s"), control->GetWindowText().c_str(), control->IsChecked() ? _T("selected") : _T("cleared"));
    FindChildByName(L"selection_feedback")->SetWindowText(message);
    SetStatus(message);
    return TRUE;
}

BOOL GalleryMainDialog::OnRadioState(IEvtArgs *event)
{
    if (!m_initialized)
        return FALSE;
    EventSwndStateChanged *state = sobj_cast<EventSwndStateChanged>(event);
    if (!EventSwndStateChanged_CheckState(state, WndState_Check) || !(state->dwNewState & WndState_Check))
        return FALSE;
    return OnSelection(event);
}

BOOL GalleryMainDialog::OnCombo(IEvtArgs *event)
{
    SComboBase *combo = sobj_cast<SComboBase>(event->Sender());
    SStringT message;
    message.Format(_T("Selected: %s"), combo->GetWindowText().c_str());
    FindChildByName(L"combo_feedback")->SetWindowText(message);
    SetStatus(message);
    return TRUE;
}

BOOL GalleryMainDialog::OnList(IEvtArgs *)
{
    SListBox *list = FindChildByName2<SListBox>(L"sample_list");
    SStringT message;
    message.Format(_T("Selected row %d  /  %d items"), list->GetCurSel() + 1, list->GetCount());
    FindChildByName(L"list_feedback")->SetWindowText(message);
    SetStatus(message);
    return TRUE;
}

void GalleryMainDialog::OnAddItem()
{
    SListBox *list = FindChildByName2<SListBox>(L"sample_list");
    SStringT text;
    text.Format(_T("New collection %d"), ++m_addedItems);
    list->InsertString(-1, text);
    list->SetCurSel(list->GetCount() - 1, TRUE);
}

void GalleryMainDialog::OnRemoveItem()
{
    SListBox *list = FindChildByName2<SListBox>(L"sample_list");
    const int selected = list->GetCurSel();
    if (selected < 0)
    {
        SetStatus(_T("Select an item before removing it"));
        return;
    }
    list->DeleteString(selected);
    list->SetCurSel(list->GetCount() ? (std::min)(selected, list->GetCount() - 1) : -1, TRUE);
    OnList(NULL);
}

BOOL GalleryMainDialog::OnTree(IEvtArgs *)
{
    if (!m_initialized)
        return FALSE;
    STreeCtrl *tree = FindChildByName2<STreeCtrl>(L"sample_tree");
    SStringT text;
    tree->GetItemText(tree->GetSelectedItem(), text);
    SStringT message;
    message.Format(_T("Selected: %s"), text.c_str());
    FindChildByName(L"tree_feedback")->SetWindowText(message);
    SetStatus(message);
    return TRUE;
}

BOOL GalleryMainDialog::OnTabs(IEvtArgs *)
{
    SStringT message;
    message.Format(_T("Tab %d is active  /  Each view keeps its state"), FindChildByName2<STabCtrl>(L"sample_tabs")->GetCurSel() + 1);
    SetStatus(message);
    return TRUE;
}

void GalleryMainDialog::OnDialog()
{
    KillTimer(ProgressTimer);
    CDecisionDialog dialog;
    const INT_PTR result = dialog.DoModal(m_hWnd);
    SetStatus(result == IDOK ? _T("Dialog confirmed  /  Your changes were saved") : _T("Dialog dismissed  /  No changes made"));
    FindChildByName(L"dialog_feedback")->SetWindowText(result == IDOK ? _T("Confirmed. Everything is up to date.") : _T("Cancelled. Your original state is unchanged."));
}

void GalleryMainDialog::OnThemeChanged()
{
    if (SIconWnd *icon = FindChildByName2<SIconWnd>(L"sample_icon"))
        icon->SetIcon(Gallery::CreateVectorIcon(icon->GetScale()));
}

void GalleryMainDialog::OnStackPrevious()
{
    SStackView *stack = FindChildByName2<SStackView>(L"sample_stack");
    stack->SelectPage(0, FALSE);
    SetStatus(_T("Stack / Workspace selected"));
}
void GalleryMainDialog::OnStackNext()
{
    FindChildByName2<SStackView>(L"sample_stack")->SelectPage(1, FALSE);
    SetStatus(_T("Stack / Activity selected"));
}
void GalleryMainDialog::OnRestoreDock()
{
    FindChildByName(L"sample_dockbar")->SetVisible(TRUE, TRUE);
    SetStatus(_T("Dock bar restored"));
}
BOOL GalleryMainDialog::OnViewEvent(IEvtArgs *event)
{
    if (!m_initialized)
        return FALSE;
    SWindow *sender = sobj_cast<SWindow>(event->Sender());
    SStringT message;
    if (EventLVSelChanged *selected = sobj_cast<EventLVSelChanged>(event))
        message.Format(_T("%s / selected model row %d"), S_CW2T(sender->GetObjectClass()).c_str(), selected->iNewSel + 1);
    else if (EventLCSelChanged *selected = sobj_cast<EventLCSelChanged>(event))
        message.Format(_T("SListCtrl / selected stored row %d"), selected->nNewSel + 1);
    else
        message = _T("STreeView / selected a data-bound node");
    FindChildByName(sender->IsClass(L"tileview") ? L"tiles_feedback" : L"views_feedback")->SetWindowText(message);
    SetStatus(message);
    return TRUE;
}
BOOL GalleryMainDialog::OnExtraState(IEvtArgs *event)
{
    if (!m_initialized)
        return FALSE;
    if (EventSwndStateChanged *state = sobj_cast<EventSwndStateChanged>(event))
        if (!EventSwndStateChanged_CheckState(state, WndState_Check))
            return FALSE;
    SWindow *sender = sobj_cast<SWindow>(event->Sender());
    SStringT message;
    message.Format(_T("%s / %s"), sender->GetName(), sender->IsChecked() ? _T("checked") : _T("unchecked"));
    if (sender == FindChildByName(L"sample_switch"))
        FindChildByName(L"switch_feedback")->SetWindowText(sender->IsChecked() ? _T("Sync is on") : _T("Sync is off"));
    if (EventRadioGroupCheckChanged *radio = sobj_cast<EventRadioGroupCheckChanged>(event))
    {
        if (!radio->pChecked)
            return FALSE;
        message.Format(_T("Radio group / %s"), sobj_cast<SWindow>(radio->pChecked)->GetWindowText().c_str());
    }
    FindChildByName(L"selection_feedback")->SetWindowText(message);
    SetStatus(message);
    return TRUE;
}
BOOL GalleryMainDialog::OnExtraInput(IEvtArgs *event)
{
    if (!m_initialized)
        return FALSE;
    if (EventRENotify *notify = sobj_cast<EventRENotify>(event))
        if (notify->iNotify != EN_CHANGE)
            return FALSE;
    SStringT message;
    SWindow *sender = sobj_cast<SWindow>(event->Sender());
    // RichEdit can reinitialize its text service on show/scale/theme changes.
    // Those EN_CHANGE notifications are not edits made by the user.
    if (sender == FindChildByName(L"sample_richedit") && !sender->IsFocused())
        return FALSE;
    if (EventScroll *scroll = sobj_cast<EventScroll>(event))
    {
        message.Format(_T("Scrollbar / position %d"), scroll->nPos);
        FindChildByName(L"scrollbar_feedback")->SetWindowText(message);
    }
    else
    {
        message.Format(_T("%s / %s"), sender->GetName(), sender->GetWindowText().c_str());
        if (sender == FindChildByName(L"sample_number"))
            FindChildByName(L"number_feedback")->SetWindowText(message);
        if (sender == FindChildByName(L"sample_hotkey"))
            FindChildByName(L"hotkey_feedback")->SetWindowText(message);
    }
    SetStatus(message);
    return TRUE;
}
BOOL GalleryMainDialog::OnHeader(IEvtArgs *event)
{
    SHeaderCtrl *header = sobj_cast<SHeaderCtrl>(event->Sender());
    EventHeaderClick *clicked = sobj_cast<EventHeaderClick>(event);
    // The standalone header has no adapter. Alternate its indicator explicitly.
    const bool descending = header->GetUserData() != 0;
    for (UINT i = 0; i < header->GetItemCount(); ++i)
        header->SetItemSort(i, i == static_cast<UINT>(clicked->iItem) ? (descending ? HDF_SORTDOWN : HDF_SORTUP) : 0);
    header->SetUserData(!descending);
    SetStatus(descending ? _T("Header / descending sort indicator") : _T("Header / ascending sort indicator"));
    return TRUE;
}
BOOL GalleryMainDialog::OnDateChanged(IEvtArgs *event)
{
    if (!m_initialized)
        return FALSE;
    SStringT message;
    if (EventDateTimeChanged *date = sobj_cast<EventDateTimeChanged>(event))
        message.Format(_T("Selected date / %04d-%02d-%02d %02d:%02d"), date->newTime.wYear, date->newTime.wMonth, date->newTime.wDay, date->newTime.wHour, date->newTime.wMinute);
    else
    {
        EventCalendarExChanged *calendar = sobj_cast<EventCalendarExChanged>(event);
        message.Format(_T("Calendar / %04d-%02d-%02d"), calendar->iNewYear, calendar->iNewMonth, calendar->iNewDay);
    }
    FindChildByName(L"date_feedback")->SetWindowText(message);
    SetStatus(message);
    return TRUE;
}
BOOL GalleryMainDialog::OnCommandBar(IEvtArgs *event)
{
    SStringT message;
    message.Format(_T("Toolbar / command %d"), sobj_cast<EventToolBarCmd>(event)->nCmdId);
    FindChildByName(L"commands_feedback")->SetWindowText(message);
    SetStatus(message);
    return TRUE;
}
BOOL GalleryMainDialog::OnMenuCommand(IEvtArgs *event)
{
    SStringT message;
    message.Format(_T("Menu / command %d"), sobj_cast<EventSelectMenu>(event)->nMenuId);
    FindChildByName(L"commands_feedback")->SetWindowText(message);
    SetStatus(message);
    return TRUE;
}
void GalleryMainDialog::OnSuggest()
{
    SStringT query = FindChildByName(L"sample_suggestion_input")->GetWindowText();
    FindChildByName2<SSearchDropdownList>(L"sample_suggestions")->DropDown(&query);
}
BOOL GalleryMainDialog::OnSuggestionFill(IEvtArgs *event)
{
    EventDropdownListFill *fill = sobj_cast<EventDropdownListFill>(event);
    Gallery::FillSearch(fill->pListView, SStringT(fill->strKey));
    fill->bPopup = fill->pListView->GetAdapter()->getCount() > 0;
    if (!fill->bPopup)
        FindChildByName(L"suggestion_feedback")->SetWindowText(_T("No matching items. Try list, view, or an empty keyword."));
    return TRUE;
}
BOOL GalleryMainDialog::OnSuggestionRect(IEvtArgs *event)
{
    sobj_cast<EventDropdownListGetBuddyRect>(event)->rcBuddy = FindChildByName(L"sample_suggestion_input")->GetWindowRect();
    return TRUE;
}
BOOL GalleryMainDialog::OnSuggestionSelected(IEvtArgs *event)
{
    EventDropdownListSelected *selection = sobj_cast<EventDropdownListSelected>(event);
    SStringT text;
    SStringW description;
    selection->pListView->GetAdapter()->getItemDesc(selection->nValue, &description);
    text = S_CW2T(description);
    FindChildByName(L"sample_suggestion_input")->SetWindowText(text);
    FindChildByName(L"suggestion_feedback")->SetWindowText(text);
    SetStatus(text);
    return TRUE;
}
void GalleryMainDialog::OnModal()
{
    if (GetLastModalViewSessionID() != 0)
        return;
    SModalRoot *modal = sobj_cast<SModalRoot>(SApplication::getSingleton().CreateWindowByName(L"modalroot"));
    if (!modal)
        return;
    if (!modal->InitFromResId(_T("layout:gallery_modal")))
    {
        modal->Release();
        return;
    }
    MemberFunctionSlot<GalleryMainDialog, IEvtArgs> close = Subscriber(&GalleryMainDialog::OnModalClose, this);
    modal->FindChildByName(L"modal_confirm")->SubscribeEvent(EventCmd::EventID, &close);
    modal->FindChildByName(L"modal_cancel")->SubscribeEvent(EventCmd::EventID, &close);
    MemberFunctionSlot<GalleryMainDialog, IEvtArgs> exited = Subscriber(&GalleryMainDialog::OnModalExited, this);
    modal->SubscribeEvent(EventExitModalView::EventID, &exited);

    m_modalPreviousFocus = GetRoot()->GetContainer()->GetFocus();
    // The host focus manager traverses the entire tree. Temporarily disabling
    // the existing root subtrees confines Tab/Shift+Tab and accelerators to the
    // new modal tree. Preserve roots that were already disabled.
    for (SWindow *child = GetRoot()->GetWindow(GSW_FIRSTCHILD); child; child = child->GetWindow(GSW_NEXTSIBLING))
    {
        if (!child->IsDisabled(FALSE))
        {
            m_modalDisabledRoots.push_back(child->GetSwnd());
            child->EnableWindow(FALSE, TRUE);
        }
    }
    if (BeginModalViewSession(modal) == 0)
    {
        RestoreModalInput();
        modal->Release();
        return;
    }
    modal->FindChildByName(L"modal_confirm")->SetFocus();
    // 会话持有窗口树的引用，弹窗在会话结束前一直存活；抛掉创建引用的最后一份。
    modal->Release();
}
BOOL GalleryMainDialog::OnModalClose(IEvtArgs *event)
{
    const bool confirmed = wcscmp(event->NameFrom(), L"modal_confirm") == 0;
    EndModalViewSession(0, confirmed ? IDOK : IDCANCEL);
    return TRUE;
}
void GalleryMainDialog::RestoreModalInput()
{
    for (size_t i = 0; i < m_modalDisabledRoots.size(); ++i)
        if (SWindow *window = SWindowMgr::GetWindow(m_modalDisabledRoots[i]))
            window->EnableWindow(TRUE, TRUE);
    m_modalDisabledRoots.clear();
    if (SWindow *previous = SWindowMgr::GetWindow(m_modalPreviousFocus))
        if (previous->IsVisible(TRUE) && !previous->IsDisabled(TRUE))
            previous->SetFocus();
    m_modalPreviousFocus = 0;
}
BOOL GalleryMainDialog::OnModalExited(IEvtArgs *event)
{
    RestoreModalInput();
    const bool confirmed = sobj_cast<EventExitModalView>(event)->exitCode == IDOK;
    if (m_initialized)
        SetStatus(confirmed ? _T("In-window dialog confirmed") : _T("In-window dialog cancelled"));
    return FALSE;
}
void GalleryMainDialog::OnNativeHost()
{
    CNativePreviewDialog dialog;
    dialog.DoModal(m_hWnd);
    SetStatus(_T("Native child host closed / HWND lifetime ended"));
}

void GalleryMainDialog::ShowSection(LPCWSTR name)
{
    // Deep-link to a live control card without synthesizing desktop input.
    SWindow *sample = FindChildByName(name);
    SWindow *page = FindChildByName2<STabCtrl>(L"gallery_pages")->GetItem(m_category);
    SScrollView *scroll = sobj_cast<SScrollView>(page->GetWindow(GSW_FIRSTCHILD));
    if (!sample || !scroll)
        return;
    GetRoot()->UpdateLayout();
    POINT origin;
    scroll->GetViewOrigin(&origin);
    origin.y += sample->GetWindowRect().top - scroll->GetWindowRect().top - 100;
    origin.y = smax(0L, origin.y);
    scroll->SetViewOrigin(origin);
}
