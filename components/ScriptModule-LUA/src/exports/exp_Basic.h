
BOOL ExpLua_Basic(lua_State *L)
{
	try{

		//POINT
		lua_tinker::class_add<POINT>(L,"POINT");
		lua_tinker::class_mem<POINT>(L, "x", &POINT::x);
		lua_tinker::class_mem<POINT>(L, "y", &POINT::y);
		//RECT
		lua_tinker::class_add<RECT>(L,"RECT");
		lua_tinker::class_mem<RECT>(L, "left", &RECT::left);
		lua_tinker::class_mem<RECT>(L, "top", &RECT::top);
		lua_tinker::class_mem<RECT>(L, "right", &RECT::right);
		lua_tinker::class_mem<RECT>(L, "bottom", &RECT::bottom);
		//SIZE
		lua_tinker::class_add<SIZE>(L,"SIZE");
		lua_tinker::class_mem<SIZE>(L, "cx", &SIZE::cx);
		lua_tinker::class_mem<SIZE>(L, "cy", &SIZE::cy);

		//CPoint
		lua_tinker::class_add<CPoint>(L,"CPoint");
		lua_tinker::class_inh<CPoint,POINT>(L);
		lua_tinker::class_con<CPoint>(L,lua_tinker::constructor<CPoint,LONG,LONG>);
		//CRect
		lua_tinker::class_add<CRect>(L,"CRect");
		lua_tinker::class_inh<CRect,RECT>(L);
		lua_tinker::class_con<CRect>(L,lua_tinker::constructor<CRect,LONG,LONG,LONG,LONG>);
		lua_tinker::class_def<CRect>(L,"Width",&CRect::Width);
		lua_tinker::class_def<CRect>(L,"Height",&CRect::Height);
		lua_tinker::class_def<CRect>(L,"Size",&CRect::Size);
		lua_tinker::class_def<CRect>(L,"IsRectEmpty",&CRect::IsRectEmpty);
		lua_tinker::class_def<CRect>(L,"IsRectNull",&CRect::IsRectNull);
		lua_tinker::class_def<CRect>(L,"PtInRect",&CRect::PtInRect);
		lua_tinker::class_def<CRect>(L,"SetRectEmpty",&CRect::SetRectEmpty);
        lua_tinker::class_def<CRect>(L,"OffsetRect",(void (CRect::*)(int,int))&CRect::OffsetRect);

		lua_tinker::class_def<CRect>(L,"SetRect",(void (CRect::*)(int,int,int,int))&CRect::SetRect);
		lua_tinker::class_def<CRect>(L,"CenterPoint",&CRect::CenterPoint);
		lua_tinker::class_def<CRect>(L,"CopyRect",(void (CRect::*)(LPCRECT))&CRect::CopyRect);
		lua_tinker::class_def<CRect>(L,"EqualRect",(BOOL (CRect::*)(LPCRECT) const)&CRect::EqualRect);
		lua_tinker::class_def<CRect>(L,"InflateRect",(void (CRect::*)(int,int))&CRect::InflateRect);
		lua_tinker::class_def<CRect>(L,"InflateRect2",(void (CRect::*)(int,int,int,int))&CRect::InflateRect);
		lua_tinker::class_def<CRect>(L,"DeflateRect",(void (CRect::*)(int,int))&CRect::DeflateRect);
		lua_tinker::class_def<CRect>(L,"DeflateRect2",(void (CRect::*)(int,int,int,int))&CRect::DeflateRect);
		lua_tinker::class_def<CRect>(L,"NormalizeRect",&CRect::NormalizeRect);
		lua_tinker::class_def<CRect>(L,"IntersectRect",(BOOL (CRect::*)(LPCRECT,LPCRECT))&CRect::IntersectRect);
		lua_tinker::class_def<CRect>(L,"UnionRect",(BOOL (CRect::*)(LPCRECT,LPCRECT))&CRect::UnionRect);
		lua_tinker::class_def<CRect>(L,"SubtractRect",(BOOL (CRect::*)(const RECT *,const RECT *))&CRect::SubtractRect);
		lua_tinker::class_def<CRect>(L,"MoveToX",&CRect::MoveToX);
		lua_tinker::class_def<CRect>(L,"MoveToY",&CRect::MoveToY);
		lua_tinker::class_def<CRect>(L,"MoveToXY",(void (CRect::*)(int,int))&CRect::MoveToXY);


		//CSize
		lua_tinker::class_add<CSize>(L,"CSize");
		lua_tinker::class_inh<CSize,SIZE>(L);
		lua_tinker::class_con<CSize>(L,lua_tinker::constructor<CSize,LONG,LONG>);

		//MSG
		lua_tinker::class_add<MSG>(L,"MSG");
		lua_tinker::class_mem<MSG>(L, "hwnd", &MSG::hwnd);
		lua_tinker::class_mem<MSG>(L, "message", &MSG::message);
		lua_tinker::class_mem<MSG>(L, "wParam", &MSG::wParam);
		lua_tinker::class_mem<MSG>(L, "lParam", &MSG::lParam);
		lua_tinker::class_mem<MSG>(L, "time", &MSG::time);
		lua_tinker::class_mem<MSG>(L, "pt", &MSG::pt);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}

}