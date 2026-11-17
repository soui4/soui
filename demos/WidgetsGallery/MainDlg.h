#pragma once

#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SouiCtrls.h>
#include <control/SSliderBar.h>
#include <control/STreeCtrl.h>
#include <SouiFactory.h>
#include "GalleryModel.h"
#include <vector>

using namespace SNS;

class GalleryMainDialog
    : public SHostWnd
    , public SDpiHandler<GalleryMainDialog> {
  public:
    GalleryMainDialog();
    BOOL OnInitDialog(HWND, LPARAM);
    void OnClose();
    void OnMinimize();
    void OnMaximize();
    void OnRestore();
    void OnSize(UINT type, CSize);
    void OnTimer(UINT_PTR timer);
    void OnDestroy();
    void OnNavigate(UINT id);
    void OnAnimate();
    void OnReset();
    void OnSource();
    void OnAction(UINT id);
    void OnAddItem();
    void OnRemoveItem();
    void OnDialog();
    BOOL OnSearch(IEvtArgs *event);
    BOOL OnInput(IEvtArgs *event);
    BOOL OnSlider(IEvtArgs *event);
    BOOL OnSelection(IEvtArgs *event);
    BOOL OnRadioState(IEvtArgs *event);
    BOOL OnCombo(IEvtArgs *event);
    BOOL OnList(IEvtArgs *event);
    BOOL OnTree(IEvtArgs *event);
    BOOL OnTabs(IEvtArgs *event);
    void OnThemeChanged();
    void ShowSection(LPCWSTR name);
    void OnStackPrevious();
    void OnStackNext();
    void OnRestoreDock();
    void OnSuggest();
    void OnModal();
    void OnNativeHost();
    BOOL OnModalClose(IEvtArgs *event);
    BOOL OnModalExited(IEvtArgs *event);
    BOOL OnViewEvent(IEvtArgs *event);
    BOOL OnExtraState(IEvtArgs *event);
    BOOL OnExtraInput(IEvtArgs *event);
    BOOL OnHeader(IEvtArgs *event);
    BOOL OnDateChanged(IEvtArgs *event);
    BOOL OnCommandBar(IEvtArgs *event);
    BOOL OnMenuCommand(IEvtArgs *event);
    BOOL OnSuggestionFill(IEvtArgs *event);
    BOOL OnSuggestionRect(IEvtArgs *event);
    BOOL OnSuggestionSelected(IEvtArgs *event);

  private:
    void Navigate(int index);
    void RestoreModalInput();
    void SetStatus(const SStringT &text);
    void UpdateProgressLabels();
    int m_category;
    bool m_initialized;
    bool m_sourceVisible;
    bool m_alternateTarget;
    DWORD m_animationStart;
    int m_animationFrom[4];
    int m_animationTo[4];
    int m_addedItems;
    SWND m_modalPreviousFocus;
    std::vector<SWND> m_modalDisabledRoots;

    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND_RANGE(4100, 4119, OnNavigate)
        EVENT_ID_COMMAND_RANGE(4300, 4312, OnAction)
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_NAME_COMMAND(L"btn_animate", OnAnimate)
        EVENT_NAME_COMMAND(L"btn_reset", OnReset)
        EVENT_NAME_COMMAND(L"btn_source", OnSource)
        EVENT_NAME_COMMAND(L"btn_add_item", OnAddItem)
        EVENT_NAME_COMMAND(L"btn_remove_item", OnRemoveItem)
        EVENT_NAME_COMMAND(L"btn_dialog", OnDialog)
        EVENT_NAME_HANDLER(L"gallery_search", EventRENotify::EventID, OnSearch)
        EVENT_NAME_HANDLER(L"sample_input", EventRENotify::EventID, OnInput)
        EVENT_NAME_HANDLER(L"sample_slider", EventSliderPos::EventID, OnSlider)
        EVENT_NAME_HANDLER(L"sample_slider_steps", EventSliderPos::EventID, OnSlider)
        EVENT_ID_RANGE_HANDLER(4400, 4401, EventCmd::EventID, OnSelection)
        EVENT_ID_RANGE_HANDLER(4402, 4404, EventSwndStateChanged::EventID, OnRadioState)
        EVENT_NAME_HANDLER(L"sample_combo", EventCBSelChange::EventID, OnCombo)
        EVENT_NAME_HANDLER(L"sample_combo_edit", EventCBSelChange::EventID, OnCombo)
        EVENT_NAME_HANDLER(L"sample_list", EventLBSelChanged::EventID, OnList)
        EVENT_NAME_HANDLER(L"sample_tree", EventTCSelChanged::EventID, OnTree)
        EVENT_NAME_HANDLER(L"sample_tabs", EventTabSelChanged::EventID, OnTabs)
        EVENT_NAME_COMMAND(L"btn_stack_prev", OnStackPrevious)
        EVENT_NAME_COMMAND(L"btn_stack_next", OnStackNext)
        EVENT_NAME_COMMAND(L"btn_dock_restore", OnRestoreDock)
        EVENT_NAME_COMMAND(L"btn_suggest", OnSuggest)
        EVENT_NAME_COMMAND(L"btn_modal", OnModal)
        EVENT_NAME_COMMAND(L"btn_native_host", OnNativeHost)
        EVENT_NAME_HANDLER(L"sample_listctrl", EventLCSelChanged::EventID, OnViewEvent)
        EVENT_NAME_HANDLER(L"sample_listview", EventLVSelChanged::EventID, OnViewEvent)
        EVENT_NAME_HANDLER(L"sample_mclistview", EventLVSelChanged::EventID, OnViewEvent)
        EVENT_NAME_HANDLER(L"sample_tileview", EventLVSelChanged::EventID, OnViewEvent)
        EVENT_NAME_HANDLER(L"sample_treeview", EventTVSelChanged::EventID, OnViewEvent)
        EVENT_NAME_HANDLER(L"sample_comboview", EventCBSelChange::EventID, OnCombo)
        EVENT_NAME_HANDLER(L"sample_switch", EventSwndStateChanged::EventID, OnExtraState)
        EVENT_NAME_HANDLER(L"sample_toggle", EventSwndStateChanged::EventID, OnExtraState)
        EVENT_NAME_HANDLER(L"sample_radiogroup", EventRadioGroupCheckChanged::EventID, OnExtraState)
        EVENT_NAME_HANDLER(L"sample_number", EventRENotify::EventID, OnExtraInput)
        EVENT_NAME_HANDLER(L"sample_richedit", EventRENotify::EventID, OnExtraInput)
        EVENT_NAME_HANDLER(L"sample_hotkey", EventSetHotKey::EventID, OnExtraInput)
        EVENT_NAME_HANDLER(L"sample_scrollbar", EventScroll::EventID, OnExtraInput)
        EVENT_NAME_HANDLER(L"sample_header", EventHeaderClick::EventID, OnHeader)
        EVENT_NAME_HANDLER(L"sample_calendar", EventCalendarExChanged::EventID, OnDateChanged)
        EVENT_NAME_HANDLER(L"sample_datetime", EventDateTimeChanged::EventID, OnDateChanged)
        EVENT_NAME_HANDLER(L"sample_date", EventDateTimeChanged::EventID, OnDateChanged)
        EVENT_NAME_HANDLER(L"sample_toolbar", EventToolBarCmd::EventID, OnCommandBar)
        EVENT_NAME_HANDLER(L"sample_menubar", EventSelectMenu::EventID, OnMenuCommand)
        EVENT_NAME_HANDLER(L"sample_suggestions", EventDropdownListFill::EventID, OnSuggestionFill)
        EVENT_NAME_HANDLER(L"sample_suggestions", EventDropdownListGetBuddyRect::EventID, OnSuggestionRect)
        EVENT_NAME_HANDLER(L"sample_suggestions", EventDropdownListSelected::EventID, OnSuggestionSelected)
    EVENT_MAP_END2(SHostWnd)

    BEGIN_MSG_MAP_EX(GalleryMainDialog)
        CHAIN_MSG_MAP(SDpiHandler<GalleryMainDialog>)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_DESTROY(OnDestroy)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};
