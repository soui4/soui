//stamp:094a1be1cd1752f8
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
			const TCHAR * dlg_server;
			const TCHAR * dlg_client;
			}LAYOUT;
		struct _values{
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
			}values;
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
			_T("LAYOUT:dlg_server"),
			_T("LAYOUT:dlg_client"),
		},
		{
			_T("values:string"),
			_T("values:color"),
			_T("values:skin"),
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
		 const wchar_t * btn_break;
		 const wchar_t * btn_close;
		 const wchar_t * btn_conn;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_sum;
		 const wchar_t * btn_sum_math;
		 const wchar_t * btn_sum_str;
		 const wchar_t * edit_param1;
		 const wchar_t * edit_param2;
		 const wchar_t * edit_result;
		 const wchar_t * re_log;
		 const wchar_t * txt_status;
	}name;
	struct _id{
		int btn_break;
		int btn_close;
		int btn_conn;
		int btn_max;
		int btn_min;
		int btn_restore;
		int btn_sum;
		int btn_sum_math;
		int btn_sum_str;
		int edit_param1;
		int edit_param2;
		int edit_result;
		int re_log;
		int txt_status;
	}id;
	struct _color{
		int blue;
		int gray;
		int green;
		int red;
		int white;
	}color;
	struct _string{
		int client;
		int server;
		int ver;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"btn_break",
		L"btn_close",
		L"btn_conn",
		L"btn_max",
		L"btn_min",
		L"btn_restore",
		L"btn_sum",
		L"btn_sum_math",
		L"btn_sum_str",
		L"edit_param1",
		L"edit_param2",
		L"edit_result",
		L"re_log",
		L"txt_status"
	}
	,
	{
		65541,
		65536,
		65543,
		65537,
		65539,
		65538,
		65549,
		65547,
		65548,
		65544,
		65545,
		65546,
		65540,
		65542
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
