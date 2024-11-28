#include <interface/SCtrl-i.h>

template<typename T1,typename T2>
T1 * QueryICtrl(IWindow *pWnd){
	if(pWnd->IsClass(T2::GetClassName()))
	{
		IObjRef *pCtrl=NULL;
		pWnd->QueryInterface(__suidof(T1),&pCtrl);
		if(!pCtrl) return NULL;
		return (T1*)pCtrl;
	}
	return NULL;
}

#define DEF_QICTRL(L,itype,stype) lua_tinker::def(L,"Qi"#itype,&QueryICtrl<itype,stype>)

BOOL ExpLua_ICtrl(lua_State *L)
{
	try{
		lua_tinker::class_add<ICtrl>(L,"ICtrl");
		lua_tinker::class_inh<ICtrl,IObjRef>(L);
		lua_tinker::class_def<ICtrl>(L,"ToIWindow",&ICtrl::ToIWindow);


		lua_tinker::class_add<IOsrPanel>(L,"IOsrPanel");
		lua_tinker::class_inh<IOsrPanel,ICtrl>(L);
		lua_tinker::class_def<IOsrPanel>(L,"SetItemIndex",&IOsrPanel::SetItemIndex);
		lua_tinker::class_def<IOsrPanel>(L,"GetItemIndex",&IOsrPanel::GetItemIndex);
		lua_tinker::class_def<IOsrPanel>(L,"SetItemData",&IOsrPanel::SetItemData);
		lua_tinker::class_def<IOsrPanel>(L,"GetItemData",&IOsrPanel::GetItemData);
		DEF_QICTRL(L,IOsrPanel,SOsrPanel);

		lua_tinker::class_add<IItemPanel>(L,"IItemPanel");
		lua_tinker::class_inh<IItemPanel,IOsrPanel>(L);
		lua_tinker::class_def<IItemPanel>(L,"SetSkin",&IItemPanel::SetSkin);
		lua_tinker::class_def<IItemPanel>(L,"SetColor",&IItemPanel::SetColor);
		DEF_QICTRL(L,IItemPanel,SItemPanel);

		lua_tinker::class_add<IImageWnd>(L,"IImageWnd");
		lua_tinker::class_inh<IImageWnd,ICtrl>(L);
		lua_tinker::class_def<IImageWnd>(L,"SetSkin",&IImageWnd::SetSkin);
		lua_tinker::class_def<IImageWnd>(L,"GetSkin",&IImageWnd::GetSkin);
		lua_tinker::class_def<IImageWnd>(L,"SetImage",&IImageWnd::SetImage);
		lua_tinker::class_def<IImageWnd>(L,"GetImage",&IImageWnd::GetImage);
		lua_tinker::class_def<IImageWnd>(L,"SetIcon",&IImageWnd::SetIcon);
		DEF_QICTRL(L,IImageWnd,SImageWnd);

		lua_tinker::class_add<IAnimateImgWnd>(L,"IAnimateImgWnd");
		lua_tinker::class_inh<IAnimateImgWnd,ICtrl>(L);
		lua_tinker::class_def<IAnimateImgWnd>(L,"Start",&IAnimateImgWnd::Start);
		lua_tinker::class_def<IAnimateImgWnd>(L,"Stop",&IAnimateImgWnd::Stop);
		lua_tinker::class_def<IAnimateImgWnd>(L,"IsPlaying",&IAnimateImgWnd::IsPlaying);
		DEF_QICTRL(L,IAnimateImgWnd,SAnimateImgWnd);


		lua_tinker::class_add<IProgress>(L,"IProgress");
		lua_tinker::class_inh<IProgress,ICtrl>(L);
		lua_tinker::class_def<IProgress>(L,"SetValue",&IProgress::SetValue);
		lua_tinker::class_def<IProgress>(L,"GetValue",&IProgress::GetValue);
		lua_tinker::class_def<IProgress>(L,"SetRange",&IProgress::SetRange);
		lua_tinker::class_def<IProgress>(L,"GetRange",&IProgress::GetRange);
		lua_tinker::class_def<IProgress>(L,"IsVertical",&IProgress::IsVertical);
		DEF_QICTRL(L,IProgress,SProgress);


		lua_tinker::class_add<IPanel>(L,"IPanel");
		lua_tinker::class_inh<IPanel,ICtrl>(L);
		lua_tinker::class_def<IPanel>(L,"ShowScrollBar",&IPanel::ShowScrollBar);
		lua_tinker::class_def<IPanel>(L,"EnableScrollBar",&IPanel::EnableScrollBar);
		lua_tinker::class_def<IPanel>(L,"IsScrollBarEnable",&IPanel::IsScrollBarEnable);
		lua_tinker::class_def<IPanel>(L,"SetScrollInfo",&IPanel::SetScrollInfo);
		lua_tinker::class_def<IPanel>(L,"SetScrollPos",&IPanel::SetScrollPos);
		lua_tinker::class_def<IPanel>(L,"GetScrollPos",&IPanel::GetScrollPos);
		lua_tinker::class_def<IPanel>(L,"SetScrollRange",&IPanel::SetScrollRange);
		lua_tinker::class_def<IPanel>(L,"GetScrollRange",&IPanel::GetScrollRange);
		lua_tinker::class_def<IPanel>(L,"HasScrollBar",&IPanel::HasScrollBar);
		DEF_QICTRL(L,IPanel,SPanel);

		lua_tinker::class_add<IScrollView>(L,"IScrollView");
		lua_tinker::class_inh<IScrollView,IPanel>(L);
		lua_tinker::class_def<IScrollView>(L,"GetViewSize",&IScrollView::GetViewSize);
		lua_tinker::class_def<IScrollView>(L,"SetViewSize",&IScrollView::SetViewSize);
		lua_tinker::class_def<IScrollView>(L,"GetViewOrigin",&IScrollView::GetViewOrigin);
		lua_tinker::class_def<IScrollView>(L,"SetViewOrigin",&IScrollView::SetViewOrigin);
		DEF_QICTRL(L,IScrollView,SScrollView);


		lua_tinker::class_add<IHeaderCtrl>(L,"IHeaderCtrl");
		lua_tinker::class_inh<IHeaderCtrl,ICtrl>(L);
		lua_tinker::class_def<IHeaderCtrl>(L,"InsertItem",&IHeaderCtrl::InsertItem);
		lua_tinker::class_def<IHeaderCtrl>(L,"GetItem",&IHeaderCtrl::GetItem);
		lua_tinker::class_def<IHeaderCtrl>(L,"SetItem",&IHeaderCtrl::SetItem);
		lua_tinker::class_def<IHeaderCtrl>(L,"GetItemCount",&IHeaderCtrl::GetItemCount);
		lua_tinker::class_def<IHeaderCtrl>(L,"GetTotalWidth",&IHeaderCtrl::GetTotalWidth);
		lua_tinker::class_def<IHeaderCtrl>(L,"IsAutoResize",&IHeaderCtrl::IsAutoResize);
		lua_tinker::class_def<IHeaderCtrl>(L,"GetItemWidth",&IHeaderCtrl::GetItemWidth);
		lua_tinker::class_def<IHeaderCtrl>(L,"DeleteItem",&IHeaderCtrl::DeleteItem);
		lua_tinker::class_def<IHeaderCtrl>(L,"DeleteAllItems",&IHeaderCtrl::DeleteAllItems);
		lua_tinker::class_def<IHeaderCtrl>(L,"SetItemSort",&IHeaderCtrl::SetItemSort);
		lua_tinker::class_def<IHeaderCtrl>(L,"SetItemVisible",&IHeaderCtrl::SetItemVisible);
		lua_tinker::class_def<IHeaderCtrl>(L,"IsItemVisible",&IHeaderCtrl::IsItemVisible);
		DEF_QICTRL(L,IHeaderCtrl,SHeaderCtrl);

		lua_tinker::class_add<IListView>(L,"IListView");
		lua_tinker::class_inh<IListView,IPanel>(L);
		lua_tinker::class_def<IListView>(L,"SetAdapter",&IListView::SetAdapter);
		lua_tinker::class_def<IListView>(L,"GetAdapter",&IListView::GetAdapter);
		lua_tinker::class_def<IListView>(L,"SetItemLocator",&IListView::SetItemLocator);
		lua_tinker::class_def<IListView>(L,"GetItemLocator",&IListView::GetItemLocator);
		lua_tinker::class_def<IListView>(L,"EnsureVisible",&IListView::EnsureVisible);
		lua_tinker::class_def<IListView>(L,"SetSel",&IListView::SetSel);
		lua_tinker::class_def<IListView>(L,"GetSel",&IListView::GetSel);
		lua_tinker::class_def<IListView>(L,"HitTest",&IListView::HitTest);
		DEF_QICTRL(L,IListView,SListView);


		lua_tinker::class_add<IMcListView>(L,"IMcListView");
		lua_tinker::class_inh<IMcListView,IPanel>(L);
		lua_tinker::class_def<IMcListView>(L,"SetAdapter",&IMcListView::SetAdapter);
		lua_tinker::class_def<IMcListView>(L,"GetAdapter",&IMcListView::GetAdapter);
		lua_tinker::class_def<IMcListView>(L,"SetItemLocator",&IMcListView::SetItemLocator);
		lua_tinker::class_def<IMcListView>(L,"GetItemLocator",&IMcListView::GetItemLocator);
		lua_tinker::class_def<IMcListView>(L,"EnsureVisible",&IMcListView::EnsureVisible);
		lua_tinker::class_def<IMcListView>(L,"SetSel",&IMcListView::SetSel);
		lua_tinker::class_def<IMcListView>(L,"GetSel",&IMcListView::GetSel);
		lua_tinker::class_def<IMcListView>(L,"HitTest",&IMcListView::HitTest);
		lua_tinker::class_def<IMcListView>(L,"GetIHeaderCtrl",&IMcListView::GetIHeaderCtrl);
		lua_tinker::class_def<IMcListView>(L,"InsertColumn",&IMcListView::InsertColumn);
		lua_tinker::class_def<IMcListView>(L,"DeleteColumn",&IMcListView::DeleteColumn);
		lua_tinker::class_def<IMcListView>(L,"GetColumnCount",&IMcListView::GetColumnCount);
		DEF_QICTRL(L,IMcListView,SMCListView);

		lua_tinker::class_add<ITreeView>(L,"ITreeView");
		lua_tinker::class_inh<ITreeView,IPanel>(L);
		lua_tinker::class_def<ITreeView>(L,"SetAdapter",&ITreeView::SetAdapter);
		lua_tinker::class_def<ITreeView>(L,"GetAdapter",&ITreeView::GetAdapter);
		lua_tinker::class_def<ITreeView>(L,"SetItemLocator",&ITreeView::SetItemLocator);
		lua_tinker::class_def<ITreeView>(L,"GetItemLocator",&ITreeView::GetItemLocator);
		lua_tinker::class_def<ITreeView>(L,"EnsureVisible",&ITreeView::EnsureVisible);
		lua_tinker::class_def<ITreeView>(L,"SetSel",&ITreeView::SetSel);
		lua_tinker::class_def<ITreeView>(L,"GetSel",&ITreeView::GetSel);
		lua_tinker::class_def<ITreeView>(L,"HitTest",&ITreeView::HitTest);
		DEF_QICTRL(L,ITreeView,STreeView);

		lua_tinker::class_add<ITileView>(L,"ITileView");
		lua_tinker::class_inh<ITileView,IPanel>(L);
		lua_tinker::class_def<ITileView>(L,"SetAdapter",&ITileView::SetAdapter);
		lua_tinker::class_def<ITileView>(L,"GetAdapter",&ITileView::GetAdapter);
		lua_tinker::class_def<ITileView>(L,"SetItemLocator",&ITileView::SetItemLocator);
		lua_tinker::class_def<ITileView>(L,"GetItemLocator",&ITileView::GetItemLocator);
		lua_tinker::class_def<ITileView>(L,"EnsureVisible",&ITileView::EnsureVisible);
		lua_tinker::class_def<ITileView>(L,"SetSel",&ITileView::SetSel);
		lua_tinker::class_def<ITileView>(L,"GetSel",&ITileView::GetSel);
		lua_tinker::class_def<ITileView>(L,"HitTest",&ITileView::HitTest);
		DEF_QICTRL(L,ITileView,STileView);


		lua_tinker::class_add<IListBox>(L,"IListBox");
		lua_tinker::class_inh<IListBox,IPanel>(L);
		lua_tinker::class_def<IListBox>(L,"GetCount",&IListBox::GetCount);
		lua_tinker::class_def<IListBox>(L,"GetCurSel",&IListBox::GetCurSel);
		lua_tinker::class_def<IListBox>(L,"SetCurSel",&IListBox::SetCurSel);
		lua_tinker::class_def<IListBox>(L,"GetTopIndex",&IListBox::GetTopIndex);
		lua_tinker::class_def<IListBox>(L,"SetTopIndex",&IListBox::SetTopIndex);
		lua_tinker::class_def<IListBox>(L,"GetItemHeight",&IListBox::GetItemHeight);
		lua_tinker::class_def<IListBox>(L,"SetItemHeight",&IListBox::SetItemHeight);
		lua_tinker::class_def<IListBox>(L,"GetItemData",&IListBox::GetItemData);
		lua_tinker::class_def<IListBox>(L,"SetItemData",&IListBox::SetItemData);
		lua_tinker::class_def<IListBox>(L,"GetIText",&IListBox::GetIText);
		lua_tinker::class_def<IListBox>(L,"DeleteAll",&IListBox::DeleteAll);
		lua_tinker::class_def<IListBox>(L,"DeleteString",&IListBox::DeleteString);
		lua_tinker::class_def<IListBox>(L,"AddString",&IListBox::AddString);
		lua_tinker::class_def<IListBox>(L,"InsertString",&IListBox::InsertString);
		lua_tinker::class_def<IListBox>(L,"EnsureVisible",&IListBox::EnsureVisible);
		lua_tinker::class_def<IListBox>(L,"FindString",&IListBox::FindString);
		DEF_QICTRL(L,IListBox,SListBox);

		lua_tinker::class_add<IComboBase>(L,"IComboBase");
		lua_tinker::class_inh<IComboBase,IPanel>(L);
		lua_tinker::class_def<IComboBase>(L,"DropDown",&IComboBase::DropDown);
		lua_tinker::class_def<IComboBase>(L,"CloseUp",&IComboBase::CloseUp);
		lua_tinker::class_def<IComboBase>(L,"IsDropdown",&IComboBase::IsDropdown);
		lua_tinker::class_def<IComboBase>(L,"SetDropdown",&IComboBase::SetDropdown);
		DEF_QICTRL(L,IComboBase,SComboBase);

		lua_tinker::class_add<IComboBox>(L,"IComboBox");
		lua_tinker::class_inh<IComboBox,IComboBase>(L);
		lua_tinker::class_def<IComboBox>(L,"GetItemData",&IComboBox::GetItemData);
		lua_tinker::class_def<IComboBox>(L,"SetItemData",&IComboBox::SetItemData);
		lua_tinker::class_def<IComboBox>(L,"InsertItem",&IComboBox::InsertItem);
		lua_tinker::class_def<IComboBox>(L,"DeleteString",&IComboBox::DeleteString);
		lua_tinker::class_def<IComboBox>(L,"ResetContent",&IComboBox::ResetContent);
		lua_tinker::class_def<IComboBox>(L,"GetIListBox",&IComboBox::GetIListBox);
		DEF_QICTRL(L,IComboBox,SComboBox);

		lua_tinker::class_add<IComboView>(L,"IComboView");
		lua_tinker::class_inh<IComboView,IComboBase>(L);
		lua_tinker::class_def<IComboView>(L,"GetIListView",&IComboView::GetIListView);
		DEF_QICTRL(L,IComboView,SComboView);

		lua_tinker::class_add<IDateTimePicker>(L,"IDateTimePicker");
		lua_tinker::class_inh<IDateTimePicker,ICtrl>(L);
		lua_tinker::class_def<IDateTimePicker>(L,"SetTime",&IDateTimePicker::SetTime);
		lua_tinker::class_def<IDateTimePicker>(L,"GetTime",&IDateTimePicker::GetTime);
		lua_tinker::class_def<IDateTimePicker>(L,"CloseUp",&IDateTimePicker::CloseUp);
		lua_tinker::class_def<IDateTimePicker>(L,"DropDown",&IDateTimePicker::DropDown);
		lua_tinker::class_def<IDateTimePicker>(L,"Clear",&IDateTimePicker::Clear);
		DEF_QICTRL(L,IDateTimePicker,SDateTimePicker);

		lua_tinker::class_add<ITreeCtrl>(L,"ITreeCtrl");
		lua_tinker::class_inh<ITreeCtrl,ICtrl>(L);
		lua_tinker::class_def<ITreeCtrl>(L,"InsertItem",&ITreeCtrl:: InsertItem);
		lua_tinker::class_def<ITreeCtrl>(L,"RemoveItem",&ITreeCtrl:: RemoveItem);
		lua_tinker::class_def<ITreeCtrl>(L,"RemoveAllItems",&ITreeCtrl:: RemoveAllItems);
		lua_tinker::class_def<ITreeCtrl>(L,"GetRootItem",&ITreeCtrl:: GetRootItem);
		lua_tinker::class_def<ITreeCtrl>(L,"GetNextSiblingItem",&ITreeCtrl:: GetNextSiblingItem);
		lua_tinker::class_def<ITreeCtrl>(L,"GetPrevSiblingItem",&ITreeCtrl:: GetPrevSiblingItem);
		lua_tinker::class_def<ITreeCtrl>(L,"GetChildItem",&ITreeCtrl:: GetChildItem);
		lua_tinker::class_def<ITreeCtrl>(L,"GetParentItem",&ITreeCtrl:: GetParentItem);
		lua_tinker::class_def<ITreeCtrl>(L,"GetSelectedItem",&ITreeCtrl:: GetSelectedItem);
		lua_tinker::class_def<ITreeCtrl>(L,"GetNextItem",&ITreeCtrl:: GetNextItem);
		lua_tinker::class_def<ITreeCtrl>(L,"SortChildren",&ITreeCtrl:: SortChildren);
		lua_tinker::class_def<ITreeCtrl>(L,"SelectItem",&ITreeCtrl:: SelectItem);
		lua_tinker::class_def<ITreeCtrl>(L,"GetItemText",&ITreeCtrl:: GetItemText);
		lua_tinker::class_def<ITreeCtrl>(L,"SetItemText",&ITreeCtrl:: SetItemText);
		lua_tinker::class_def<ITreeCtrl>(L,"GetItemImage",&ITreeCtrl:: GetItemImage);
		lua_tinker::class_def<ITreeCtrl>(L,"SetItemImage",&ITreeCtrl:: SetItemImage);
		lua_tinker::class_def<ITreeCtrl>(L,"GetItemData",&ITreeCtrl:: GetItemData);
		lua_tinker::class_def<ITreeCtrl>(L,"SetItemData",&ITreeCtrl:: SetItemData);
		lua_tinker::class_def<ITreeCtrl>(L,"ItemHasChildren",&ITreeCtrl:: ItemHasChildren);
		lua_tinker::class_def<ITreeCtrl>(L,"GetCheckState",&ITreeCtrl:: GetCheckState);
		lua_tinker::class_def<ITreeCtrl>(L,"SetCheckState",&ITreeCtrl:: SetCheckState);
		lua_tinker::class_def<ITreeCtrl>(L,"Expand",&ITreeCtrl:: Expand);
		lua_tinker::class_def<ITreeCtrl>(L,"EnsureVisible",&ITreeCtrl:: EnsureVisible);
		DEF_QICTRL(L,ITreeCtrl,STreeCtrl);

		lua_tinker::class_add<IHotKeyCtrl>(L,"IHotKeyCtrl");
		lua_tinker::class_inh<IHotKeyCtrl,ICtrl>(L);
		lua_tinker::class_def<IHotKeyCtrl>(L,"SetRule",&IHotKeyCtrl:: SetRule);
		lua_tinker::class_def<IHotKeyCtrl>(L,"SetHotKey",&IHotKeyCtrl:: SetHotKey);
		lua_tinker::class_def<IHotKeyCtrl>(L,"GetHotKey",&IHotKeyCtrl:: GetHotKey);
		DEF_QICTRL(L,IHotKeyCtrl,SHotKeyCtrl);

		lua_tinker::class_add<IRichEdit>(L,"IRichEdit");
		lua_tinker::class_inh<IRichEdit,ICtrl>(L);
		lua_tinker::class_def<IRichEdit>(L,"SaveRtf",&IRichEdit::SaveRtf);
		lua_tinker::class_def<IRichEdit>(L,"LoadRtf",&IRichEdit::LoadRtf);
		lua_tinker::class_def<IRichEdit>(L,"SetSel",&IRichEdit::SetSel);
		lua_tinker::class_def<IRichEdit>(L,"ReplaceSel",&IRichEdit::ReplaceSel);
		lua_tinker::class_def<IRichEdit>(L,"GetWordWrap",&IRichEdit::GetWordWrap);
		lua_tinker::class_def<IRichEdit>(L,"SetWordWrap",&IRichEdit::SetWordWrap);
		lua_tinker::class_def<IRichEdit>(L,"GetReadOnly",&IRichEdit::GetReadOnly);
		lua_tinker::class_def<IRichEdit>(L,"SetReadOnly",&IRichEdit::SetReadOnly);
		lua_tinker::class_def<IRichEdit>(L,"GetLimitText",&IRichEdit::GetLimitText);
		lua_tinker::class_def<IRichEdit>(L,"SetLimitText",&IRichEdit::SetLimitText);
		lua_tinker::class_def<IRichEdit>(L,"GetDefaultAlign",&IRichEdit::GetDefaultAlign);
		lua_tinker::class_def<IRichEdit>(L,"SetDefaultAlign",&IRichEdit::SetDefaultAlign);
		lua_tinker::class_def<IRichEdit>(L,"GetRichTextFlag",&IRichEdit::GetRichTextFlag);
		lua_tinker::class_def<IRichEdit>(L,"SetRichTextFlag",&IRichEdit::SetRichTextFlag);
		lua_tinker::class_def<IRichEdit>(L,"GetDefaultLeftIndent",&IRichEdit::GetDefaultLeftIndent);
		lua_tinker::class_def<IRichEdit>(L,"SetDefaultLeftIndent",&IRichEdit::SetDefaultLeftIndent);
		lua_tinker::class_def<IRichEdit>(L,"SetSaveSelection",&IRichEdit::SetSaveSelection);
		lua_tinker::class_def<IRichEdit>(L,"SetDefaultTextColor",&IRichEdit::SetDefaultTextColor);
		DEF_QICTRL(L,IRichEdit,SRichEdit);

		lua_tinker::class_add<IEdit>(L,"IEdit");
		lua_tinker::class_inh<IEdit,ICtrl>(L);
		lua_tinker::class_def<IEdit>(L,"GetCueText",&IEdit::GetCueText);
		lua_tinker::class_def<IEdit>(L,"GetCueColor",&IEdit::GetCueColor);
		DEF_QICTRL(L,IEdit,SEdit);

		lua_tinker::class_add<ITabCtrl>(L,"ITabCtrl");
		lua_tinker::class_inh<ITabCtrl,ICtrl>(L);
		lua_tinker::class_def<ITabCtrl>(L,"GetCurSel",&ITabCtrl::GetCurSel);
		lua_tinker::class_def<ITabCtrl>(L,"SetCurSel",&ITabCtrl::SetCurSel);
		lua_tinker::class_def<ITabCtrl>(L,"SetItemTitle",&ITabCtrl::SetItemTitle);
		lua_tinker::class_def<ITabCtrl>(L,"InsertItem",&ITabCtrl::InsertItem);
		lua_tinker::class_def<ITabCtrl>(L,"GetItemCount",&ITabCtrl::GetItemCount);
		lua_tinker::class_def<ITabCtrl>(L,"GetPage",&ITabCtrl::GetPage);
		lua_tinker::class_def<ITabCtrl>(L,"RemoveItem",&ITabCtrl::RemoveItem);
		lua_tinker::class_def<ITabCtrl>(L,"RemoveAllItems",&ITabCtrl::RemoveAllItems);
		lua_tinker::class_def<ITabCtrl>(L,"GetPageIndex",&ITabCtrl::GetPageIndex);
		DEF_QICTRL(L,ITabCtrl,STabCtrl);

		lua_tinker::class_add<ISpinButtonCtrl>(L,"ISpinButtonCtrl");
		lua_tinker::class_inh<ISpinButtonCtrl,ICtrl>(L);
		lua_tinker::class_def<ISpinButtonCtrl>(L,"SetValue",&ISpinButtonCtrl::SetValue);
		lua_tinker::class_def<ISpinButtonCtrl>(L,"SetRange",&ISpinButtonCtrl::SetRange);
		lua_tinker::class_def<ISpinButtonCtrl>(L,"SetStep",&ISpinButtonCtrl::SetStep);
		lua_tinker::class_def<ISpinButtonCtrl>(L,"GetValue",&ISpinButtonCtrl::GetValue);
		lua_tinker::class_def<ISpinButtonCtrl>(L,"GetIBuddy",&ISpinButtonCtrl::GetIBuddy);
		DEF_QICTRL(L,ISpinButtonCtrl,SSpinButtonCtrl);

		lua_tinker::class_add<IIconWnd>(L,"IIconWnd");
		lua_tinker::class_inh<IIconWnd,ICtrl>(L);
		lua_tinker::class_def<IIconWnd>(L,"SetIcon",&IIconWnd::SetIcon);
		DEF_QICTRL(L,IIconWnd,SIconWnd);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}