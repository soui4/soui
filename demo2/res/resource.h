//stamp:0ceb13a58cafe327
/*<------------------------------------------------------------------------------------------------->*/
/*该文件由uiresbuilder生成，请不要手动修改*/
/*<------------------------------------------------------------------------------------------------->*/
#ifndef _UIRES_H_
#define _UIRES_H_
	struct _UIRES{
		struct _UIDEF{
			const TCHAR * XML_INIT;
			}UIDEF;
		struct _LAYOUT{
			const TCHAR * XML_MAINWND;
			const TCHAR * DLG_TEST;
			}LAYOUT;
		struct _SMENU{
			const TCHAR * menu_test;
			const TCHAR * menuex_test;
			}SMENU;
		struct _values{
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
			}values;
		struct _ImgTrayMenu{
			const TCHAR * circulation;
			const TCHAR * ilike;
			const TCHAR * next;
			const TCHAR * pause;
			const TCHAR * play;
			const TCHAR * pre;
			const TCHAR * volume;
			const TCHAR * menu_ico;
			const TCHAR * png_menu_border;
			}ImgTrayMenu;
		struct _ICON{
			const TCHAR * ICON_LOGO;
			}ICON;
	};
#endif//_UIRES_H_
#ifdef INIT_R_DATA
struct _UIRES UIRES={
		{
			_T("UIDEF:XML_INIT"),
		},
		{
			_T("LAYOUT:XML_MAINWND"),
			_T("LAYOUT:DLG_TEST"),
		},
		{
			_T("SMENU:menu_test"),
			_T("SMENU:menuex_test"),
		},
		{
			_T("values:string"),
			_T("values:color"),
			_T("values:skin"),
		},
		{
			_T("ImgTrayMenu:circulation"),
			_T("ImgTrayMenu:ilike"),
			_T("ImgTrayMenu:next"),
			_T("ImgTrayMenu:pause"),
			_T("ImgTrayMenu:play"),
			_T("ImgTrayMenu:pre"),
			_T("ImgTrayMenu:volume"),
			_T("ImgTrayMenu:menu_ico"),
			_T("ImgTrayMenu:png_menu_border"),
		},
		{
			_T("ICON:ICON_LOGO"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * btn_add;
		 const wchar_t * btn_clean;
		 const wchar_t * btn_close;
		 const wchar_t * btn_dialog;
		 const wchar_t * btn_flash;
		 const wchar_t * btn_menu;
		 const wchar_t * btn_menuex;
		 const wchar_t * lv_test;
		 const wchar_t * menu_btn_cir;
		 const wchar_t * menu_btn_ilike;
		 const wchar_t * menu_btn_next;
		 const wchar_t * menu_btn_play;
		 const wchar_t * menu_btn_pre;
		 const wchar_t * menu_btn_vol;
		 const wchar_t * menu_slider_vol;
		 const wchar_t * testWnd;
		 const wchar_t * tst_slider;
		 const wchar_t * txt_pos;
		 const wchar_t * txt_test;
	}name;
	struct _id{
		int btn_add;
		int btn_clean;
		int btn_close;
		int btn_dialog;
		int btn_flash;
		int btn_menu;
		int btn_menuex;
		int lv_test;
		int menu_btn_cir;
		int menu_btn_ilike;
		int menu_btn_next;
		int menu_btn_play;
		int menu_btn_pre;
		int menu_btn_vol;
		int menu_slider_vol;
		int testWnd;
		int tst_slider;
		int txt_pos;
		int txt_test;
	}id;
	struct _color{
		int blue;
		int gray;
		int green;
		int red;
		int white;
	}color;
	struct _string{
		int dialog;
		int title;
		int ver;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"btn_add",
		L"btn_clean",
		L"btn_close",
		L"btn_dialog",
		L"btn_flash",
		L"btn_menu",
		L"btn_menuex",
		L"lv_test",
		L"menu_btn_cir",
		L"menu_btn_ilike",
		L"menu_btn_next",
		L"menu_btn_play",
		L"menu_btn_pre",
		L"menu_btn_vol",
		L"menu_slider_vol",
		L"testWnd",
		L"tst_slider",
		L"txt_pos",
		L"txt_test"
	}
	,
	{
		65545,
		65546,
		65547,
		65539,
		65536,
		65537,
		65538,
		65543,
		65552,
		65551,
		65550,
		65549,
		65548,
		65553,
		65554,
		65540,
		65541,
		65542,
		65544
	}
	,
	{
		0,
		1,
		2,
		3,
		4
	}
	,
	{
		0,
		1,
		2
	}
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
