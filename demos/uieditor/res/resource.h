//stamp:2181ed602587b80d
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
			const TCHAR * property_panel;
			const TCHAR * xml_uidesigner_main;
			const TCHAR * UIDESIGNER_XML_NEW_LAYOUT;
			const TCHAR * UIDESIGNER_XML_NEW_SKIN;
			const TCHAR * DLG_INPUT;
			const TCHAR * UIDESIGNER_XML_ABOUT;
			const TCHAR * UIDESIGNER_XML_INSERT_ELEMENT;
			const TCHAR * designer_wnd;
			const TCHAR * dlg_skin_viewer;
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
			const TCHAR * png_filetypes;
			const TCHAR * png_item_bk;
			}IMG;
		struct _ICON{
			const TCHAR * ICON_logo;
			}ICON;
		struct _smenu{
			const TCHAR * menu_layoutfile;
			const TCHAR * menu_recent;
			const TCHAR * menu_file_ctx;
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
			_T("LAYOUT:property_panel"),
			_T("LAYOUT:xml_uidesigner_main"),
			_T("LAYOUT:UIDESIGNER_XML_NEW_LAYOUT"),
			_T("LAYOUT:UIDESIGNER_XML_NEW_SKIN"),
			_T("LAYOUT:DLG_INPUT"),
			_T("LAYOUT:UIDESIGNER_XML_ABOUT"),
			_T("LAYOUT:UIDESIGNER_XML_INSERT_ELEMENT"),
			_T("LAYOUT:designer_wnd"),
			_T("LAYOUT:dlg_skin_viewer"),
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
			_T("IMG:png_filetypes"),
			_T("IMG:png_item_bk"),
		},
		{
			_T("ICON:ICON_logo"),
		},
		{
			_T("smenu:menu_layoutfile"),
			_T("smenu:menu_recent"),
			_T("smenu:menu_file_ctx"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * NAME_UIDESIGNER_NEW_SKIN_LB;
		 const wchar_t * NAME_UIDESIGNER_btn_OK;
		 const wchar_t * NAME_UIDESIGNER_btn_close;
		 const wchar_t * NAME_UIDESIGNER_btn_dlg;
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
		 const wchar_t * btn_next;
		 const wchar_t * btn_prev;
		 const wchar_t * btn_restore;
		 const wchar_t * button1st;
		 const wchar_t * button2nd;
		 const wchar_t * button3rd;
		 const wchar_t * chk_auto_size;
		 const wchar_t * chk_autosave;
		 const wchar_t * color_preview;
		 const wchar_t * edtInput;
		 const wchar_t * edt_label;
		 const wchar_t * img_icon_exclamation;
		 const wchar_t * img_icon_information;
		 const wchar_t * img_icon_question;
		 const wchar_t * img_preview;
		 const wchar_t * img_preview_auto;
		 const wchar_t * img_preview_full;
		 const wchar_t * img_state;
		 const wchar_t * img_viewer;
		 const wchar_t * item_icon;
		 const wchar_t * item_text;
		 const wchar_t * lbl_index;
		 const wchar_t * lv_skin;
		 const wchar_t * lv_tb_skin;
		 const wchar_t * lv_tb_widget;
		 const wchar_t * msgicon;
		 const wchar_t * msgtext;
		 const wchar_t * msgtitle;
		 const wchar_t * new_layout_path;
		 const wchar_t * new_layout_resname;
		 const wchar_t * page_image;
		 const wchar_t * page_xml;
		 const wchar_t * propgrid_element;
		 const wchar_t * skin_preview;
		 const wchar_t * slider_zoom;
		 const wchar_t * tab_editor;
		 const wchar_t * tb_prop;
		 const wchar_t * toolbar_btn_Close;
		 const wchar_t * toolbar_btn_NewInclude;
		 const wchar_t * toolbar_btn_NewLayout;
		 const wchar_t * toolbar_btn_Open;
		 const wchar_t * toolbar_btn_formatxml;
		 const wchar_t * toolbar_btn_recent;
		 const wchar_t * toolbar_btn_savexml;
		 const wchar_t * toolbar_btn_viewskin;
		 const wchar_t * txt_desc;
		 const wchar_t * txt_label;
		 const wchar_t * txt_path;
		 const wchar_t * txt_prop_desc;
		 const wchar_t * txt_prop_title;
		 const wchar_t * txt_title;
		 const wchar_t * txt_value;
		 const wchar_t * uidesigner_scintilla;
		 const wchar_t * uidesigner_wnd_layout;
		 const wchar_t * uidesigner_wnd_xmltree;
		 const wchar_t * workspace_treeview;
		 const wchar_t * xml_editor;
		 const wchar_t * xml_editor_main;
		 const wchar_t * xml_editor_sub;
	}name;
	struct _id{
		int NAME_UIDESIGNER_NEW_SKIN_LB;
		int NAME_UIDESIGNER_btn_OK;
		int NAME_UIDESIGNER_btn_close;
		int NAME_UIDESIGNER_btn_dlg;
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
		int btn_next;
		int btn_prev;
		int btn_restore;
		int button1st;
		int button2nd;
		int button3rd;
		int chk_auto_size;
		int chk_autosave;
		int color_preview;
		int edtInput;
		int edt_label;
		int img_icon_exclamation;
		int img_icon_information;
		int img_icon_question;
		int img_preview;
		int img_preview_auto;
		int img_preview_full;
		int img_state;
		int img_viewer;
		int item_icon;
		int item_text;
		int lbl_index;
		int lv_skin;
		int lv_tb_skin;
		int lv_tb_widget;
		int msgicon;
		int msgtext;
		int msgtitle;
		int new_layout_path;
		int new_layout_resname;
		int page_image;
		int page_xml;
		int propgrid_element;
		int skin_preview;
		int slider_zoom;
		int tab_editor;
		int tb_prop;
		int toolbar_btn_Close;
		int toolbar_btn_NewInclude;
		int toolbar_btn_NewLayout;
		int toolbar_btn_Open;
		int toolbar_btn_formatxml;
		int toolbar_btn_recent;
		int toolbar_btn_savexml;
		int toolbar_btn_viewskin;
		int txt_desc;
		int txt_label;
		int txt_path;
		int txt_prop_desc;
		int txt_prop_title;
		int txt_title;
		int txt_value;
		int uidesigner_scintilla;
		int uidesigner_wnd_layout;
		int uidesigner_wnd_xmltree;
		int workspace_treeview;
		int xml_editor;
		int xml_editor_main;
		int xml_editor_sub;
	}id;
	struct _color{
		int bkBlack1;
		int bkBlack2;
		int black;
		int blue;
		int border;
		int gray;
		int green;
		int imgprviewbg;
		int itemHover;
		int itemSelected;
		int red;
		int white;
	}color;
	struct _string{
		int add_skin;
		int add_uires;
		int auto_size;
		int copy;
		int cut;
		int delete_cpp;
		int explorer;
		int format_xml;
		int image_viewer;
		int new_folder;
		int new_include;
		int new_layout;
		int new_xml;
		int openincode;
		int paste;
		int project_struct;
		int property;
		int rename;
		int save_xml;
		int skin_build;
		int title;
		int ver;
		int xml_editor;
		int xml_struct;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"NAME_UIDESIGNER_NEW_SKIN_LB",
		L"NAME_UIDESIGNER_btn_OK",
		L"NAME_UIDESIGNER_btn_close",
		L"NAME_UIDESIGNER_btn_dlg",
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
		L"btn_next",
		L"btn_prev",
		L"btn_restore",
		L"button1st",
		L"button2nd",
		L"button3rd",
		L"chk_auto_size",
		L"chk_autosave",
		L"color_preview",
		L"edtInput",
		L"edt_label",
		L"img_icon_exclamation",
		L"img_icon_information",
		L"img_icon_question",
		L"img_preview",
		L"img_preview_auto",
		L"img_preview_full",
		L"img_state",
		L"img_viewer",
		L"item_icon",
		L"item_text",
		L"lbl_index",
		L"lv_skin",
		L"lv_tb_skin",
		L"lv_tb_widget",
		L"msgicon",
		L"msgtext",
		L"msgtitle",
		L"new_layout_path",
		L"new_layout_resname",
		L"page_image",
		L"page_xml",
		L"propgrid_element",
		L"skin_preview",
		L"slider_zoom",
		L"tab_editor",
		L"tb_prop",
		L"toolbar_btn_Close",
		L"toolbar_btn_NewInclude",
		L"toolbar_btn_NewLayout",
		L"toolbar_btn_Open",
		L"toolbar_btn_formatxml",
		L"toolbar_btn_recent",
		L"toolbar_btn_savexml",
		L"toolbar_btn_viewskin",
		L"txt_desc",
		L"txt_label",
		L"txt_path",
		L"txt_prop_desc",
		L"txt_prop_title",
		L"txt_title",
		L"txt_value",
		L"uidesigner_scintilla",
		L"uidesigner_wnd_layout",
		L"uidesigner_wnd_xmltree",
		L"workspace_treeview",
		L"xml_editor",
		L"xml_editor_main",
		L"xml_editor_sub"
	}
	,
	{
		65592,
		65589,
		65585,
		65588,
		65584,
		65560,
		65546,
		65547,
		65591,
		65593,
		65542,
		65552,
		65548,
		65550,
		65549,
		65607,
		65605,
		65551,
		65543,
		65544,
		65545,
		65604,
		65559,
		65570,
		65594,
		65564,
		65540,
		65538,
		65539,
		65600,
		65603,
		65602,
		65562,
		65582,
		65578,
		65579,
		65606,
		65601,
		65577,
		65580,
		65537,
		65541,
		65536,
		65587,
		65586,
		65581,
		65572,
		65566,
		65569,
		65583,
		65571,
		65595,
		65555,
		65557,
		65556,
		65553,
		65574,
		65554,
		65573,
		65558,
		65567,
		65563,
		65596,
		65598,
		65597,
		65590,
		65568,
		100000,
		100000,
		65565,
		65561,
		65599,
		65575,
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
		11
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
		16,
		17,
		18,
		19,
		20,
		21,
		22,
		23
	}
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
