//stamp:076e39c18e18b43d
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
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_restore;
		 const wchar_t * img_1;
		 const wchar_t * img_2;
		 const wchar_t * img_3;
		 const wchar_t * img_4;
		 const wchar_t * img_5;
		 const wchar_t * lb_catetory;
		 const wchar_t * lv_thumbnail;
	}name;
	struct _id{
		int btn_close;
		int btn_max;
		int btn_min;
		int btn_restore;
		int img_1;
		int img_2;
		int img_3;
		int img_4;
		int img_5;
		int lb_catetory;
		int lv_thumbnail;
	}id;
	struct _color{
		int blue;
		int colorHover;
		int gray;
		int green;
		int red;
		int white;
	}color;
	struct _string{
		int title;
		int ver;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"btn_close",
		L"btn_max",
		L"btn_min",
		L"btn_restore",
		L"img_1",
		L"img_2",
		L"img_3",
		L"img_4",
		L"img_5",
		L"lb_catetory",
		L"lv_thumbnail"
	}
	,
	{
		65536,
		65537,
		65539,
		65538,
		500,
		501,
		502,
		503,
		504,
		65540,
		65541
	}
	,
	{
		0,
		1,
		2,
		3,
		4,
		5
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
