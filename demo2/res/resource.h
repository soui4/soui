//stamp:0938c84edeab5a51
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
			_T("LAYOUT:XML_MAINWND"),
			_T("LAYOUT:DLG_TEST"),
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
		 const wchar_t * btn_close;
		 const wchar_t * btn_exit;
		 const wchar_t * lv_test;
		 const wchar_t * txt_test;
	}name;
	struct _id{
		int btn_close;
		int btn_exit;
		int lv_test;
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
		L"btn_close",
		L"btn_exit",
		L"lv_test",
		L"txt_test"
	}
	,
	{
		65539,
		65536,
		65537,
		65538
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
