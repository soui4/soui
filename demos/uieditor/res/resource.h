//stamp:1fa074e6d672e440
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
			const TCHAR * xml_messagebox;
			const TCHAR * XML_MAINWND;
			const TCHAR * xml_mainwnd_left;
			const TCHAR * xml_mainwnd_right;
			const TCHAR * xml_uidesigner_main;
			const TCHAR * xml_res_mgr;
			const TCHAR * UIDESIGNER_XML_NEW_LAYOUT;
			const TCHAR * UIDESIGNER_XML_NEW_SKIN;
			const TCHAR * UIDESIGNER_XML_INPUT;
			const TCHAR * UIDESIGNER_XML_ABOUT;
			const TCHAR * UIDESIGNER_XML_INSERT_ELEMENT;
			const TCHAR * designer_wnd;
			}LAYOUT;
		struct _values{
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
			}values;
		struct _IMG{
			const TCHAR * icon_main;
			const TCHAR * caption_line;
			const TCHAR * png_tool_button;
			const TCHAR * png_prop_switch;
			const TCHAR * png_tab_skin;
			const TCHAR * png_btn_blue;
			const TCHAR * png_app_button;
			const TCHAR * default_scrollbar;
			const TCHAR * open_menu_png;
			const TCHAR * tree_toggle_png;
			const TCHAR * menu_skin_png;
			const TCHAR * menu_sep_png;
			const TCHAR * png_logo;
			const TCHAR * png_toolbar_state;
			const TCHAR * png_toolbar_prop;
			const TCHAR * png_btn_help;
			const TCHAR * png_designer_background;
			}IMG;
		struct _ICON{
			const TCHAR * ICON_logo;
			}ICON;
		struct _smenu{
			const TCHAR * menu_layoutfile;
			const TCHAR * menu_recent;
			}smenu;
	};
#endif//_UIRES_H_
#ifdef INIT_R_DATA
struct _UIRES UIRES={
		{
			_T("UIDEF:XML_INIT"),
		},
		{
			_T("LAYOUT:xml_messagebox"),
			_T("LAYOUT:XML_MAINWND"),
			_T("LAYOUT:xml_mainwnd_left"),
			_T("LAYOUT:xml_mainwnd_right"),
			_T("LAYOUT:xml_uidesigner_main"),
			_T("LAYOUT:xml_res_mgr"),
			_T("LAYOUT:UIDESIGNER_XML_NEW_LAYOUT"),
			_T("LAYOUT:UIDESIGNER_XML_NEW_SKIN"),
			_T("LAYOUT:UIDESIGNER_XML_INPUT"),
			_T("LAYOUT:UIDESIGNER_XML_ABOUT"),
			_T("LAYOUT:UIDESIGNER_XML_INSERT_ELEMENT"),
			_T("LAYOUT:designer_wnd"),
		},
		{
			_T("values:string"),
			_T("values:color"),
			_T("values:skin"),
		},
		{
			_T("IMG:icon_main"),
			_T("IMG:caption_line"),
			_T("IMG:png_tool_button"),
			_T("IMG:png_prop_switch"),
			_T("IMG:png_tab_skin"),
			_T("IMG:png_btn_blue"),
			_T("IMG:png_app_button"),
			_T("IMG:default.scrollbar"),
			_T("IMG:open_menu_png"),
			_T("IMG:tree_toggle_png"),
			_T("IMG:menu_skin_png"),
			_T("IMG:menu_sep_png"),
			_T("IMG:png_logo"),
			_T("IMG:png_toolbar_state"),
			_T("IMG:png_toolbar_prop"),
			_T("IMG:png_btn_help"),
			_T("IMG:png_designer_background"),
		},
		{
			_T("ICON:ICON_logo"),
		},
		{
			_T("smenu:menu_layoutfile"),
			_T("smenu:menu_recent"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * NAME_UIDESIGNER_IMG_VIEW;
		 const wchar_t * NAME_UIDESIGNER_LB_ZYLX;
		 const wchar_t * NAME_UIDESIGNER_NEW_SKIN_LB;
		 const wchar_t * NAME_UIDESIGNER_btn_OK;
		 const wchar_t * NAME_UIDESIGNER_btn_ZYLX_DEL;
		 const wchar_t * NAME_UIDESIGNER_btn_ZYLX_new;
		 const wchar_t * NAME_UIDESIGNER_btn_ZY_DEL;
		 const wchar_t * NAME_UIDESIGNER_btn_ZY_NEW;
		 const wchar_t * NAME_UIDESIGNER_btn_ZY_NEWALL;
		 const wchar_t * NAME_UIDESIGNER_btn_close;
		 const wchar_t * NAME_UIDESIGNER_btn_dlg;
		 const wchar_t * NAME_UIDESIGNER_edit_SEARCH;
		 const wchar_t * NAME_UIDESIGNER_main_icon;
		 const wchar_t * NAME_UIDESIGNER_split_col;
		 const wchar_t * UI_main_caption;
		 const wchar_t * apptitle;
		 const wchar_t * btnCancel;
		 const wchar_t * btnOK;
		 const wchar_t * btnSwitch;
		 const wchar_t * btn_close;
		 const wchar_t * btn_help;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_restore;
		 const wchar_t * button1st;
		 const wchar_t * button2nd;
		 const wchar_t * button3rd;
		 const wchar_t * chk_autosave;
		 const wchar_t * color_preview;
		 const wchar_t * edtInput;
		 const wchar_t * img_icon_exclamation;
		 const wchar_t * img_icon_information;
		 const wchar_t * img_icon_question;
		 const wchar_t * img_preview;
		 const wchar_t * item_icon;
		 const wchar_t * item_text;
		 const wchar_t * lv_res_path;
		 const wchar_t * lv_tb_skin;
		 const wchar_t * lv_tb_widget;
		 const wchar_t * msgicon;
		 const wchar_t * msgtext;
		 const wchar_t * msgtitle;
		 const wchar_t * new_layout_path;
		 const wchar_t * new_layout_resname;
		 const wchar_t * propgrid_element;
		 const wchar_t * sel_image_size;
		 const wchar_t * skin_preview;
		 const wchar_t * tb_prop;
		 const wchar_t * toolbar_btn_Close;
		 const wchar_t * toolbar_btn_NewInclude;
		 const wchar_t * toolbar_btn_NewLayout;
		 const wchar_t * toolbar_btn_Open;
		 const wchar_t * toolbar_btn_recent;
		 const wchar_t * toolbar_btn_resmgr;
		 const wchar_t * toolbar_btn_savexml;
		 const wchar_t * txt_desc;
		 const wchar_t * txt_name;
		 const wchar_t * txt_path;
		 const wchar_t * txt_prop_desc;
		 const wchar_t * txt_prop_title;
		 const wchar_t * txt_title;
		 const wchar_t * txt_value;
		 const wchar_t * uidesigner_scintilla;
		 const wchar_t * uidesigner_wnd_layout;
		 const wchar_t * uidesigner_wnd_xmltree;
		 const wchar_t * workspace_tab;
		 const wchar_t * workspace_tree;
		 const wchar_t * workspace_xmlfile_lb;
		 const wchar_t * xml_editor;
		 const wchar_t * xml_editor_splitcol;
	}name;
	struct _id{
		int NAME_UIDESIGNER_IMG_VIEW;
		int NAME_UIDESIGNER_LB_ZYLX;
		int NAME_UIDESIGNER_NEW_SKIN_LB;
		int NAME_UIDESIGNER_btn_OK;
		int NAME_UIDESIGNER_btn_ZYLX_DEL;
		int NAME_UIDESIGNER_btn_ZYLX_new;
		int NAME_UIDESIGNER_btn_ZY_DEL;
		int NAME_UIDESIGNER_btn_ZY_NEW;
		int NAME_UIDESIGNER_btn_ZY_NEWALL;
		int NAME_UIDESIGNER_btn_close;
		int NAME_UIDESIGNER_btn_dlg;
		int NAME_UIDESIGNER_edit_SEARCH;
		int NAME_UIDESIGNER_main_icon;
		int NAME_UIDESIGNER_split_col;
		int UI_main_caption;
		int apptitle;
		int btnCancel;
		int btnOK;
		int btnSwitch;
		int btn_close;
		int btn_help;
		int btn_max;
		int btn_min;
		int btn_restore;
		int button1st;
		int button2nd;
		int button3rd;
		int chk_autosave;
		int color_preview;
		int edtInput;
		int img_icon_exclamation;
		int img_icon_information;
		int img_icon_question;
		int img_preview;
		int item_icon;
		int item_text;
		int lv_res_path;
		int lv_tb_skin;
		int lv_tb_widget;
		int msgicon;
		int msgtext;
		int msgtitle;
		int new_layout_path;
		int new_layout_resname;
		int propgrid_element;
		int sel_image_size;
		int skin_preview;
		int tb_prop;
		int toolbar_btn_Close;
		int toolbar_btn_NewInclude;
		int toolbar_btn_NewLayout;
		int toolbar_btn_Open;
		int toolbar_btn_recent;
		int toolbar_btn_resmgr;
		int toolbar_btn_savexml;
		int txt_desc;
		int txt_name;
		int txt_path;
		int txt_prop_desc;
		int txt_prop_title;
		int txt_title;
		int txt_value;
		int uidesigner_scintilla;
		int uidesigner_wnd_layout;
		int uidesigner_wnd_xmltree;
		int workspace_tab;
		int workspace_tree;
		int workspace_xmlfile_lb;
		int xml_editor;
		int xml_editor_splitcol;
	}id;
	struct _color{
		int bkBlack1;
		int bkBlack2;
		int black;
		int blue;
		int gray;
		int green;
		int imgprviewbg;
		int itemHover;
		int itemSelected;
		int red;
		int white;
	}color;
	struct _string{
		int openincode;
		int skin_build;
		int title;
		int ver;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"NAME_UIDESIGNER_IMG_VIEW",
		L"NAME_UIDESIGNER_LB_ZYLX",
		L"NAME_UIDESIGNER_NEW_SKIN_LB",
		L"NAME_UIDESIGNER_btn_OK",
		L"NAME_UIDESIGNER_btn_ZYLX_DEL",
		L"NAME_UIDESIGNER_btn_ZYLX_new",
		L"NAME_UIDESIGNER_btn_ZY_DEL",
		L"NAME_UIDESIGNER_btn_ZY_NEW",
		L"NAME_UIDESIGNER_btn_ZY_NEWALL",
		L"NAME_UIDESIGNER_btn_close",
		L"NAME_UIDESIGNER_btn_dlg",
		L"NAME_UIDESIGNER_edit_SEARCH",
		L"NAME_UIDESIGNER_main_icon",
		L"NAME_UIDESIGNER_split_col",
		L"UI_main_caption",
		L"apptitle",
		L"btnCancel",
		L"btnOK",
		L"btnSwitch",
		L"btn_close",
		L"btn_help",
		L"btn_max",
		L"btn_min",
		L"btn_restore",
		L"button1st",
		L"button2nd",
		L"button3rd",
		L"chk_autosave",
		L"color_preview",
		L"edtInput",
		L"img_icon_exclamation",
		L"img_icon_information",
		L"img_icon_question",
		L"img_preview",
		L"item_icon",
		L"item_text",
		L"lv_res_path",
		L"lv_tb_skin",
		L"lv_tb_widget",
		L"msgicon",
		L"msgtext",
		L"msgtitle",
		L"new_layout_path",
		L"new_layout_resname",
		L"propgrid_element",
		L"sel_image_size",
		L"skin_preview",
		L"tb_prop",
		L"toolbar_btn_Close",
		L"toolbar_btn_NewInclude",
		L"toolbar_btn_NewLayout",
		L"toolbar_btn_Open",
		L"toolbar_btn_recent",
		L"toolbar_btn_resmgr",
		L"toolbar_btn_savexml",
		L"txt_desc",
		L"txt_name",
		L"txt_path",
		L"txt_prop_desc",
		L"txt_prop_title",
		L"txt_title",
		L"txt_value",
		L"uidesigner_scintilla",
		L"uidesigner_wnd_layout",
		L"uidesigner_wnd_xmltree",
		L"workspace_tab",
		L"workspace_tree",
		L"workspace_xmlfile_lb",
		L"xml_editor",
		L"xml_editor_splitcol"
	}
	,
	{
		65588,
		65580,
		65598,
		65595,
		65579,
		65578,
		65582,
		65581,
		65583,
		65591,
		65594,
		65584,
		65577,
		65561,
		65546,
		65547,
		65597,
		65590,
		65542,
		65552,
		65548,
		65550,
		65549,
		65551,
		65543,
		65544,
		65545,
		65560,
		65572,
		65599,
		65540,
		65538,
		65539,
		65603,
		65574,
		65575,
		65585,
		65573,
		65576,
		65537,
		65541,
		65536,
		65593,
		65592,
		65568,
		65589,
		65571,
		65600,
		65555,
		65557,
		65556,
		65553,
		65554,
		65559,
		65558,
		65569,
		65586,
		65587,
		65602,
		65601,
		65596,
		65570,
		100000,
		100000,
		65565,
		65562,
		65563,
		65564,
		65566,
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
		6,
		7,
		8,
		9,
		10
	}
	,
	{
		0,
		1,
		2,
		3
	}
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
