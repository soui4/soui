//stamp:147e080ffdfcf259
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
			const TCHAR * dlg_theme_progress;
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
		struct _gif{
			const TCHAR * GIF_AVATAR_CHAIR;
			const TCHAR * GIF_AVATAR_MALE;
			const TCHAR * GIF_AVATAR_FEMALE;
			const TCHAR * GIF_AVATAR_01;
			const TCHAR * GIF_AVATAR_02;
			}gif;
		struct _svg{
			const TCHAR * btn_primary;
			const TCHAR * btn_login;
			const TCHAR * cnchess_logo;
			const TCHAR * edit_border;
			const TCHAR * form_icons;
			const TCHAR * login_bg;
			const TCHAR * lobby_table_bk;
			const TCHAR * chess_table;
			const TCHAR * playing_icon;
			const TCHAR * seat_states;
			const TCHAR * progress_bar;
			const TCHAR * progress_pos;
			const TCHAR * login_panel_bg;
			const TCHAR * login_corner;
			const TCHAR * login_corner_rtl;
			const TCHAR * login_divider;
			const TCHAR * login_btn_login;
			const TCHAR * login_gold_line;
			const TCHAR * login_divider_line;
			const TCHAR * login_deco_dot;
			const TCHAR * bg_piece_ju;
			const TCHAR * bg_piece_ma;
			const TCHAR * bg_piece_pao;
			const TCHAR * bg_piece_xiang;
			const TCHAR * avatar_section_border;
			const TCHAR * avatar_slot;
			const TCHAR * avatar_preview;
			const TCHAR * btn_small;
			const TCHAR * combobox_dropbtn;
			const TCHAR * edit_bg;
			const TCHAR * list_item;
			}svg;
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
			_T("LAYOUT:dlg_theme_progress"),
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
		{
			_T("gif:GIF_AVATAR_CHAIR"),
			_T("gif:GIF_AVATAR_MALE"),
			_T("gif:GIF_AVATAR_FEMALE"),
			_T("gif:GIF_AVATAR_01"),
			_T("gif:GIF_AVATAR_02"),
		},
		{
			_T("svg:btn_primary"),
			_T("svg:btn_login"),
			_T("svg:cnchess_logo"),
			_T("svg:edit_border"),
			_T("svg:form_icons"),
			_T("svg:login_bg"),
			_T("svg:lobby_table_bk"),
			_T("svg:chess_table"),
			_T("svg:playing_icon"),
			_T("svg:seat_states"),
			_T("svg:progress_bar"),
			_T("svg:progress_pos"),
			_T("svg:login_panel_bg"),
			_T("svg:login_corner"),
			_T("svg:login_corner_rtl"),
			_T("svg:login_divider"),
			_T("svg:login_btn_login"),
			_T("svg:login_gold_line"),
			_T("svg:login_divider_line"),
			_T("svg:login_deco_dot"),
			_T("svg:bg_piece_ju"),
			_T("svg:bg_piece_ma"),
			_T("svg:bg_piece_pao"),
			_T("svg:bg_piece_xiang"),
			_T("svg:avatar_section_border"),
			_T("svg:avatar_slot"),
			_T("svg:avatar_preview"),
			_T("svg:btn_small"),
			_T("svg:combobox_dropbtn"),
			_T("svg:edit_bg"),
			_T("svg:list_item"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * btn_avatar_1;
		 const wchar_t * btn_avatar_2;
		 const wchar_t * btn_avatar_3;
		 const wchar_t * btn_avatar_4;
		 const wchar_t * btn_avatar_ext;
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
		 const wchar_t * gp_preview;
		 const wchar_t * img_playing;
		 const wchar_t * progress_bar;
		 const wchar_t * room_container;
		 const wchar_t * seat_0;
		 const wchar_t * seat_1;
		 const wchar_t * tileview_lobby;
		 const wchar_t * txt_anchor_detail;
		 const wchar_t * txt_anchor_layout;
		 const wchar_t * txt_contact;
		 const wchar_t * txt_copyright;
		 const wchar_t * txt_desc;
		 const wchar_t * txt_info;
		 const wchar_t * txt_new_features;
		 const wchar_t * txt_online;
		 const wchar_t * txt_path_animation;
		 const wchar_t * txt_path_detail;
		 const wchar_t * txt_prop_detail;
		 const wchar_t * txt_property_animation;
		 const wchar_t * txt_qq_group;
		 const wchar_t * txt_seat_0;
		 const wchar_t * txt_seat_1;
		 const wchar_t * txt_status;
		 const wchar_t * txt_table_id;
		 const wchar_t * txt_table_info;
		 const wchar_t * txt_thanks;
		 const wchar_t * txt_title;
		 const wchar_t * txt_website;
	}name;
	struct _id{
		int btn_avatar_1;
		int btn_avatar_2;
		int btn_avatar_3;
		int btn_avatar_4;
		int btn_avatar_ext;
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
		int gp_preview;
		int img_playing;
		int progress_bar;
		int room_container;
		int seat_0;
		int seat_1;
		int tileview_lobby;
		int txt_anchor_detail;
		int txt_anchor_layout;
		int txt_contact;
		int txt_copyright;
		int txt_desc;
		int txt_info;
		int txt_new_features;
		int txt_online;
		int txt_path_animation;
		int txt_path_detail;
		int txt_prop_detail;
		int txt_property_animation;
		int txt_qq_group;
		int txt_seat_0;
		int txt_seat_1;
		int txt_status;
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
		int login_avatar_border;
		int login_border;
		int login_gold;
		int login_gold_dark;
		int login_gold_light;
		int login_input_bg;
		int login_panel;
		int login_panel_dark;
		int login_text;
		int login_text_sub;
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
		L"btn_avatar_1",
		L"btn_avatar_2",
		L"btn_avatar_3",
		L"btn_avatar_4",
		L"btn_avatar_ext",
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
		L"gp_preview",
		L"img_playing",
		L"progress_bar",
		L"room_container",
		L"seat_0",
		L"seat_1",
		L"tileview_lobby",
		L"txt_anchor_detail",
		L"txt_anchor_layout",
		L"txt_contact",
		L"txt_copyright",
		L"txt_desc",
		L"txt_info",
		L"txt_new_features",
		L"txt_online",
		L"txt_path_animation",
		L"txt_path_detail",
		L"txt_prop_detail",
		L"txt_property_animation",
		L"txt_qq_group",
		L"txt_seat_0",
		L"txt_seat_1",
		L"txt_status",
		L"txt_table_id",
		L"txt_table_info",
		L"txt_thanks",
		L"txt_title",
		L"txt_website"
	}
	,
	{
		65547,
		65548,
		65549,
		65550,
		65552,
		65542,
		65553,
		65540,
		65539,
		65537,
		65541,
		65544,
		65565,
		65538,
		65546,
		65564,
		65580,
		65545,
		65543,
		65563,
		65551,
		65558,
		65581,
		65554,
		10,
		11,
		65556,
		65569,
		65568,
		65574,
		65579,
		65566,
		65577,
		65567,
		65555,
		65572,
		65573,
		65571,
		65570,
		65575,
		65559,
		65560,
		65561,
		65557,
		65562,
		65578,
		65536,
		65576
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
		9,
		10,
		11,
		12,
		13,
		14,
		15,
		16
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
