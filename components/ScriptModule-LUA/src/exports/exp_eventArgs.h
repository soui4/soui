#include <event/SEvents.h>
#include "toobj.h"


BOOL ExpLua_EventArgs(lua_State *L)
{
	try{
        
        lua_tinker::class_add<EventTimer>(L,"EventTimer");
		lua_tinker::class_inh<EventTimer,IEvtArgs>(L);
        lua_tinker::class_mem<EventTimer>(L,"uID",&StEventTimer::uID);
		DEF_TOOBJ(L,EventTimer);
		lua_tinker::def(L,"toStEventTimer",&sobj_cast<EventTimer>); // backward-compat alias

        lua_tinker::class_add<EventSwndSize>(L,"EventSwndSize");
		lua_tinker::class_inh<EventSwndSize,IEvtArgs>(L);
        lua_tinker::class_mem<EventSwndSize>(L,"szWnd",&EventSwndSize::szWnd);
		DEF_TOOBJ(L,EventSwndSize);
		lua_tinker::def(L,"toStEventSwndSize",&sobj_cast<EventSwndSize>); // backward-compat alias

		lua_tinker::class_add<EventScroll>(L,"EventScroll");
		lua_tinker::class_inh<EventScroll,IEvtArgs>(L);
		lua_tinker::class_mem<EventScroll>(L,"nSbCode",&EventScroll::nSbCode);
		lua_tinker::class_mem<EventScroll>(L,"nPos",&EventScroll::nPos);
		lua_tinker::class_mem<EventScroll>(L,"bVertical",&EventScroll::bVertical);
		DEF_TOOBJ(L,EventScroll);
		lua_tinker::def(L,"toStEventScroll",&sobj_cast<EventScroll>); // backward-compat alias

		lua_tinker::class_add<EventSpinValue2String>(L,"EventSpinValue2String");
		lua_tinker::class_inh<EventSpinValue2String,IEvtArgs>(L);
		lua_tinker::class_mem<EventSpinValue2String>(L,"bInit",&EventSpinValue2String::bInit);
		lua_tinker::class_mem<EventSpinValue2String>(L,"nValue",&EventSpinValue2String::nValue);
		lua_tinker::class_mem<EventSpinValue2String>(L,"strValue",&EventSpinValue2String::strValue);
		DEF_TOOBJ(L,EventSpinValue2String);
		lua_tinker::def(L,"toStEventSpinValue2String",&sobj_cast<EventSpinValue2String>); // backward-compat alias

		lua_tinker::class_add<EventSwndStateChanged>(L,"EventSwndStateChanged");
		lua_tinker::class_inh<EventSwndStateChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventSwndStateChanged>(L,"dwOldState",&EventSwndStateChanged::dwOldState);
		lua_tinker::class_mem<EventSwndStateChanged>(L,"dwNewState",&EventSwndStateChanged::dwNewState);
		DEF_TOOBJ(L,EventSwndStateChanged);
		lua_tinker::def(L,"toStEventSwndStateChanged",&sobj_cast<EventSwndStateChanged>); // backward-compat alias

		lua_tinker::class_add<EventSwndAnimationStart>(L,"EventSwndAnimationStart");
		lua_tinker::class_inh<EventSwndAnimationStart,IEvtArgs>(L);
		lua_tinker::class_mem<EventSwndAnimationStart>(L,"pAni",&EventSwndAnimationStart::pAni);
		DEF_TOOBJ(L,EventSwndAnimationStart);
		lua_tinker::def(L,"toStEventSwndAnimationStart",&sobj_cast<EventSwndAnimationStart>); // backward-compat alias

		lua_tinker::class_add<EventSwndAnimationStop>(L,"EventSwndAnimationStop");
		lua_tinker::class_inh<EventSwndAnimationStop,IEvtArgs>(L);
		lua_tinker::class_mem<EventSwndAnimationStop>(L,"pAni",&EventSwndAnimationStop::pAni);
		DEF_TOOBJ(L,EventSwndAnimationStop);
		lua_tinker::def(L,"toStEventSwndAnimationStop",&sobj_cast<EventSwndAnimationStop>); // backward-compat alias

		lua_tinker::class_add<EventSwndAnimationRepeat>(L,"EventSwndAnimationRepeat");
		lua_tinker::class_inh<EventSwndAnimationRepeat,IEvtArgs>(L);
		lua_tinker::class_mem<EventSwndAnimationRepeat>(L,"pAni",&EventSwndAnimationRepeat::pAni);
		DEF_TOOBJ(L,EventSwndAnimationRepeat);
		lua_tinker::def(L,"toStEventSwndAnimationRepeat",&sobj_cast<EventSwndAnimationRepeat>); // backward-compat alias

		lua_tinker::class_add<EventKeyDown>(L,"EventKeyDown");
		lua_tinker::class_inh<EventKeyDown,IEvtArgs>(L);
		lua_tinker::class_mem<EventKeyDown>(L,"nChar",&EventKeyDown::nChar);
		lua_tinker::class_mem<EventKeyDown>(L,"nFlags",&EventKeyDown::nFlags);
		lua_tinker::class_mem<EventKeyDown>(L,"nRepCnt",&EventKeyDown::nRepCnt);
		DEF_TOOBJ(L,EventKeyDown);
		lua_tinker::def(L,"toStEventKeyDown",&sobj_cast<EventKeyDown>); // backward-compat alias

		lua_tinker::class_add<EventKeyUp>(L,"EventKeyUp");
		lua_tinker::class_inh<EventKeyUp,IEvtArgs>(L);
		lua_tinker::class_mem<EventKeyUp>(L,"nChar",&EventKeyUp::nChar);
		lua_tinker::class_mem<EventKeyUp>(L,"nRepCnt",&EventKeyUp::nRepCnt);
		DEF_TOOBJ(L,EventKeyUp);
		lua_tinker::def(L,"toStEventKeyUp",&sobj_cast<EventKeyUp>); // backward-compat alias

		lua_tinker::class_add<EventSwndUpdateTooltip>(L,"EventSwndUpdateTooltip");
		lua_tinker::class_inh<EventSwndUpdateTooltip,IEvtArgs>(L);
		lua_tinker::class_mem<EventSwndUpdateTooltip>(L,"strToolTip",&EventSwndUpdateTooltip::strToolTip);
		lua_tinker::class_mem<EventSwndUpdateTooltip>(L,"bUpdated",&EventSwndUpdateTooltip::bUpdated);
		DEF_TOOBJ(L,EventSwndUpdateTooltip);
		lua_tinker::def(L,"toStEventSwndUpdateTooltip",&sobj_cast<EventSwndUpdateTooltip>); // backward-compat alias


		lua_tinker::class_add<EventRadioGroupCheckChanged>(L,"EventRadioGroupCheckChanged");
		lua_tinker::class_inh<EventRadioGroupCheckChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventRadioGroupCheckChanged>(L,"pChecked",&EventRadioGroupCheckChanged::pChecked);
		DEF_TOOBJ(L,EventRadioGroupCheckChanged);
		lua_tinker::def(L,"toStEventRadioGroupCheckChanged",&sobj_cast<EventRadioGroupCheckChanged>); // backward-compat alias

		lua_tinker::class_add<EventItemPanelRclick>(L,"EventItemPanelRclick");
		lua_tinker::class_inh<EventItemPanelRclick,IEvtArgs>(L);
		lua_tinker::class_mem<EventItemPanelRclick>(L,"wParam",&EventItemPanelRclick::wParam);
		lua_tinker::class_mem<EventItemPanelRclick>(L,"lParam",&EventItemPanelRclick::lParam);
		DEF_TOOBJ(L,EventItemPanelRclick);
		lua_tinker::def(L,"toStEventItemPanelRclick",&sobj_cast<EventItemPanelRclick>); // backward-compat alias

		lua_tinker::class_add<EventItemPanelDbclick>(L,"EventItemPanelDbclick");
		lua_tinker::class_inh<EventItemPanelDbclick,IEvtArgs>(L);
		lua_tinker::class_mem<EventItemPanelDbclick>(L,"wParam",&EventItemPanelDbclick::wParam);
		lua_tinker::class_mem<EventItemPanelDbclick>(L,"lParam",&EventItemPanelDbclick::lParam);
		DEF_TOOBJ(L,EventItemPanelDbclick);
		lua_tinker::def(L,"toStEventItemPanelDbclick",&sobj_cast<EventItemPanelDbclick>); // backward-compat alias


		lua_tinker::class_add<EventItemPanelClick>(L,"EventItemPanelClick");
		lua_tinker::class_inh<EventItemPanelClick,IEvtArgs>(L);
		lua_tinker::class_mem<EventItemPanelClick>(L,"wParam",&EventItemPanelClick::wParam);
		lua_tinker::class_mem<EventItemPanelClick>(L,"lParam",&EventItemPanelClick::lParam);
		DEF_TOOBJ(L,EventItemPanelClick);
		lua_tinker::def(L,"toStEventItemPanelClick",&sobj_cast<EventItemPanelClick>); // backward-compat alias

		lua_tinker::class_add<EventItemPanelRclickUp>(L,"EventItemPanelRclickUp");
		lua_tinker::class_inh<EventItemPanelRclickUp,IEvtArgs>(L);
		lua_tinker::class_mem<EventItemPanelRclickUp>(L,"wParam",&EventItemPanelRclickUp::wParam);
		lua_tinker::class_mem<EventItemPanelRclickUp>(L,"lParam",&EventItemPanelRclickUp::lParam);
		DEF_TOOBJ(L,EventItemPanelRclickUp);
		lua_tinker::def(L,"toStEventItemPanelRclickUp",&sobj_cast<EventItemPanelRclickUp>); // backward-compat alias

		
		lua_tinker::class_add<EventItemPanelClickUp>(L,"EventItemPanelClickUp");
		lua_tinker::class_inh<EventItemPanelClickUp,IEvtArgs>(L);
		lua_tinker::class_mem<EventItemPanelClickUp>(L,"wParam",&EventItemPanelClickUp::wParam);
		lua_tinker::class_mem<EventItemPanelClickUp>(L,"lParam",&EventItemPanelClickUp::lParam);
		DEF_TOOBJ(L,EventItemPanelClickUp);
		lua_tinker::def(L,"toStEventItemPanelClickUp",&sobj_cast<EventItemPanelClickUp>); // backward-compat alias

		lua_tinker::class_add<EventItemPanelHover>(L,"EventItemPanelHover");
		lua_tinker::class_inh<EventItemPanelHover,IEvtArgs>(L);
		lua_tinker::class_mem<EventItemPanelHover>(L,"wParam",&EventItemPanelHover::wParam);
		lua_tinker::class_mem<EventItemPanelHover>(L,"lParam",&EventItemPanelHover::lParam);
		DEF_TOOBJ(L,EventItemPanelHover);
		lua_tinker::def(L,"toStEventItemPanelHover",&sobj_cast<EventItemPanelHover>); // backward-compat alias


		lua_tinker::class_add<EventMouseClick>(L,"EventMouseClick");
		lua_tinker::class_inh<EventMouseClick,IEvtArgs>(L);
		lua_tinker::class_mem<EventMouseClick>(L,"pt",&EventMouseClick::pt);
		lua_tinker::class_mem<EventMouseClick>(L,"clickId",&EventMouseClick::clickId);
		lua_tinker::class_mem<EventMouseClick>(L,"uFlags",&EventMouseClick::uFlags);
		DEF_TOOBJ(L,EventMouseClick);
		lua_tinker::def(L,"toStEventMouseClick",&sobj_cast<EventMouseClick>); // backward-compat alias

		lua_tinker::class_add<EventCtxMenu>(L,"EventCtxMenu");
		lua_tinker::class_inh<EventCtxMenu,IEvtArgs>(L);
		lua_tinker::class_mem<EventCtxMenu>(L,"pt",&EventCtxMenu::pt);
		lua_tinker::class_mem<EventCtxMenu>(L,"bCancel",&EventCtxMenu::bCancel);
		DEF_TOOBJ(L,EventCtxMenu);
		lua_tinker::def(L,"toStEventCtxMenu",&sobj_cast<EventCtxMenu>); // backward-compat alias

		lua_tinker::class_add<EventSetFocus>(L,"EventSetFocus");
		lua_tinker::class_inh<EventSetFocus,IEvtArgs>(L);
		lua_tinker::class_mem<EventSetFocus>(L,"wndOld",&EventSetFocus::wndOld);
		DEF_TOOBJ(L,EventSetFocus);
		lua_tinker::def(L,"toStEventSetFocus",&sobj_cast<EventSetFocus>); // backward-compat alias

		lua_tinker::class_add<EventKillFocus>(L,"EventKillFocus");
		lua_tinker::class_inh<EventKillFocus,IEvtArgs>(L);
		lua_tinker::class_mem<EventKillFocus>(L,"wndFocus",&EventKillFocus::wndFocus);
		DEF_TOOBJ(L,EventKillFocus);
		lua_tinker::def(L,"toStEventKillFocus",&sobj_cast<EventKillFocus>); // backward-compat alias


		lua_tinker::class_add<EventScrollViewOriginChanged>(L,"EventScrollViewOriginChanged");
		lua_tinker::class_inh<EventScrollViewOriginChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventScrollViewOriginChanged>(L,"ptOldOrigin",&EventScrollViewOriginChanged::ptOldOrigin);
		lua_tinker::class_mem<EventScrollViewOriginChanged>(L,"ptNewOrigin",&EventScrollViewOriginChanged::ptNewOrigin);
		DEF_TOOBJ(L,EventScrollViewOriginChanged);
		lua_tinker::def(L,"toStEventScrollViewOriginChanged",&sobj_cast<EventScrollViewOriginChanged>); // backward-compat alias


		lua_tinker::class_add<EventScrollViewSizeChanged>(L,"EventScrollViewSizeChanged");
		lua_tinker::class_inh<EventScrollViewSizeChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventScrollViewSizeChanged>(L,"szOldViewSize",&EventScrollViewSizeChanged::szOldViewSize);
		lua_tinker::class_mem<EventScrollViewSizeChanged>(L,"szNewViewSize",&EventScrollViewSizeChanged::szNewViewSize);
		DEF_TOOBJ(L,EventScrollViewSizeChanged);
		lua_tinker::def(L,"toStEventScrollViewSizeChanged",&sobj_cast<EventScrollViewSizeChanged>); // backward-compat alias

		lua_tinker::class_add<EventOfEvent>(L,"EventOfEvent");
		lua_tinker::class_inh<EventOfEvent,IEvtArgs>(L);
		lua_tinker::class_mem<EventOfEvent>(L,"pOrgEvt",&EventOfEvent::pOrgEvt);
		DEF_TOOBJ(L,EventOfEvent);
		lua_tinker::def(L,"toStEventOfEvent",&sobj_cast<EventOfEvent>); // backward-compat alias

		lua_tinker::class_add<EventOfPanel>(L,"EventOfPanel");
		lua_tinker::class_inh<EventOfPanel,IEvtArgs>(L);
		lua_tinker::class_mem<EventOfPanel>(L,"pPanel",&EventOfPanel::pPanel);
		lua_tinker::class_mem<EventOfPanel>(L,"pOrgEvt",&EventOfPanel::pOrgEvt);
		DEF_TOOBJ(L,EventOfPanel);
		lua_tinker::def(L,"toStEventOfPanel",&sobj_cast<EventOfPanel>); // backward-compat alias

		lua_tinker::class_add<EventTabSelChanging>(L,"EventTabSelChanging");
		lua_tinker::class_inh<EventTabSelChanging,IEvtArgs>(L);
		lua_tinker::class_mem<EventTabSelChanging>(L,"uOldSel",&EventTabSelChanging::uOldSel);
		lua_tinker::class_mem<EventTabSelChanging>(L,"uNewSel",&EventTabSelChanging::uNewSel);
		lua_tinker::class_mem<EventTabSelChanging>(L,"bCancel",&EventTabSelChanging::bCancel);
		DEF_TOOBJ(L,EventTabSelChanging);
		lua_tinker::def(L,"toStEventTabSelChanging",&sobj_cast<EventTabSelChanging>); // backward-compat alias

		lua_tinker::class_add<EventTabSelChanged>(L,"EventTabSelChanged");
		lua_tinker::class_inh<EventTabSelChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventTabSelChanged>(L,"uOldSel",&EventTabSelChanged::uOldSel);
		lua_tinker::class_mem<EventTabSelChanged>(L,"uNewSel",&EventTabSelChanged::uNewSel);
		DEF_TOOBJ(L,EventTabSelChanged);
		lua_tinker::def(L,"toStEventTabSelChanged",&sobj_cast<EventTabSelChanged>); // backward-compat alias

		lua_tinker::class_add<EventTabItemHover>(L,"EventTabItemHover");
		lua_tinker::class_inh<EventTabItemHover,IEvtArgs>(L);
		lua_tinker::class_mem<EventTabItemHover>(L,"iHover",&EventTabItemHover::iHover);
		DEF_TOOBJ(L,EventTabItemHover);
		lua_tinker::def(L,"toStEventTabItemHover",&sobj_cast<EventTabItemHover>); // backward-compat alias

		lua_tinker::class_add<EventTabItemLeave>(L,"EventTabItemLeave");
		lua_tinker::class_inh<EventTabItemLeave,IEvtArgs>(L);
		lua_tinker::class_mem<EventTabItemLeave>(L,"iLeave",&EventTabItemLeave::iLeave);
		DEF_TOOBJ(L,EventTabItemLeave);
		lua_tinker::def(L,"toStEventTabItemLeave",&sobj_cast<EventTabItemLeave>); // backward-compat alias

		lua_tinker::class_add<EventLBSelChanging>(L,"EventLBSelChanging");
		lua_tinker::class_inh<EventLBSelChanging,IEvtArgs>(L);
		lua_tinker::class_mem<EventLBSelChanging>(L,"nNewSel",&EventLBSelChanging::nNewSel);
		lua_tinker::class_mem<EventLBSelChanging>(L,"nOldSel",&EventLBSelChanging::nOldSel);
		lua_tinker::class_mem<EventLBSelChanging>(L,"bCancel",&EventLBSelChanging::bCancel);
		DEF_TOOBJ(L,EventLBSelChanging);
		lua_tinker::def(L,"toStEventLBSelChanging",&sobj_cast<EventLBSelChanging>); // backward-compat alias

		lua_tinker::class_add<EventLBSelChanged>(L,"EventLBSelChanged");
		lua_tinker::class_inh<EventLBSelChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventLBSelChanged>(L,"nNewSel",&EventLBSelChanged::nNewSel);
		lua_tinker::class_mem<EventLBSelChanged>(L,"nOldSel",&EventLBSelChanged::nOldSel);
		DEF_TOOBJ(L,EventLBSelChanged);
		lua_tinker::def(L,"toStEventLBSelChanged",&sobj_cast<EventLBSelChanged>); // backward-compat alias

		lua_tinker::class_add<EventLBDbClick>(L,"EventLBDbClick");
		lua_tinker::class_inh<EventLBDbClick,IEvtArgs>(L);
		lua_tinker::class_mem<EventLBDbClick>(L,"nCurSel",&EventLBDbClick::nCurSel);
		DEF_TOOBJ(L,EventLBDbClick);
		lua_tinker::def(L,"toStEventLBDbClick",&sobj_cast<EventLBDbClick>); // backward-compat alias

		lua_tinker::class_add<EventLVSelChanged>(L,"EventLVSelChanged");
		lua_tinker::class_inh<EventLVSelChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventLVSelChanged>(L,"iOldSel",&EventLVSelChanged::iOldSel);
		lua_tinker::class_mem<EventLVSelChanged>(L,"iNewSel",&EventLVSelChanged::iNewSel);
		DEF_TOOBJ(L,EventLVSelChanged);
		lua_tinker::def(L,"toStEventLVSelChanged",&sobj_cast<EventLVSelChanged>); // backward-compat alias

		lua_tinker::class_add<EventLVSelChanging>(L,"EventLVSelChanging");
		lua_tinker::class_inh<EventLVSelChanging,IEvtArgs>(L);
		lua_tinker::class_mem<EventLVSelChanging>(L,"iOldSel",&EventLVSelChanging::iOldSel);
		lua_tinker::class_mem<EventLVSelChanging>(L,"iNewSel",&EventLVSelChanging::iNewSel);
		lua_tinker::class_mem<EventLVSelChanging>(L,"bCancel",&EventLVSelChanging::bCancel);
		DEF_TOOBJ(L,EventLVSelChanging);
		lua_tinker::def(L,"toStEventLVSelChanging",&sobj_cast<EventLVSelChanging>); // backward-compat alias

		lua_tinker::class_add<EventTVSelChanged>(L,"EventTVSelChanged");
		lua_tinker::class_inh<EventTVSelChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventTVSelChanged>(L,"hOldSel",&EventTVSelChanged::hOldSel);
		lua_tinker::class_mem<EventTVSelChanged>(L,"hNewSel",&EventTVSelChanged::hNewSel);
		DEF_TOOBJ(L,EventTVSelChanged);
		lua_tinker::def(L,"toStEventTVSelChanged",&sobj_cast<EventTVSelChanged>); // backward-compat alias

		lua_tinker::class_add<EventTVSelChanging>(L,"EventTVSelChanging");
		lua_tinker::class_inh<EventTVSelChanging,IEvtArgs>(L);
		lua_tinker::class_mem<EventTVSelChanging>(L,"hOldSel",&EventTVSelChanging::hOldSel);
		lua_tinker::class_mem<EventTVSelChanging>(L,"hNewSel",&EventTVSelChanging::hNewSel);
		lua_tinker::class_mem<EventTVSelChanging>(L,"bCancel",&EventTVSelChanging::bCancel);
		DEF_TOOBJ(L,EventTVSelChanging);
		lua_tinker::def(L,"toStEventTVSelChanging",&sobj_cast<EventTVSelChanging>); // backward-compat alias

		lua_tinker::class_add<EventRENotify>(L,"EventRENotify");
		lua_tinker::class_inh<EventRENotify,IEvtArgs>(L);
		lua_tinker::class_mem<EventRENotify>(L,"iNotify",&EventRENotify::iNotify);
		lua_tinker::class_mem<EventRENotify>(L,"pv",&EventRENotify::pv);
		lua_tinker::class_mem<EventRENotify>(L,"hr",&EventRENotify::hr);
		DEF_TOOBJ(L,EventRENotify);
		lua_tinker::def(L,"toStEventRENotify",&sobj_cast<EventRENotify>); // backward-compat alias


		lua_tinker::class_add<EventREMenu>(L,"EventREMenu");
		lua_tinker::class_inh<EventREMenu,IEvtArgs>(L);
		lua_tinker::class_mem<EventREMenu>(L,"uCmd",&EventREMenu::uCmd);
		DEF_TOOBJ(L,EventREMenu);
		lua_tinker::def(L,"toStEventREMenu",&sobj_cast<EventREMenu>); // backward-compat alias

		lua_tinker::class_add<EventSliderPos>(L,"EventSliderPos");
		lua_tinker::class_inh<EventSliderPos,IEvtArgs>(L);
		lua_tinker::class_mem<EventSliderPos>(L,"nPos",&EventSliderPos::nPos);
		DEF_TOOBJ(L,EventSliderPos);
		lua_tinker::def(L,"toStEventSliderPos",&sobj_cast<EventSliderPos>); // backward-compat alias

		lua_tinker::class_add<EventHeaderClick>(L,"EventHeaderClick");
		lua_tinker::class_inh<EventHeaderClick,IEvtArgs>(L);
		lua_tinker::class_mem<EventHeaderClick>(L,"iItem",&EventHeaderClick::iItem);
		DEF_TOOBJ(L,EventHeaderClick);
		lua_tinker::def(L,"toStEventHeaderClick",&sobj_cast<EventHeaderClick>); // backward-compat alias

		lua_tinker::class_add<EventHeaderItemChanging>(L,"EventHeaderItemChanging");
		lua_tinker::class_inh<EventHeaderItemChanging,IEvtArgs>(L);
		lua_tinker::class_mem<EventHeaderItemChanging>(L,"iItem",&EventHeaderItemChanging::iItem);
		lua_tinker::class_mem<EventHeaderItemChanging>(L,"nWidth",&EventHeaderItemChanging::nWidth);
		DEF_TOOBJ(L,EventHeaderItemChanging);
		lua_tinker::def(L,"toStEventHeaderItemChanging",&sobj_cast<EventHeaderItemChanging>); // backward-compat alias

		lua_tinker::class_add<EventHeaderItemChanged>(L,"EventHeaderItemChanged");
		lua_tinker::class_inh<EventHeaderItemChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventHeaderItemChanged>(L,"iItem",&EventHeaderItemChanged::iItem);
		lua_tinker::class_mem<EventHeaderItemChanged>(L,"nWidth",&EventHeaderItemChanged::nWidth);
		DEF_TOOBJ(L,EventHeaderItemChanged);
		lua_tinker::def(L,"toStEventHeaderItemChanged",&sobj_cast<EventHeaderItemChanged>); // backward-compat alias

		lua_tinker::class_add<EventHeaderItemSwap>(L,"EventHeaderItemSwap");
		lua_tinker::class_inh<EventHeaderItemSwap,IEvtArgs>(L);
		lua_tinker::class_mem<EventHeaderItemSwap>(L,"iOldIndex",&EventHeaderItemSwap::iOldIndex);
		lua_tinker::class_mem<EventHeaderItemSwap>(L,"iNewIndex",&EventHeaderItemSwap::iNewIndex);
		DEF_TOOBJ(L,EventHeaderItemSwap);
		lua_tinker::def(L,"toStEventHeaderItemSwap",&sobj_cast<EventHeaderItemSwap>); // backward-compat alias


		lua_tinker::class_add<EventCBSelChange>(L,"EventCBSelChange");
		lua_tinker::class_inh<EventCBSelChange,IEvtArgs>(L);
		lua_tinker::class_mem<EventCBSelChange>(L,"nCurSel",&EventCBSelChange::nCurSel);
		DEF_TOOBJ(L,EventCBSelChange);
		lua_tinker::def(L,"toStEventCBSelChange",&sobj_cast<EventCBSelChange>); // backward-compat alias

		lua_tinker::class_add<EventCBDropdown>(L,"EventCBDropdown");
		lua_tinker::class_inh<EventCBDropdown,IEvtArgs>(L);
		lua_tinker::class_mem<EventCBDropdown>(L,"pDropDown",&EventCBDropdown::pDropDown);
		DEF_TOOBJ(L,EventCBDropdown);
		lua_tinker::def(L,"toStEventCBDropdown",&sobj_cast<EventCBDropdown>); // backward-compat alias

		lua_tinker::class_add<EventCBBeforeCloseUp>(L,"EventCBBeforeCloseUp");
		lua_tinker::class_inh<EventCBBeforeCloseUp,IEvtArgs>(L);
		lua_tinker::class_mem<EventCBBeforeCloseUp>(L,"bCloseBlock",&EventCBBeforeCloseUp::bCloseBlock);
		DEF_TOOBJ(L,EventCBBeforeCloseUp);
		lua_tinker::def(L,"toStEventCBBeforeCloseUp",&sobj_cast<EventCBBeforeCloseUp>); // backward-compat alias

		lua_tinker::class_add<EventLCSelChanging>(L,"EventLCSelChanging");
		lua_tinker::class_inh<EventLCSelChanging,IEvtArgs>(L);
		lua_tinker::class_mem<EventLCSelChanging>(L,"nNewSel",&EventLCSelChanging::nNewSel);
		lua_tinker::class_mem<EventLCSelChanging>(L,"nOldSel",&EventLCSelChanging::nOldSel);
		lua_tinker::class_mem<EventLCSelChanging>(L,"bCancel",&EventLCSelChanging::bCancel);
		DEF_TOOBJ(L,EventLCSelChanging);
		lua_tinker::def(L,"toStEventLCSelChanging",&sobj_cast<EventLCSelChanging>); // backward-compat alias

		lua_tinker::class_add<EventLCSelChanged>(L,"EventLCSelChanged");
		lua_tinker::class_inh<EventLCSelChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventLCSelChanged>(L,"nNewSel",&EventLCSelChanged::nNewSel);
		lua_tinker::class_mem<EventLCSelChanged>(L,"nOldSel",&EventLCSelChanged::nOldSel);
		DEF_TOOBJ(L,EventLCSelChanged);
		lua_tinker::def(L,"toStEventLCSelChanged",&sobj_cast<EventLCSelChanged>); // backward-compat alias

		lua_tinker::class_add<EventLCDbClick>(L,"EventLCDbClick");
		lua_tinker::class_inh<EventLCDbClick,IEvtArgs>(L);
		lua_tinker::class_mem<EventLCDbClick>(L,"nCurSel",&EventLCDbClick::nCurSel);
		DEF_TOOBJ(L,EventLCDbClick);
		lua_tinker::def(L,"toStEventLCDbClick",&sobj_cast<EventLCDbClick>); // backward-compat alias

		lua_tinker::class_add<EventLCItemDeleted>(L,"EventLCItemDeleted");
		lua_tinker::class_inh<EventLCItemDeleted,IEvtArgs>(L);
		lua_tinker::class_mem<EventLCItemDeleted>(L,"nItem",&EventLCItemDeleted::nItem);
		lua_tinker::class_mem<EventLCItemDeleted>(L,"dwData",&EventLCItemDeleted::dwData);
		DEF_TOOBJ(L,EventLCItemDeleted);
		lua_tinker::def(L,"toStEventLCItemDeleted",&sobj_cast<EventLCItemDeleted>); // backward-compat alias

		lua_tinker::class_add<EventCalendarSelDay>(L,"EventCalendarSelDay");
		lua_tinker::class_inh<EventCalendarSelDay,IEvtArgs>(L);
		lua_tinker::class_mem<EventCalendarSelDay>(L,"wOldDay",&EventCalendarSelDay::wOldDay);
		lua_tinker::class_mem<EventCalendarSelDay>(L,"wNewDay",&EventCalendarSelDay::wNewDay);
		DEF_TOOBJ(L,EventCalendarSelDay);
		lua_tinker::def(L,"toStEventCalendarSelDay",&sobj_cast<EventCalendarSelDay>); // backward-compat alias

		lua_tinker::class_add<EventCalendarExChanged>(L,"EventCalendarExChanged");
		lua_tinker::class_inh<EventCalendarExChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventCalendarExChanged>(L,"iNewDay",&EventCalendarExChanged::iNewDay);
		lua_tinker::class_mem<EventCalendarExChanged>(L,"iNewMonth",&EventCalendarExChanged::iNewMonth);
		lua_tinker::class_mem<EventCalendarExChanged>(L,"iNewYear",&EventCalendarExChanged::iNewYear);
		lua_tinker::class_mem<EventCalendarExChanged>(L,"nBtnType",&EventCalendarExChanged::nBtnType);
		DEF_TOOBJ(L,EventCalendarExChanged);
		lua_tinker::def(L,"toStEventCalendarExChanged",&sobj_cast<EventCalendarExChanged>); // backward-compat alias

		lua_tinker::class_add<EventDateTimeChanged>(L,"EventDateTimeChanged");
		lua_tinker::class_inh<EventDateTimeChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventDateTimeChanged>(L,"newTime",&EventDateTimeChanged::newTime);
		DEF_TOOBJ(L,EventDateTimeChanged);
		lua_tinker::def(L,"toStEventDateTimeChanged",&sobj_cast<EventDateTimeChanged>); // backward-compat alias

		lua_tinker::class_add<EventTCSelChanging>(L,"EventTCSelChanging");
		lua_tinker::class_inh<EventTCSelChanging,IEvtArgs>(L);
		lua_tinker::class_mem<EventTCSelChanging>(L,"hOldSel",&EventTCSelChanging::hOldSel);
		lua_tinker::class_mem<EventTCSelChanging>(L,"hNewSel",&EventTCSelChanging::hNewSel);
		lua_tinker::class_mem<EventTCSelChanging>(L,"bCancel",&EventTCSelChanging::bCancel);
		DEF_TOOBJ(L,EventTCSelChanging);
		lua_tinker::def(L,"toStEventTCSelChanging",&sobj_cast<EventTCSelChanging>); // backward-compat alias


		lua_tinker::class_add<EventTCSelChanged>(L,"EventTCSelChanged");
		lua_tinker::class_inh<EventTCSelChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventTCSelChanged>(L,"hOldSel",&EventTCSelChanged::hOldSel);
		lua_tinker::class_mem<EventTCSelChanged>(L,"hNewSel",&EventTCSelChanged::hNewSel);
		DEF_TOOBJ(L,EventTCSelChanged);
		lua_tinker::def(L,"toStEventTCSelChanged",&sobj_cast<EventTCSelChanged>); // backward-compat alias

		lua_tinker::class_add<EventTCCheckState>(L,"EventTCCheckState");
		lua_tinker::class_inh<EventTCCheckState,IEvtArgs>(L);
		lua_tinker::class_mem<EventTCCheckState>(L,"hItem",&EventTCCheckState::hItem);
		lua_tinker::class_mem<EventTCCheckState>(L,"uCheckState",&EventTCCheckState::uCheckState);
		DEF_TOOBJ(L,EventTCCheckState);
		lua_tinker::def(L,"toStEventTCCheckState",&sobj_cast<EventTCCheckState>); // backward-compat alias

		lua_tinker::class_add<EventTCExpand>(L,"EventTCExpand");
		lua_tinker::class_inh<EventTCExpand,IEvtArgs>(L);
		lua_tinker::class_mem<EventTCExpand>(L,"hItem",&EventTCExpand::hItem);
		lua_tinker::class_mem<EventTCExpand>(L,"bCollapsed",&EventTCExpand::bCollapsed);
		DEF_TOOBJ(L,EventTCExpand);
		lua_tinker::def(L,"toStEventTCExpand",&sobj_cast<EventTCExpand>); // backward-compat alias

		lua_tinker::class_add<EventTCDbClick>(L,"EventTCDbClick");
		lua_tinker::class_inh<EventTCDbClick,IEvtArgs>(L);
		lua_tinker::class_mem<EventTCDbClick>(L,"hItem",&EventTCDbClick::hItem);
		lua_tinker::class_mem<EventTCDbClick>(L,"bCancel",&EventTCDbClick::bCancel);
		DEF_TOOBJ(L,EventTCDbClick);
		lua_tinker::def(L,"toStEventTCDbClick",&sobj_cast<EventTCDbClick>); // backward-compat alias

		lua_tinker::class_add<EventSplitPaneMoved>(L,"EventSplitPaneMoved");
		lua_tinker::class_inh<EventSplitPaneMoved,IEvtArgs>(L);
		lua_tinker::class_mem<EventSplitPaneMoved>(L,"rcPane",&EventSplitPaneMoved::rcPane);
		DEF_TOOBJ(L,EventSplitPaneMoved);
		lua_tinker::def(L,"toStEventSplitPaneMoved",&sobj_cast<EventSplitPaneMoved>); // backward-compat alias

		lua_tinker::class_add<EventSelectMenu>(L,"EventSelectMenu");
		lua_tinker::class_inh<EventSelectMenu,IEvtArgs>(L);
		lua_tinker::class_mem<EventSelectMenu>(L,"nMenuId",&EventSelectMenu::nMenuId);
		lua_tinker::class_mem<EventSelectMenu>(L,"pMenu",&EventSelectMenu::pMenu);
		DEF_TOOBJ(L,EventSelectMenu);
		lua_tinker::def(L,"toStEventSelectMenu",&sobj_cast<EventSelectMenu>); // backward-compat alias

		lua_tinker::class_add<EventPopMenu>(L,"EventPopMenu");
		lua_tinker::class_inh<EventPopMenu,IEvtArgs>(L);
		lua_tinker::class_mem<EventPopMenu>(L,"nMenuIndex",&EventPopMenu::nMenuIndex);
		lua_tinker::class_mem<EventPopMenu>(L,"pMenu",&EventPopMenu::pMenu);
		DEF_TOOBJ(L,EventPopMenu);
		lua_tinker::def(L,"toStEventPopMenu",&sobj_cast<EventPopMenu>); // backward-compat alias

		lua_tinker::class_add<EventSetHotKey>(L,"EventSetHotKey");
		lua_tinker::class_inh<EventSetHotKey,IEvtArgs>(L);
		lua_tinker::class_mem<EventSetHotKey>(L,"vKey",&EventSetHotKey::vKey);
		lua_tinker::class_mem<EventSetHotKey>(L,"wModifiers",&EventSetHotKey::wModifiers);
		DEF_TOOBJ(L,EventSetHotKey);
		lua_tinker::def(L,"toStEventSetHotKey",&sobj_cast<EventSetHotKey>); // backward-compat alias

		// events newly added (parity with soui4js)
		lua_tinker::class_add<EventSwndCaptureChanged>(L,"EventSwndCaptureChanged");
		lua_tinker::class_inh<EventSwndCaptureChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventSwndCaptureChanged>(L,"bCaptured",&StEventSwndCaptureChanged::bCaptured);
		DEF_TOOBJ(L,EventSwndCaptureChanged);
		lua_tinker::def(L,"toStEventSwndCaptureChanged",&sobj_cast<EventSwndCaptureChanged>); // backward-compat alias

		lua_tinker::class_add<EventSwndVisibleChanged>(L,"EventSwndVisibleChanged");
		lua_tinker::class_inh<EventSwndVisibleChanged,IEvtArgs>(L);
		lua_tinker::class_mem<EventSwndVisibleChanged>(L,"bVisible",&StEventSwndVisibleChanged::bVisible);
		DEF_TOOBJ(L,EventSwndVisibleChanged);
		lua_tinker::def(L,"toStEventSwndVisibleChanged",&sobj_cast<EventSwndVisibleChanged>); // backward-compat alias

		lua_tinker::class_add<EventMenuCmd>(L,"EventMenuCmd");
		lua_tinker::class_inh<EventMenuCmd,IEvtArgs>(L);
		lua_tinker::class_mem<EventMenuCmd>(L,"menuId",&StEventMenuCmd::menuId);
		DEF_TOOBJ(L,EventMenuCmd);
		lua_tinker::def(L,"toStEventMenuCmd",&sobj_cast<EventMenuCmd>); // backward-compat alias

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}