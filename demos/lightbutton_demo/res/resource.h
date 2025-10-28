//stamp:01dc4719eaae0b1c
/*<------------------------------------------------------------------------------------------------->*/
/*该文件由uiresbuilder生成，请不要手动修改*/
/*<------------------------------------------------------------------------------------------------->*/
#ifndef _UIRES_H_
#define _UIRES_H_
	struct _UIRES{
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
		 const wchar_t * btn_black;
		 const wchar_t * btn_blue;
		 const wchar_t * btn_close;
		 const wchar_t * btn_gray;
		 const wchar_t * btn_green;
		 const wchar_t * btn_light_blue;
		 const wchar_t * btn_light_green;
		 const wchar_t * btn_light_red;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_red;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_start_alarm;
		 const wchar_t * btn_stop_alarm;
		 const wchar_t * btn_toggle_overlay;
		 const wchar_t * btn_toggle_rect;
		 const wchar_t * btn_yellow;
		 const wchar_t * lightbutton1;
		 const wchar_t * lightbutton2;
		 const wchar_t * lightbutton3;
		 const wchar_t * lightbutton4;
		 const wchar_t * lightbutton5;
		 const wchar_t * lightbutton6;
	}name;
	struct _id{
		int btn_black;
		int btn_blue;
		int btn_close;
		int btn_gray;
		int btn_green;
		int btn_light_blue;
		int btn_light_green;
		int btn_light_red;
		int btn_max;
		int btn_min;
		int btn_red;
		int btn_restore;
		int btn_start_alarm;
		int btn_stop_alarm;
		int btn_toggle_overlay;
		int btn_toggle_rect;
		int btn_yellow;
		int lightbutton1;
		int lightbutton2;
		int lightbutton3;
		int lightbutton4;
		int lightbutton5;
		int lightbutton6;
	}id;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"btn_black",
		L"btn_blue",
		L"btn_close",
		L"btn_gray",
		L"btn_green",
		L"btn_light_blue",
		L"btn_light_green",
		L"btn_light_red",
		L"btn_max",
		L"btn_min",
		L"btn_red",
		L"btn_restore",
		L"btn_start_alarm",
		L"btn_stop_alarm",
		L"btn_toggle_overlay",
		L"btn_toggle_rect",
		L"btn_yellow",
		L"lightbutton1",
		L"lightbutton2",
		L"lightbutton3",
		L"lightbutton4",
		L"lightbutton5",
		L"lightbutton6"
	}
	,
	{
		65550,
		65549,
		65539,
		65551,
		65546,
		65552,
		65554,
		65553,
		65537,
		65536,
		65547,
		65538,
		65555,
		65556,
		65558,
		65557,
		65548,
		65540,
		65541,
		65542,
		65543,
		65544,
		65545
	}
	,
};
#else
extern struct _R R;
#endif//INIT_R_DATA
