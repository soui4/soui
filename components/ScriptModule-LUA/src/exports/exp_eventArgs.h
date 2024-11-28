#include <event/SEvents.h>
#include "toobj.h"


BOOL ExpLua_EventArgs(lua_State *L)
{
	try{
        
        lua_tinker::class_add<StEventTimer>(L,"StEventTimer");
        lua_tinker::class_mem<StEventTimer>(L,"uID",&StEventTimer::uID);
		DEF_CAST_PVOID(L,StEventTimer);

        lua_tinker::class_add<StEventSwndSize>(L,"StEventSwndSize");
        lua_tinker::class_mem<StEventSwndSize>(L,"szWnd",&EventSwndSize::szWnd);
		DEF_CAST_PVOID(L,StEventSwndSize);

		lua_tinker::class_add<StEventScroll>(L,"StEventScroll");
		lua_tinker::class_mem<StEventScroll>(L,"nSbCode",&EventScroll::nSbCode);
		lua_tinker::class_mem<StEventScroll>(L,"nPos",&EventScroll::nPos);
		lua_tinker::class_mem<StEventScroll>(L,"bVertical",&EventScroll::bVertical);
		DEF_CAST_PVOID(L,StEventScroll);

		lua_tinker::class_add<StEventSpinValue2String>(L,"StEventSpinValue2String");
		lua_tinker::class_mem<StEventSpinValue2String>(L,"bInit",&EventSpinValue2String::bInit);
		lua_tinker::class_mem<StEventSpinValue2String>(L,"nValue",&EventSpinValue2String::nValue);
		lua_tinker::class_mem<StEventSpinValue2String>(L,"strValue",&EventSpinValue2String::strValue);
		DEF_CAST_PVOID(L,StEventSpinValue2String);

		lua_tinker::class_add<StEventSwndStateChanged>(L,"StEventSwndStateChanged");
		lua_tinker::class_mem<StEventSwndStateChanged>(L,"dwOldState",&EventSwndStateChanged::dwOldState);
		lua_tinker::class_mem<StEventSwndStateChanged>(L,"dwNewState",&EventSwndStateChanged::dwNewState);
		DEF_CAST_PVOID(L,StEventSwndStateChanged);

		lua_tinker::class_add<StEventSwndAnimationStart>(L,"StEventSwndAnimationStart");
		lua_tinker::class_mem<StEventSwndAnimationStart>(L,"pAni",&EventSwndAnimationStart::pAni);
		DEF_CAST_PVOID(L,StEventSwndAnimationStart);

		lua_tinker::class_add<StEventSwndAnimationStop>(L,"StEventSwndAnimationStop");
		lua_tinker::class_mem<StEventSwndAnimationStop>(L,"pAni",&EventSwndAnimationStop::pAni);
		DEF_CAST_PVOID(L,StEventSwndAnimationStop);

		lua_tinker::class_add<StEventSwndAnimationRepeat>(L,"StEventSwndAnimationRepeat");
		lua_tinker::class_mem<StEventSwndAnimationRepeat>(L,"pAni",&EventSwndAnimationRepeat::pAni);
		DEF_CAST_PVOID(L,StEventSwndAnimationRepeat);

		lua_tinker::class_add<StEventKeyDown>(L,"StEventKeyDown");
		lua_tinker::class_mem<StEventKeyDown>(L,"nChar",&EventKeyDown::nChar);
		lua_tinker::class_mem<StEventKeyDown>(L,"nFlags",&EventKeyDown::nFlags);
		lua_tinker::class_mem<StEventKeyDown>(L,"bCancel",&EventKeyDown::bCancel);
		DEF_CAST_PVOID(L,StEventKeyDown);


		lua_tinker::class_add<StEventSwndUpdateTooltip>(L,"StEventSwndUpdateTooltip");
		lua_tinker::class_mem<StEventSwndUpdateTooltip>(L,"strToolTip",&EventSwndUpdateTooltip::strToolTip);
		lua_tinker::class_mem<StEventSwndUpdateTooltip>(L,"bUpdated",&EventSwndUpdateTooltip::bUpdated);
		DEF_CAST_PVOID(L,StEventSwndUpdateTooltip);


		lua_tinker::class_add<StEventRadioGroupCheckChanged>(L,"StEventRadioGroupCheckChanged");
		lua_tinker::class_mem<StEventRadioGroupCheckChanged>(L,"pChecked",&EventRadioGroupCheckChanged::pChecked);
		DEF_CAST_PVOID(L,StEventRadioGroupCheckChanged);

		lua_tinker::class_add<StEventItemPanelRclick>(L,"StEventItemPanelRclick");
		lua_tinker::class_mem<StEventItemPanelRclick>(L,"wParam",&EventItemPanelRclick::wParam);
		lua_tinker::class_mem<StEventItemPanelRclick>(L,"lParam",&EventItemPanelRclick::lParam);
		DEF_CAST_PVOID(L,StEventItemPanelRclick);

		lua_tinker::class_add<StEventItemPanelDbclick>(L,"StEventItemPanelDbclick");
		lua_tinker::class_mem<StEventItemPanelDbclick>(L,"wParam",&EventItemPanelDbclick::wParam);
		lua_tinker::class_mem<StEventItemPanelDbclick>(L,"lParam",&EventItemPanelDbclick::lParam);
		DEF_CAST_PVOID(L,StEventItemPanelDbclick);


		lua_tinker::class_add<StEventItemPanelClick>(L,"StEventItemPanelClick");
		lua_tinker::class_mem<StEventItemPanelClick>(L,"wParam",&EventItemPanelClick::wParam);
		lua_tinker::class_mem<StEventItemPanelClick>(L,"lParam",&EventItemPanelClick::lParam);
		DEF_CAST_PVOID(L,StEventItemPanelClick);

		lua_tinker::class_add<StEventItemPanelRclickUp>(L,"StEventItemPanelRclickUp");
		lua_tinker::class_mem<StEventItemPanelRclickUp>(L,"wParam",&EventItemPanelRclickUp::wParam);
		lua_tinker::class_mem<StEventItemPanelRclickUp>(L,"lParam",&EventItemPanelRclickUp::lParam);
		DEF_CAST_PVOID(L,StEventItemPanelRclickUp);

		
		lua_tinker::class_add<StEventItemPanelClickUp>(L,"StEventItemPanelClickUp");
		lua_tinker::class_mem<StEventItemPanelClickUp>(L,"wParam",&EventItemPanelClickUp::wParam);
		lua_tinker::class_mem<StEventItemPanelClickUp>(L,"lParam",&EventItemPanelClickUp::lParam);
		DEF_CAST_PVOID(L,StEventItemPanelClickUp);

		lua_tinker::class_add<StEventItemPanelHover>(L,"StEventItemPanelHover");
		lua_tinker::class_mem<StEventItemPanelHover>(L,"wParam",&EventItemPanelHover::wParam);
		lua_tinker::class_mem<StEventItemPanelHover>(L,"lParam",&EventItemPanelHover::lParam);
		DEF_CAST_PVOID(L,StEventItemPanelHover);


		lua_tinker::class_add<StEventMouseClick>(L,"StEventMouseClick");
		lua_tinker::class_mem<StEventMouseClick>(L,"pt",&EventMouseClick::pt);
		lua_tinker::class_mem<StEventMouseClick>(L,"clickId",&EventMouseClick::clickId);
		lua_tinker::class_mem<StEventMouseClick>(L,"uFlags",&EventMouseClick::uFlags);
		DEF_CAST_PVOID(L,StEventMouseClick);

		lua_tinker::class_add<StEventCtxMenu>(L,"StEventCtxMenu");
		lua_tinker::class_mem<StEventCtxMenu>(L,"pt",&EventCtxMenu::pt);
		lua_tinker::class_mem<StEventCtxMenu>(L,"bCancel",&EventCtxMenu::bCancel);
		DEF_CAST_PVOID(L,StEventCtxMenu);

		lua_tinker::class_add<StEventSetFocus>(L,"StEventSetFocus");
		lua_tinker::class_mem<StEventSetFocus>(L,"wndOld",&EventSetFocus::wndOld);
		DEF_CAST_PVOID(L,StEventSetFocus);

		lua_tinker::class_add<StEventKillFocus>(L,"StEventKillFocus");
		lua_tinker::class_mem<StEventKillFocus>(L,"wndFocus",&EventKillFocus::wndFocus);
		DEF_CAST_PVOID(L,StEventKillFocus);


		lua_tinker::class_add<StEventScrollViewOriginChanged>(L,"StEventScrollViewOriginChanged");
		lua_tinker::class_mem<StEventScrollViewOriginChanged>(L,"ptOldOrigin",&EventScrollViewOriginChanged::ptOldOrigin);
		lua_tinker::class_mem<StEventScrollViewOriginChanged>(L,"ptNewOrigin",&EventScrollViewOriginChanged::ptNewOrigin);
		DEF_CAST_PVOID(L,StEventScrollViewOriginChanged);


		lua_tinker::class_add<StEventScrollViewSizeChanged>(L,"StEventScrollViewSizeChanged");
		lua_tinker::class_mem<StEventScrollViewSizeChanged>(L,"szOldViewSize",&EventScrollViewSizeChanged::szOldViewSize);
		lua_tinker::class_mem<StEventScrollViewSizeChanged>(L,"szNewViewSize",&EventScrollViewSizeChanged::szNewViewSize);
		DEF_CAST_PVOID(L,StEventScrollViewSizeChanged);

		lua_tinker::class_add<StEventOfEvent>(L,"StEventOfEvent");
		lua_tinker::class_mem<StEventOfEvent>(L,"pOrgEvt",&EventOfEvent::pOrgEvt);
		DEF_CAST_PVOID(L,StEventOfEvent);

		lua_tinker::class_add<StEventOfPanel>(L,"StEventOfPanel");
		lua_tinker::class_mem<StEventOfPanel>(L,"pPanel",&EventOfPanel::pPanel);
		lua_tinker::class_mem<StEventOfPanel>(L,"pOrgEvt",&EventOfPanel::pOrgEvt);
		DEF_CAST_PVOID(L,StEventOfPanel);

		lua_tinker::class_add<StEventTabSelChanging>(L,"StEventTabSelChanging");
		lua_tinker::class_mem<StEventTabSelChanging>(L,"uOldSel",&EventTabSelChanging::uOldSel);
		lua_tinker::class_mem<StEventTabSelChanging>(L,"uNewSel",&EventTabSelChanging::uNewSel);
		lua_tinker::class_mem<StEventTabSelChanging>(L,"bCancel",&EventTabSelChanging::bCancel);
		DEF_CAST_PVOID(L,StEventTabSelChanging);

		lua_tinker::class_add<StEventTabSelChanged>(L,"StEventTabSelChanged");
		lua_tinker::class_mem<StEventTabSelChanged>(L,"uOldSel",&EventTabSelChanged::uOldSel);
		lua_tinker::class_mem<StEventTabSelChanged>(L,"uNewSel",&EventTabSelChanged::uNewSel);
		DEF_CAST_PVOID(L,StEventTabSelChanged);

		lua_tinker::class_add<StEventTabItemHover>(L,"StEventTabItemHover");
		lua_tinker::class_mem<StEventTabItemHover>(L,"iHover",&EventTabItemHover::iHover);
		DEF_CAST_PVOID(L,StEventTabItemHover);

		lua_tinker::class_add<StEventTabItemLeave>(L,"StEventTabItemLeave");
		lua_tinker::class_mem<StEventTabItemLeave>(L,"iLeave",&EventTabItemLeave::iLeave);
		DEF_CAST_PVOID(L,StEventTabItemLeave);

		lua_tinker::class_add<StEventLBSelChanging>(L,"StEventLBSelChanging");
		lua_tinker::class_mem<StEventLBSelChanging>(L,"nNewSel",&EventLBSelChanging::nNewSel);
		lua_tinker::class_mem<StEventLBSelChanging>(L,"nOldSel",&EventLBSelChanging::nOldSel);
		lua_tinker::class_mem<StEventLBSelChanging>(L,"bCancel",&EventLBSelChanging::bCancel);
		DEF_CAST_PVOID(L,StEventLBSelChanging);

		lua_tinker::class_add<StEventLBSelChanged>(L,"StEventLBSelChanged");
		lua_tinker::class_mem<StEventLBSelChanged>(L,"nNewSel",&EventLBSelChanged::nNewSel);
		lua_tinker::class_mem<StEventLBSelChanged>(L,"nOldSel",&EventLBSelChanged::nOldSel);
		DEF_CAST_PVOID(L,StEventLBSelChanged);

		lua_tinker::class_add<StEventLBDbClick>(L,"StEventLBDbClick");
		lua_tinker::class_mem<StEventLBDbClick>(L,"nCurSel",&EventLBDbClick::nCurSel);
		DEF_CAST_PVOID(L,StEventLBDbClick);

		lua_tinker::class_add<StEventLVSelChanged>(L,"StEventLVSelChanged");
		lua_tinker::class_mem<StEventLVSelChanged>(L,"iOldSel",&EventLVSelChanged::iOldSel);
		lua_tinker::class_mem<StEventLVSelChanged>(L,"iNewSel",&EventLVSelChanged::iNewSel);
		DEF_CAST_PVOID(L,StEventLVSelChanged);

		lua_tinker::class_add<StEventLVSelChanging>(L,"StEventLVSelChanging");
		lua_tinker::class_mem<StEventLVSelChanging>(L,"iOldSel",&EventLVSelChanging::iOldSel);
		lua_tinker::class_mem<StEventLVSelChanging>(L,"iNewSel",&EventLVSelChanging::iNewSel);
		lua_tinker::class_mem<StEventLVSelChanging>(L,"bCancel",&EventLVSelChanging::bCancel);
		DEF_CAST_PVOID(L,StEventLVSelChanging);

		lua_tinker::class_add<StEventTVSelChanged>(L,"StEventTVSelChanged");
		lua_tinker::class_mem<StEventTVSelChanged>(L,"hOldSel",&EventTVSelChanged::hOldSel);
		lua_tinker::class_mem<StEventTVSelChanged>(L,"hNewSel",&EventTVSelChanged::hNewSel);
		DEF_CAST_PVOID(L,StEventTVSelChanged);

		lua_tinker::class_add<StEventTVSelChanging>(L,"StEventTVSelChanging");
		lua_tinker::class_mem<StEventTVSelChanging>(L,"hOldSel",&EventTVSelChanging::hOldSel);
		lua_tinker::class_mem<StEventTVSelChanging>(L,"hNewSel",&EventTVSelChanging::hNewSel);
		lua_tinker::class_mem<StEventTVSelChanging>(L,"bCancel",&EventTVSelChanging::bCancel);
		DEF_CAST_PVOID(L,StEventTVSelChanging);

		lua_tinker::class_add<StEventRENotify>(L,"StEventRENotify");
		lua_tinker::class_mem<StEventRENotify>(L,"iNotify",&EventRENotify::iNotify);
		lua_tinker::class_mem<StEventRENotify>(L,"pv",&EventRENotify::pv);
		lua_tinker::class_mem<StEventRENotify>(L,"hr",&EventRENotify::hr);
		DEF_CAST_PVOID(L,StEventRENotify);


		lua_tinker::class_add<StEventREMenu>(L,"StEventREMenu");
		lua_tinker::class_mem<StEventREMenu>(L,"uCmd",&EventREMenu::uCmd);
		DEF_CAST_PVOID(L,StEventREMenu);

		lua_tinker::class_add<StEventSliderPos>(L,"StEventSliderPos");
		lua_tinker::class_mem<StEventSliderPos>(L,"nPos",&EventSliderPos::nPos);
		DEF_CAST_PVOID(L,StEventSliderPos);

		lua_tinker::class_add<StEventHeaderClick>(L,"StEventHeaderClick");
		lua_tinker::class_mem<StEventHeaderClick>(L,"iItem",&EventHeaderClick::iItem);
		DEF_CAST_PVOID(L,StEventHeaderClick);

		lua_tinker::class_add<StEventHeaderItemChanging>(L,"StEventHeaderItemChanging");
		lua_tinker::class_mem<StEventHeaderItemChanging>(L,"iItem",&EventHeaderItemChanging::iItem);
		lua_tinker::class_mem<StEventHeaderItemChanging>(L,"nWidth",&EventHeaderItemChanging::nWidth);
		DEF_CAST_PVOID(L,StEventHeaderItemChanging);

		lua_tinker::class_add<StEventHeaderItemChanged>(L,"StEventHeaderItemChanged");
		lua_tinker::class_mem<StEventHeaderItemChanged>(L,"iItem",&EventHeaderItemChanged::iItem);
		lua_tinker::class_mem<StEventHeaderItemChanged>(L,"nWidth",&EventHeaderItemChanged::nWidth);
		DEF_CAST_PVOID(L,StEventHeaderItemChanged);

		lua_tinker::class_add<StEventHeaderItemSwap>(L,"StEventHeaderItemSwap");
		lua_tinker::class_mem<StEventHeaderItemSwap>(L,"iOldIndex",&EventHeaderItemSwap::iOldIndex);
		lua_tinker::class_mem<StEventHeaderItemSwap>(L,"iNewIndex",&EventHeaderItemSwap::iNewIndex);
		DEF_CAST_PVOID(L,StEventHeaderItemSwap);


		lua_tinker::class_add<StEventCBSelChange>(L,"StEventCBSelChange");
		lua_tinker::class_mem<StEventCBSelChange>(L,"nCurSel",&EventCBSelChange::nCurSel);
		DEF_CAST_PVOID(L,StEventCBSelChange);

		lua_tinker::class_add<StEventCBDropdown>(L,"StEventCBDropdown");
		lua_tinker::class_mem<StEventCBDropdown>(L,"pDropDown",&EventCBDropdown::pDropDown);
		DEF_CAST_PVOID(L,StEventCBDropdown);

		lua_tinker::class_add<StEventCBBeforeCloseUp>(L,"StEventCBBeforeCloseUp");
		lua_tinker::class_mem<StEventCBBeforeCloseUp>(L,"bCloseBlock",&EventCBBeforeCloseUp::bCloseBlock);
		DEF_CAST_PVOID(L,StEventCBBeforeCloseUp);

		lua_tinker::class_add<StEventLCSelChanging>(L,"StEventLCSelChanging");
		lua_tinker::class_mem<StEventLCSelChanging>(L,"nNewSel",&EventLCSelChanging::nNewSel);
		lua_tinker::class_mem<StEventLCSelChanging>(L,"nOldSel",&EventLCSelChanging::nOldSel);
		lua_tinker::class_mem<StEventLCSelChanging>(L,"bCancel",&EventLCSelChanging::bCancel);
		DEF_CAST_PVOID(L,StEventLCSelChanging);

		lua_tinker::class_add<StEventLCSelChanged>(L,"StEventLCSelChanged");
		lua_tinker::class_mem<StEventLCSelChanged>(L,"nNewSel",&EventLCSelChanged::nNewSel);
		lua_tinker::class_mem<StEventLCSelChanged>(L,"nOldSel",&EventLCSelChanged::nOldSel);
		DEF_CAST_PVOID(L,StEventLCSelChanged);

		lua_tinker::class_add<StEventLCDbClick>(L,"StEventLCDbClick");
		lua_tinker::class_mem<StEventLCDbClick>(L,"nCurSel",&EventLCDbClick::nCurSel);
		DEF_CAST_PVOID(L,StEventLCDbClick);

		lua_tinker::class_add<StEventLCItemDeleted>(L,"StEventLCItemDeleted");
		lua_tinker::class_mem<StEventLCItemDeleted>(L,"nItem",&EventLCItemDeleted::nItem);
		lua_tinker::class_mem<StEventLCItemDeleted>(L,"dwData",&EventLCItemDeleted::dwData);
		DEF_CAST_PVOID(L,StEventLCItemDeleted);

		lua_tinker::class_add<StEventCalendarSelDay>(L,"StEventCalendarSelDay");
		lua_tinker::class_mem<StEventCalendarSelDay>(L,"wOldDay",&EventCalendarSelDay::wOldDay);
		lua_tinker::class_mem<StEventCalendarSelDay>(L,"wNewDay",&EventCalendarSelDay::wNewDay);
		DEF_CAST_PVOID(L,StEventCalendarSelDay);

		lua_tinker::class_add<StEventCalendarExChanged>(L,"StEventCalendarExChanged");
		lua_tinker::class_mem<StEventCalendarExChanged>(L,"iNewDay",&EventCalendarExChanged::iNewDay);
		lua_tinker::class_mem<StEventCalendarExChanged>(L,"iNewMonth",&EventCalendarExChanged::iNewMonth);
		lua_tinker::class_mem<StEventCalendarExChanged>(L,"iNewYear",&EventCalendarExChanged::iNewYear);
		lua_tinker::class_mem<StEventCalendarExChanged>(L,"nBtnType",&EventCalendarExChanged::nBtnType);
		DEF_CAST_PVOID(L,StEventCalendarExChanged);

		lua_tinker::class_add<StEventDateTimeChanged>(L,"StEventDateTimeChanged");
		lua_tinker::class_mem<StEventDateTimeChanged>(L,"newTime",&EventDateTimeChanged::newTime);
		DEF_CAST_PVOID(L,StEventDateTimeChanged);

		lua_tinker::class_add<StEventTCSelChanging>(L,"StEventTCSelChanging");
		lua_tinker::class_mem<StEventTCSelChanging>(L,"hOldSel",&EventTCSelChanging::hOldSel);
		lua_tinker::class_mem<StEventTCSelChanging>(L,"hNewSel",&EventTCSelChanging::hNewSel);
		lua_tinker::class_mem<StEventTCSelChanging>(L,"bCancel",&EventTCSelChanging::bCancel);
		DEF_CAST_PVOID(L,StEventTCSelChanging);


		lua_tinker::class_add<StEventTCSelChanged>(L,"StEventTCSelChanged");
		lua_tinker::class_mem<StEventTCSelChanged>(L,"hOldSel",&EventTCSelChanged::hOldSel);
		lua_tinker::class_mem<StEventTCSelChanged>(L,"hNewSel",&EventTCSelChanged::hNewSel);
		DEF_CAST_PVOID(L,StEventTCSelChanged);

		lua_tinker::class_add<StEventTCCheckState>(L,"StEventTCCheckState");
		lua_tinker::class_mem<StEventTCCheckState>(L,"hItem",&EventTCCheckState::hItem);
		lua_tinker::class_mem<StEventTCCheckState>(L,"uCheckState",&EventTCCheckState::uCheckState);
		DEF_CAST_PVOID(L,StEventTCCheckState);

		lua_tinker::class_add<StEventTCExpand>(L,"StEventTCExpand");
		lua_tinker::class_mem<StEventTCExpand>(L,"hItem",&EventTCExpand::hItem);
		lua_tinker::class_mem<StEventTCExpand>(L,"bCollapsed",&EventTCExpand::bCollapsed);
		DEF_CAST_PVOID(L,StEventTCExpand);

		lua_tinker::class_add<StEventTCDbClick>(L,"StEventTCDbClick");
		lua_tinker::class_mem<StEventTCDbClick>(L,"hItem",&EventTCDbClick::hItem);
		lua_tinker::class_mem<StEventTCDbClick>(L,"bCancel",&EventTCDbClick::bCancel);
		DEF_CAST_PVOID(L,StEventTCDbClick);

		lua_tinker::class_add<StEventSplitPaneMoved>(L,"StEventSplitPaneMoved");
		lua_tinker::class_mem<StEventSplitPaneMoved>(L,"rcPane",&EventSplitPaneMoved::rcPane);
		DEF_CAST_PVOID(L,StEventSplitPaneMoved);

		lua_tinker::class_add<StEventSelectMenu>(L,"StEventSelectMenu");
		lua_tinker::class_mem<StEventSelectMenu>(L,"nMenuId",&EventSelectMenu::nMenuId);
		lua_tinker::class_mem<StEventSelectMenu>(L,"pMenu",&EventSelectMenu::pMenu);
		DEF_CAST_PVOID(L,StEventSelectMenu);

		lua_tinker::class_add<StEventPopMenu>(L,"StEventPopMenu");
		lua_tinker::class_mem<StEventPopMenu>(L,"nMenuIndex",&EventPopMenu::nMenuIndex);
		lua_tinker::class_mem<StEventPopMenu>(L,"pMenu",&EventPopMenu::pMenu);
		DEF_CAST_PVOID(L,StEventPopMenu);

		lua_tinker::class_add<StEventSetHotKey>(L,"StEventSetHotKey");
		lua_tinker::class_mem<StEventSetHotKey>(L,"vKey",&EventSetHotKey::vKey);
		lua_tinker::class_mem<StEventSetHotKey>(L,"wModifiers",&EventSetHotKey::wModifiers);
		DEF_CAST_PVOID(L,StEventSetHotKey);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}