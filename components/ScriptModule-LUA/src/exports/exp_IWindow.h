#include <interface/SWindow-i.h>
#include "toobj.h"

BOOL ExpLua_IWindow(lua_State *L)
{
	try{
		DEF_TOOBJ(L,IWindow);

		lua_tinker::class_add<IWindow>(L,"IWindow");
		lua_tinker::class_inh<IWindow,IObject>(L);

		lua_tinker::class_def<IWindow>(L,"QueryInterface",&IWindow::QueryInterface);
		lua_tinker::class_def<IWindow>(L,"GetContainer",&IWindow::GetContainer);
		lua_tinker::class_def<IWindow>(L,"SetContainer",&IWindow::SetContainer);
		lua_tinker::class_def<IWindow>(L,"GetSwnd",&IWindow::GetSwnd);
		lua_tinker::class_def<IWindow>(L,"GetLayout",&IWindow::GetLayout);
		lua_tinker::class_def<IWindow>(L,"GetLayoutParam",&IWindow::GetLayoutParam);
		lua_tinker::class_def<IWindow>(L,"SetLayoutParam",&IWindow::SetLayoutParam);
		lua_tinker::class_def<IWindow>(L,"IsFloat",&IWindow::IsFloat);
		lua_tinker::class_def<IWindow>(L,"IsDisplay",&IWindow::IsDisplay);
		lua_tinker::class_def<IWindow>(L,"IsMsgTransparent",&IWindow::IsMsgTransparent);
		lua_tinker::class_def<IWindow>(L,"IsClipClient",&IWindow::IsClipClient);
		lua_tinker::class_def<IWindow>(L,"SetToolTipText",&IWindow::SetToolTipText);
		lua_tinker::class_def<IWindow>(L,"IsChecked",&IWindow::IsChecked);
		lua_tinker::class_def<IWindow>(L,"SetCheck",&IWindow::SetCheck);
		lua_tinker::class_def<IWindow>(L,"IsDisabled",&IWindow::IsDisabled);
		lua_tinker::class_def<IWindow>(L,"EnableWindow",&IWindow::EnableWindow);

		lua_tinker::class_def<IWindow>(L,"IsVisible",&IWindow::IsVisible);
		lua_tinker::class_def<IWindow>(L,"SetVisible",&IWindow::SetVisible);
		lua_tinker::class_def<IWindow>(L,"GetUserData",&IWindow::GetUserData);
		lua_tinker::class_def<IWindow>(L,"SetUserData",&IWindow::SetUserData);
		lua_tinker::class_def<IWindow>(L,"DoColorize",&IWindow::DoColorize);
		lua_tinker::class_def<IWindow>(L,"GetColorizeColor",&IWindow::GetColorizeColor);
		lua_tinker::class_def<IWindow>(L,"SSendMessage",&IWindow::SSendMessage);
		lua_tinker::class_def<IWindow>(L,"SDispatchMessage",&IWindow::SDispatchMessage);
		lua_tinker::class_def<IWindow>(L,"IsFocusable",&IWindow::IsFocusable);
		lua_tinker::class_def<IWindow>(L,"IsFocused",&IWindow::IsFocused);
		lua_tinker::class_def<IWindow>(L,"SetFocus",&IWindow::SetFocus);
		lua_tinker::class_def<IWindow>(L,"KillFocus",&IWindow::KillFocus);
		lua_tinker::class_def<IWindow>(L,"Invalidate",&IWindow::Invalidate);
		lua_tinker::class_def<IWindow>(L,"InvalidateRect",&IWindow::InvalidateRect);

		lua_tinker::class_def<IWindow>(L,"LockUpdate",&IWindow::LockUpdate);
		lua_tinker::class_def<IWindow>(L,"UnlockUpdate",&IWindow::UnlockUpdate);
		lua_tinker::class_def<IWindow>(L,"IsUpdateLocked",&IWindow::IsUpdateLocked);
		lua_tinker::class_def<IWindow>(L,"Update",&IWindow::Update);
		lua_tinker::class_def<IWindow>(L,"SetWindowRgn",&IWindow::SetWindowRgn);
		lua_tinker::class_def<IWindow>(L,"GetWindowRgn",&IWindow::GetWindowRgn);
		lua_tinker::class_def<IWindow>(L,"SetWindowPath",&IWindow::SetWindowPath);
		lua_tinker::class_def<IWindow>(L,"GetWindowPath",&IWindow::GetWindowPath);
		lua_tinker::class_def<IWindow>(L,"Move2",&IWindow::Move2);
		lua_tinker::class_def<IWindow>(L,"Move",&IWindow::Move);
		lua_tinker::class_def<IWindow>(L,"GetWindowRect",&IWindow::GetWindowRect);
		lua_tinker::class_def<IWindow>(L,"GetClientRect",&IWindow::GetClientRect);

		lua_tinker::class_def<IWindow>(L,"IsContainPoint",&IWindow::IsContainPoint);
		lua_tinker::class_def<IWindow>(L,"SwndFromPoint",&IWindow::SwndFromPoint);
		lua_tinker::class_def<IWindow>(L,"SetTimer",&IWindow::SetTimer);
		lua_tinker::class_def<IWindow>(L,"KillTimer",&IWindow::KillTimer);
		lua_tinker::class_def<IWindow>(L,"GetCapture",&IWindow::GetCapture);
		lua_tinker::class_def<IWindow>(L,"SetCapture",&IWindow::SetCapture);
		lua_tinker::class_def<IWindow>(L,"ReleaseCapture",&IWindow::ReleaseCapture);
		lua_tinker::class_def<IWindow>(L,"SetAnimation",&IWindow::SetAnimation);
		lua_tinker::class_def<IWindow>(L,"StartAnimation",&IWindow::StartAnimation);
		lua_tinker::class_def<IWindow>(L,"GetAnimation",&IWindow::GetAnimation);
		lua_tinker::class_def<IWindow>(L,"ClearAnimation",&IWindow::ClearAnimation);

		lua_tinker::class_def<IWindow>(L,"SetAlpha",&IWindow::SetAlpha);
		lua_tinker::class_def<IWindow>(L,"GetAlpha",&IWindow::GetAlpha);
		lua_tinker::class_def<IWindow>(L,"SetMatrix",&IWindow::SetMatrix);
		lua_tinker::class_def<IWindow>(L,"GetMatrix",&IWindow::GetMatrix);
		lua_tinker::class_def<IWindow>(L,"GetScale",&IWindow::GetScale);
		lua_tinker::class_def<IWindow>(L,"RequestRelayout",&IWindow::RequestRelayout);
		lua_tinker::class_def<IWindow>(L,"UpdateLayout",&IWindow::UpdateLayout);
		lua_tinker::class_def<IWindow>(L,"UpdateChildrenPosition",&IWindow::UpdateChildrenPosition);
		lua_tinker::class_def<IWindow>(L,"IsLayoutDirty",&IWindow::IsLayoutDirty);
		lua_tinker::class_def<IWindow>(L,"OnGetDlgCode",&IWindow::OnGetDlgCode);
		lua_tinker::class_def<IWindow>(L,"GetIRoot",&IWindow::GetIRoot);
		lua_tinker::class_def<IWindow>(L,"GetIParent",&IWindow::GetIParent);


		lua_tinker::class_def<IWindow>(L,"GetIWindow",&IWindow::GetIWindow);
		lua_tinker::class_def<IWindow>(L,"GetIChild",&IWindow::GetIChild);
		lua_tinker::class_def<IWindow>(L,"GetChildrenCount",&IWindow::GetChildrenCount);
		lua_tinker::class_def<IWindow>(L,"IsDescendant",&IWindow::IsDescendant);
		lua_tinker::class_def<IWindow>(L,"SetIOwner",&IWindow::SetIOwner);
		lua_tinker::class_def<IWindow>(L,"GetIOwner",&IWindow::GetIOwner);
		lua_tinker::class_def<IWindow>(L,"BringWindowToTop",&IWindow::BringWindowToTop);
		lua_tinker::class_def<IWindow>(L,"AdjustIZOrder",&IWindow::AdjustIZOrder);
		lua_tinker::class_def<IWindow>(L,"InsertIChild",&IWindow::InsertIChild);
		lua_tinker::class_def<IWindow>(L,"RemoveIChild",&IWindow::RemoveIChild);
		lua_tinker::class_def<IWindow>(L,"CreateChildrenFromXml",&IWindow::CreateChildrenFromXml);
		lua_tinker::class_def<IWindow>(L,"FindIChildByID",&IWindow::FindIChildByID);
		lua_tinker::class_def<IWindow>(L,"FindIChildByName",&IWindow::FindIChildByName);
		lua_tinker::class_def<IWindow>(L,"FindIChildByNameA",&IWindow::FindIChildByNameA);

		lua_tinker::class_def<IWindow>(L,"DestroyIChild",&IWindow::DestroyIChild);
		lua_tinker::class_def<IWindow>(L,"DestroyAllChildren",&IWindow::DestroyAllChildren);
		lua_tinker::class_def<IWindow>(L,"Destroy",&IWindow::Destroy);
		lua_tinker::class_def<IWindow>(L,"GetNextLayoutIChild",&IWindow::GetNextLayoutIChild);
		lua_tinker::class_def<IWindow>(L,"GetChildrenLayoutRect",&IWindow::GetChildrenLayoutRect);
		lua_tinker::class_def<IWindow>(L,"GetDesiredSize",&IWindow::GetDesiredSize);
		lua_tinker::class_def<IWindow>(L,"GetBkgndColor",&IWindow::GetBkgndColor);
		lua_tinker::class_def<IWindow>(L,"SetWindowText",&IWindow::SetWindowText);
		lua_tinker::class_def<IWindow>(L,"GetWindowText",&IWindow::GetWindowText);
		lua_tinker::class_def<IWindow>(L,"GetState",&IWindow::GetState);
		lua_tinker::class_def<IWindow>(L,"ModifyState",&IWindow::ModifyState);

		lua_tinker::class_def<IWindow>(L,"GetISelectedSiblingInGroup",&IWindow::GetISelectedSiblingInGroup);
		lua_tinker::class_def<IWindow>(L,"GetISelectedChildInGroup",&IWindow::GetISelectedChildInGroup);
		lua_tinker::class_def<IWindow>(L,"IsSiblingsAutoGroupped",&IWindow::IsSiblingsAutoGroupped);
		lua_tinker::class_def<IWindow>(L,"CreateCaret",&IWindow::CreateCaret);
		lua_tinker::class_def<IWindow>(L,"ShowCaret",&IWindow::ShowCaret);
		lua_tinker::class_def<IWindow>(L,"SetCaretPos",&IWindow::SetCaretPos);
		lua_tinker::class_def<IWindow>(L,"SetEventMute",&IWindow::SetEventMute);
		lua_tinker::class_def<IWindow>(L,"SubscribeEvent",&IWindow::SubscribeEvent);
		lua_tinker::class_def<IWindow>(L,"UnsubscribeEvent",&IWindow::UnsubscribeEvent);
		lua_tinker::class_def<IWindow>(L,"AddEvent",&IWindow::AddEvent);
		lua_tinker::class_def<IWindow>(L,"RemoveEvent",&IWindow::RemoveEvent);
		lua_tinker::class_def<IWindow>(L,"FireEvent",&IWindow::FireEvent);
		lua_tinker::class_def<IWindow>(L,"FireCommand",&IWindow::FireCommand);

		lua_tinker::class_def<IWindow>(L,"FireCtxMenu",&IWindow::FireCtxMenu);
		lua_tinker::class_def<IWindow>(L,"SwndProc",&IWindow::SwndProc);
		lua_tinker::class_def<IWindow>(L,"SetSwndProc",&IWindow::SetSwndProc);
		lua_tinker::class_def<IWindow>(L,"GetHostHwnd",&IWindow::GetHostHwnd);
		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}