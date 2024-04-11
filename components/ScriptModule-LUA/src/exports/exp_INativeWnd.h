#include <interface/SNativeWnd-i.h>


BOOL ExpLua_INativeWnd(lua_State *L)
{
	try
	{
		lua_tinker::class_add<INativeWnd>(L,"INativeWnd");
		lua_tinker::class_inh<INativeWnd,IObjRef>(L);

		lua_tinker::class_def<INativeWnd>(L,"CreateNative",&INativeWnd::CreateNative);
		lua_tinker::class_def<INativeWnd>(L,"GetHwnd",&INativeWnd::GetHwnd);
		lua_tinker::class_def<INativeWnd>(L,"SubclassWindow",&INativeWnd::SubclassWindow);
		lua_tinker::class_def<INativeWnd>(L,"UnsubclassWindow",&INativeWnd::UnsubclassWindow);
		lua_tinker::class_def<INativeWnd>(L,"GetCurrentMessage",&INativeWnd::GetCurrentMessage);
		lua_tinker::class_def<INativeWnd>(L,"GetDlgCtrlID",&INativeWnd::GetDlgCtrlID);
		lua_tinker::class_def<INativeWnd>(L,"GetStyle",&INativeWnd::GetStyle);
		lua_tinker::class_def<INativeWnd>(L,"GetExStyle",&INativeWnd::GetExStyle);
		lua_tinker::class_def<INativeWnd>(L,"GetWindowLongPtr",&INativeWnd::GetWindowLongPtr);
		lua_tinker::class_def<INativeWnd>(L,"SetWindowLongPtr",&INativeWnd::SetWindowLongPtr);

		lua_tinker::class_def<INativeWnd>(L,"GetParent",&INativeWnd::GetParent);
		lua_tinker::class_def<INativeWnd>(L,"SetParent",&INativeWnd::SetParent);
		lua_tinker::class_def<INativeWnd>(L,"IsWindowEnabled",&INativeWnd::IsWindowEnabled);
		lua_tinker::class_def<INativeWnd>(L,"ModifyStyle",&INativeWnd::ModifyStyle);
		lua_tinker::class_def<INativeWnd>(L,"ModifyStyleEx",&INativeWnd::ModifyStyleEx);
		lua_tinker::class_def<INativeWnd>(L,"SetWindowPos",&INativeWnd::SetWindowPos);
		lua_tinker::class_def<INativeWnd>(L,"CenterWindow",&INativeWnd::CenterWindow);
		lua_tinker::class_def<INativeWnd>(L,"DestroyWindow",&INativeWnd::DestroyWindow);
		lua_tinker::class_def<INativeWnd>(L,"IsWindow",&INativeWnd::IsWindow);
		lua_tinker::class_def<INativeWnd>(L,"Invalidate",&INativeWnd::Invalidate);
		lua_tinker::class_def<INativeWnd>(L,"InvalidateRect",&INativeWnd::InvalidateRect);
		lua_tinker::class_def<INativeWnd>(L,"GetWindowRect",&INativeWnd::GetWindowRect);
		lua_tinker::class_def<INativeWnd>(L,"GetClientRect",&INativeWnd::GetClientRect);
		lua_tinker::class_def<INativeWnd>(L,"ClientToScreen",&INativeWnd::ClientToScreen);
		lua_tinker::class_def<INativeWnd>(L,"ClientToScreen2",&INativeWnd::ClientToScreen2);
		lua_tinker::class_def<INativeWnd>(L,"ScreenToClient",&INativeWnd::ScreenToClient);
		lua_tinker::class_def<INativeWnd>(L,"ScreenToClient2",&INativeWnd::ScreenToClient2);
		lua_tinker::class_def<INativeWnd>(L,"MapWindowPoints",&INativeWnd::MapWindowPoints);
		lua_tinker::class_def<INativeWnd>(L,"MapWindowRect",&INativeWnd::MapWindowRect);
		lua_tinker::class_def<INativeWnd>(L,"SetTimer",&INativeWnd::SetTimer);
		lua_tinker::class_def<INativeWnd>(L,"KillTimer",&INativeWnd::KillTimer);

		lua_tinker::class_def<INativeWnd>(L,"GetDC",&INativeWnd::GetDC);
		lua_tinker::class_def<INativeWnd>(L,"GetWindowDC",&INativeWnd::GetWindowDC);
		lua_tinker::class_def<INativeWnd>(L,"ReleaseDC",&INativeWnd::ReleaseDC);
		lua_tinker::class_def<INativeWnd>(L,"CreateCaret",&INativeWnd::CreateCaret);
		lua_tinker::class_def<INativeWnd>(L,"HideCaret",&INativeWnd::HideCaret);
		lua_tinker::class_def<INativeWnd>(L,"ShowCaret",&INativeWnd::ShowCaret);
		lua_tinker::class_def<INativeWnd>(L,"GetCapture",&INativeWnd::GetCapture);
		lua_tinker::class_def<INativeWnd>(L,"SetCapture",&INativeWnd::SetCapture);
		lua_tinker::class_def<INativeWnd>(L,"ReleaseCapture",&INativeWnd::ReleaseCapture);
		lua_tinker::class_def<INativeWnd>(L,"SetFocus",&INativeWnd::SetFocus);
		lua_tinker::class_def<INativeWnd>(L,"SendMessage",&INativeWnd::SendMessage);
		lua_tinker::class_def<INativeWnd>(L,"PostMessage",&INativeWnd::PostMessage);
		lua_tinker::class_def<INativeWnd>(L,"SetWindowText",&INativeWnd::SetWindowText);
		lua_tinker::class_def<INativeWnd>(L,"GetWindowText",&INativeWnd::GetWindowText);
		lua_tinker::class_def<INativeWnd>(L,"IsIconic",&INativeWnd::IsIconic);

		lua_tinker::class_def<INativeWnd>(L,"IsZoomed",&INativeWnd::IsZoomed);
		lua_tinker::class_def<INativeWnd>(L,"IsWindowVisible",&INativeWnd::IsWindowVisible);
		lua_tinker::class_def<INativeWnd>(L,"MoveWindow",&INativeWnd::MoveWindow);
		lua_tinker::class_def<INativeWnd>(L,"MoveWindow2",&INativeWnd::MoveWindow2);
		lua_tinker::class_def<INativeWnd>(L,"ShowWindow",&INativeWnd::ShowWindow);
		lua_tinker::class_def<INativeWnd>(L,"SetWindowRgn",&INativeWnd::SetWindowRgn);
		lua_tinker::class_def<INativeWnd>(L,"SetLayeredWindowAttributes",&INativeWnd::SetLayeredWindowAttributes);
		lua_tinker::class_def<INativeWnd>(L,"UpdateLayeredWindow",&INativeWnd::UpdateLayeredWindow);
		lua_tinker::class_def<INativeWnd>(L,"SetMsgHandler",&INativeWnd::SetMsgHandler);
		lua_tinker::class_def<INativeWnd>(L,"GetMsgHandler",&INativeWnd::GetMsgHandler);
		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}