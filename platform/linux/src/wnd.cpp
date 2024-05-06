#include <wnd.h>

HWND WIN_CreateWindowEx( CREATESTRUCT *cs, LPCSTR className, HINSTANCE module);

/***********************************************************************
 *		CreateWindowExW (USER32.@)
 */
HWND WINAPI  CreateWindowEx( DWORD exStyle, LPCSTR className,
                                 LPCSTR windowName, DWORD style, INT x,
                                 INT y, INT width, INT height,
                                 HWND parent, HMENU menu,
                                 HINSTANCE instance, LPVOID data )
{
    CREATESTRUCT cs;
    cs.lpCreateParams = data;
    cs.hInstance      = instance;
    cs.hMenu          = menu;
    cs.hwndParent     = parent;
    cs.x              = x;
    cs.y              = y;
    cs.cx             = width;
    cs.cy             = height;
    cs.style          = style;
    cs.lpszName       = windowName;
    cs.lpszClass      = className;
    cs.dwExStyle      = exStyle;

    return WIN_CreateWindowEx( &cs, className, instance);
}

/***********************************************************************
 *           WIN_CreateWindowEx
 *
 * Implementation of CreateWindowEx().
 */
HWND WIN_CreateWindowEx( CREATESTRUCT *cs, LPCSTR className, HINSTANCE module)
{
    UNICODE_STRING _class, window_name = {0};
    HWND hwnd, top_child = 0;
    WNDCLASSEX info;
    WCHAR name_buf[8];
    HMENU menu;

    if (!get_class_info( module, className, &info, &_class, FALSE )) return FALSE;

    /* Fix the styles for MDI children */
    if (cs->dwExStyle & WS_EX_MDICHILD)
    {
        POINT pos[2];
        UINT id = 0;

        if (!NtUserGetMDIClientInfo( cs->hwndParent ))
        {
            WARN("WS_EX_MDICHILD, but parent %p is not MDIClient\n", cs->hwndParent);
            return 0;
        }

        /* cs->lpCreateParams of WM_[NC]CREATE is different for MDI children.
         * MDICREATESTRUCT members have the originally passed values.
         *
         * Note: we rely on the fact that MDICREATESTRUCTA and MDICREATESTRUCTW
         * have the same layout.
         */
        mdi_cs.szClass = cs->lpszClass;
        mdi_cs.szTitle = cs->lpszName;
        mdi_cs.hOwner = cs->hInstance;
        mdi_cs.x = cs->x;
        mdi_cs.y = cs->y;
        mdi_cs.cx = cs->cx;
        mdi_cs.cy = cs->cy;
        mdi_cs.style = cs->style;
        mdi_cs.lParam = (LPARAM)cs->lpCreateParams;

        cs->lpCreateParams = &mdi_cs;

        if (GetWindowLongW(cs->hwndParent, GWL_STYLE) & MDIS_ALLCHILDSTYLES)
        {
            if (cs->style & WS_POPUP)
            {
                TRACE("WS_POPUP with MDIS_ALLCHILDSTYLES is not allowed\n");
                return 0;
            }
            cs->style |= WS_CHILD | WS_CLIPSIBLINGS;
        }
        else
        {
            cs->style &= ~WS_POPUP;
            cs->style |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CAPTION |
                WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        }

        top_child = GetWindow(cs->hwndParent, GW_CHILD);

        if (top_child)
        {
            /* Restore current maximized child */
            if((cs->style & WS_VISIBLE) && IsZoomed(top_child))
            {
                TRACE("Restoring current maximized child %p\n", top_child);
                if (cs->style & WS_MAXIMIZE)
                {
                    /* if the new window is maximized don't bother repainting */
                    SendMessageW( top_child, WM_SETREDRAW, FALSE, 0 );
                    NtUserShowWindow( top_child, SW_SHOWNORMAL );
                    SendMessageW( top_child, WM_SETREDRAW, TRUE, 0 );
                }
                else NtUserShowWindow( top_child, SW_SHOWNORMAL );
            }
        }

        MDI_CalcDefaultChildPos( cs->hwndParent, -1, pos, 0, &id );
        if (!(cs->style & WS_POPUP)) cs->hMenu = ULongToHandle(id);

        TRACE( "MDI child id %04x\n", id );

        if (cs->style & (WS_CHILD | WS_POPUP))
        {
            if (is_default_coord( cs->x ))
            {
                cs->x = pos[0].x;
                cs->y = pos[0].y;
            }
            if (is_default_coord( cs->cx ) || !cs->cx) cs->cx = pos[1].x;
            if (is_default_coord( cs->cy ) || !cs->cy) cs->cy = pos[1].y;
        }
    }

    if (!unicode && cs->lpszName)
    {
        const char *nameA = (const char *)cs->lpszName;
        /* resource ID string is a special case */
        if (nameA[0] == '\xff')
        {
            name_buf[0] = 0xffff;
            name_buf[1] = MAKEWORD( nameA[1], nameA[2] );
            name_buf[2] = 0;
            RtlInitUnicodeString( &window_name, name_buf );
        }
        else if (!RtlCreateUnicodeStringFromAsciiz( &window_name, (const char *)cs->lpszName ))
            return 0;
    }
    else RtlInitUnicodeString( &window_name, cs->lpszName );

    menu = cs->hMenu;
    if (!menu && info.lpszMenuName && (cs->style & (WS_CHILD | WS_POPUP)) != WS_CHILD)
        menu = LoadMenuW( cs->hInstance, info.lpszMenuName );

    hwnd = NtUserCreateWindowEx( cs->dwExStyle, &class, NULL, &window_name, cs->style,
                                 cs->x, cs->y, cs->cx, cs->cy, cs->hwndParent, menu, module,
                                 cs->lpCreateParams, 0, cs->hInstance, 0, !unicode );
    if (!hwnd && menu && menu != cs->hMenu) NtUserDestroyMenu( menu );
    if (!unicode && window_name.Buffer != name_buf) RtlFreeUnicodeString( &window_name );
    return hwnd;
}
