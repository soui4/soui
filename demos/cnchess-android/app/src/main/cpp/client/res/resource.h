//stamp:129c88c0792cf0ff
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
			const TCHAR * XML_LOGIN;
			const TCHAR * page_lobby;
			const TCHAR * page_game;
			const TCHAR * page_about;
			const TCHAR * dlg_peace_req;
			const TCHAR * dlg_peace_ack;
			}LAYOUT;
		struct _values{
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
			}values;
		struct _ICON{
			const TCHAR * ICON_LOGO;
			}ICON;
		struct _image{
			const TCHAR * PNG_TABLE;
			const TCHAR * PNG_ITEM_BK;
			const TCHAR * PNG_HANDUP;
			const TCHAR * PNG_PLAYING;
			const TCHAR * PNG_MUTE;
			const TCHAR * PNG_UNMUTE;
			}image;
	};
#endif//_UIRES_H_
#ifdef INIT_R_DATA
struct _UIRES UIRES={
		{
			_T("UIDEF:XML_INIT"),
		},
		{
			_T("LAYOUT:XML_MAINWND"),
			_T("LAYOUT:XML_LOGIN"),
			_T("LAYOUT:page_lobby"),
			_T("LAYOUT:page_game"),
			_T("LAYOUT:page_about"),
			_T("LAYOUT:dlg_peace_req"),
			_T("LAYOUT:dlg_peace_ack"),
		},
		{
			_T("values:string"),
			_T("values:color"),
			_T("values:skin"),
		},
		{
			_T("ICON:ICON_LOGO"),
		},
		{
			_T("image:PNG_TABLE"),
			_T("image:PNG_ITEM_BK"),
			_T("image:PNG_HANDUP"),
			_T("image:PNG_PLAYING"),
			_T("image:PNG_MUTE"),
			_T("image:PNG_UNMUTE"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * btn_close;
		 const wchar_t * btn_login;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_mute;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_svr_local;
		 const wchar_t * btn_test;
		 const wchar_t * btn_unmute;
		 const wchar_t * cbx_sex;
		 const wchar_t * chessboard;
		 const wchar_t * edit_desc;
		 const wchar_t * edt_name;
		 const wchar_t * edt_svr;
		 const wchar_t * game_container;
		 const wchar_t * img_playing;
		 const wchar_t * room_container;
		 const wchar_t * seat_0;
		 const wchar_t * seat_1;
		 const wchar_t * seat_ready;
		 const wchar_t * tileview_lobby;
		 const wchar_t * txt_anchor_detail;
		 const wchar_t * txt_anchor_layout;
		 const wchar_t * txt_contact;
		 const wchar_t * txt_copyright;
		 const wchar_t * txt_desc;
		 const wchar_t * txt_info;
		 const wchar_t * txt_new_features;
		 const wchar_t * txt_path_animation;
		 const wchar_t * txt_path_detail;
		 const wchar_t * txt_prop_detail;
		 const wchar_t * txt_property_animation;
		 const wchar_t * txt_qq_group;
		 const wchar_t * txt_table_id;
		 const wchar_t * txt_table_info;
		 const wchar_t * txt_thanks;
		 const wchar_t * txt_title;
		 const wchar_t * txt_website;
	}name;
	struct _id{
		int btn_close;
		int btn_login;
		int btn_max;
		int btn_min;
		int btn_mute;
		int btn_restore;
		int btn_svr_local;
		int btn_test;
		int btn_unmute;
		int cbx_sex;
		int chessboard;
		int edit_desc;
		int edt_name;
		int edt_svr;
		int game_container;
		int img_playing;
		int room_container;
		int seat_0;
		int seat_1;
		int seat_ready;
		int tileview_lobby;
		int txt_anchor_detail;
		int txt_anchor_layout;
		int txt_contact;
		int txt_copyright;
		int txt_desc;
		int txt_info;
		int txt_new_features;
		int txt_path_animation;
		int txt_path_detail;
		int txt_prop_detail;
		int txt_property_animation;
		int txt_qq_group;
		int txt_table_id;
		int txt_table_info;
		int txt_thanks;
		int txt_title;
		int txt_website;
	}id;
	struct _color{
		int black;
		int blue;
		int gray;
		int green;
		int mask_select;
		int red;
		int white;
	}color;
	struct _string{
		int about;
		int cnchess;
		int lobby;
		int login;
		int peace_ack;
		int peace_req;
		int regret;
		int surrender;
		int title;
		int ver;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"btn_close",
		L"btn_login",
		L"btn_max",
		L"btn_min",
		L"btn_mute",
		L"btn_restore",
		L"btn_svr_local",
		L"btn_test",
		L"btn_unmute",
		L"cbx_sex",
		L"chessboard",
		L"edit_desc",
		L"edt_name",
		L"edt_svr",
		L"game_container",
		L"img_playing",
		L"room_container",
		L"seat_0",
		L"seat_1",
		L"seat_ready",
		L"tileview_lobby",
		L"txt_anchor_detail",
		L"txt_anchor_layout",
		L"txt_contact",
		L"txt_copyright",
		L"txt_desc",
		L"txt_info",
		L"txt_new_features",
		L"txt_path_animation",
		L"txt_path_detail",
		L"txt_prop_detail",
		L"txt_property_animation",
		L"txt_qq_group",
		L"txt_table_id",
		L"txt_table_info",
		L"txt_thanks",
		L"txt_title",
		L"txt_website"
	}
	,
	{
		65542,
		65547,
		65540,
		65539,
		65537,
		65541,
		65544,
		65556,
		65538,
		65546,
		65555,
		65571,
		65545,
		65543,
		65554,
		65551,
		65548,
		10,
		11,
		65550,
		65549,
		65560,
		65559,
		65565,
		65570,
		65557,
		65568,
		65558,
		65563,
		65564,
		65562,
		65561,
		65566,
		65552,
		65553,
		65569,
		65536,
		65567
	}
	,
	{
		0,
		1,
		2,
		3,
		4,
		5,
		6
	}
	,
	{
		0,
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		9
	}
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
