//stamp:1824ed2d8efc7f0c
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
			const TCHAR * dlg_main;
			const TCHAR * page_imgmerge;
			const TCHAR * page_folder;
			const TCHAR * page_codeline;
			const TCHAR * page_2unicode;
			const TCHAR * page_folderscan;
			const TCHAR * page_md5;
			const TCHAR * page_about;
			const TCHAR * page_window_spy;
			}LAYOUT;
		struct _xml{
			const TCHAR * syntax;
			const TCHAR * menu_filetype;
			}xml;
		struct _IMG{
			const TCHAR * png_page_icons;
			const TCHAR * png_tab_main;
			const TCHAR * png_spyhost;
			}IMG;
		struct _img_imagemerge{
			const TCHAR * png_transbkgnd;
			}img_imagemerge;
		struct _img_codeline{
			const TCHAR * png_foldericon;
			const TCHAR * png_tabskin;
			}img_codeline;
		struct _img_folderscan{
			const TCHAR * png_scan;
			}img_folderscan;
		struct _ICON{
			const TCHAR * ICON_LOGO;
			}ICON;
		struct _SMENUEX{
			const TCHAR * menuex_group;
			const TCHAR * menuex_item;
			const TCHAR * menuex_none;
			}SMENUEX;
		struct _CURSOR{
			const TCHAR * target;
			}CURSOR;
	};
#endif//_UIRES_H_
#ifdef INIT_R_DATA
struct _UIRES UIRES={
		{
			_T("UIDEF:XML_INIT"),
		},
		{
			_T("LAYOUT:dlg_main"),
			_T("LAYOUT:page_imgmerge"),
			_T("LAYOUT:page_folder"),
			_T("LAYOUT:page_codeline"),
			_T("LAYOUT:page_2unicode"),
			_T("LAYOUT:page_folderscan"),
			_T("LAYOUT:page_md5"),
			_T("LAYOUT:page_about"),
			_T("LAYOUT:page_window_spy"),
		},
		{
			_T("xml:syntax"),
			_T("xml:menu_filetype"),
		},
		{
			_T("IMG:png_page_icons"),
			_T("IMG:png_tab_main"),
			_T("IMG:png_spyhost"),
		},
		{
			_T("img_imagemerge:png_transbkgnd"),
		},
		{
			_T("img_codeline:png_foldericon"),
			_T("img_codeline:png_tabskin"),
		},
		{
			_T("img_folderscan:png_scan"),
		},
		{
			_T("ICON:ICON_LOGO"),
		},
		{
			_T("SMENUEX:menuex_group"),
			_T("SMENUEX:menuex_item"),
			_T("SMENUEX:menuex_none"),
		},
		{
			_T("CURSOR:target"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * ani_scan;
		 const wchar_t * btn_clear;
		 const wchar_t * btn_close;
		 const wchar_t * btn_dropdown_filetypes;
		 const wchar_t * btn_go;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_save;
		 const wchar_t * btn_save_ico;
		 const wchar_t * btn_save_sub;
		 const wchar_t * btn_split;
		 const wchar_t * edit_dir;
		 const wchar_t * edit_filetypes;
		 const wchar_t * edit_input;
		 const wchar_t * edit_number;
		 const wchar_t * edit_output;
		 const wchar_t * img_capture_host;
		 const wchar_t * lst_linecounter_report;
		 const wchar_t * page_2unicode;
		 const wchar_t * page_codeline;
		 const wchar_t * page_dir;
		 const wchar_t * page_folderscan;
		 const wchar_t * page_imagemerge;
		 const wchar_t * page_md5;
		 const wchar_t * page_result;
		 const wchar_t * prog_run;
		 const wchar_t * radio_horz;
		 const wchar_t * radio_vert;
		 const wchar_t * split_frame;
		 const wchar_t * split_tree;
		 const wchar_t * tab_codeline;
		 const wchar_t * tab_main;
		 const wchar_t * toolbar;
		 const wchar_t * tree_dir;
		 const wchar_t * txt_blanklines;
		 const wchar_t * txt_codelines;
		 const wchar_t * txt_files;
		 const wchar_t * txt_folders;
		 const wchar_t * txt_remarklines;
		 const wchar_t * wnd_canvas;
		 const wchar_t * wnd_class;
		 const wchar_t * wnd_exepath;
		 const wchar_t * wnd_exstyle;
		 const wchar_t * wnd_hwnd;
		 const wchar_t * wnd_proid;
		 const wchar_t * wnd_rect;
		 const wchar_t * wnd_style;
		 const wchar_t * wnd_title;
	}name;
	struct _id{
		int ani_scan;
		int btn_clear;
		int btn_close;
		int btn_dropdown_filetypes;
		int btn_go;
		int btn_max;
		int btn_min;
		int btn_restore;
		int btn_save;
		int btn_save_ico;
		int btn_save_sub;
		int btn_split;
		int edit_dir;
		int edit_filetypes;
		int edit_input;
		int edit_number;
		int edit_output;
		int img_capture_host;
		int lst_linecounter_report;
		int page_2unicode;
		int page_codeline;
		int page_dir;
		int page_folderscan;
		int page_imagemerge;
		int page_md5;
		int page_result;
		int prog_run;
		int radio_horz;
		int radio_vert;
		int split_frame;
		int split_tree;
		int tab_codeline;
		int tab_main;
		int toolbar;
		int tree_dir;
		int txt_blanklines;
		int txt_codelines;
		int txt_files;
		int txt_folders;
		int txt_remarklines;
		int wnd_canvas;
		int wnd_class;
		int wnd_exepath;
		int wnd_exstyle;
		int wnd_hwnd;
		int wnd_proid;
		int wnd_rect;
		int wnd_style;
		int wnd_title;
	}id;
	struct _string{
		int title;
		int ver;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"ani_scan",
		L"btn_clear",
		L"btn_close",
		L"btn_dropdown_filetypes",
		L"btn_go",
		L"btn_max",
		L"btn_min",
		L"btn_restore",
		L"btn_save",
		L"btn_save_ico",
		L"btn_save_sub",
		L"btn_split",
		L"edit_dir",
		L"edit_filetypes",
		L"edit_input",
		L"edit_number",
		L"edit_output",
		L"img_capture_host",
		L"lst_linecounter_report",
		L"page_2unicode",
		L"page_codeline",
		L"page_dir",
		L"page_folderscan",
		L"page_imagemerge",
		L"page_md5",
		L"page_result",
		L"prog_run",
		L"radio_horz",
		L"radio_vert",
		L"split_frame",
		L"split_tree",
		L"tab_codeline",
		L"tab_main",
		L"toolbar",
		L"tree_dir",
		L"txt_blanklines",
		L"txt_codelines",
		L"txt_files",
		L"txt_folders",
		L"txt_remarklines",
		L"wnd_canvas",
		L"wnd_class",
		L"wnd_exepath",
		L"wnd_exstyle",
		L"wnd_hwnd",
		L"wnd_proid",
		L"wnd_rect",
		L"wnd_style",
		L"wnd_title"
	}
	,
	{
		65572,
		65551,
		65537,
		65558,
		65559,
		65538,
		65540,
		65539,
		65549,
		65550,
		65556,
		65554,
		1000,
		65557,
		65573,
		65555,
		65575,
		65536,
		65566,
		65544,
		65543,
		65564,
		65545,
		65542,
		65546,
		65565,
		65562,
		65552,
		65553,
		65574,
		65560,
		65563,
		65541,
		65548,
		65561,
		65570,
		65569,
		65568,
		65567,
		65571,
		65547,
		65577,
		65581,
		65583,
		65576,
		65580,
		65579,
		65582,
		65578
	}
	,
	{
		0,
		1
	}
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
