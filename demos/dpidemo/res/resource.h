//stamp:094a358649949e2c
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
			const TCHAR * skin_125;
			const TCHAR * skin_150;
			const TCHAR * skin_200;
			}values;
		struct _lang{
			const TCHAR * cn;
			const TCHAR * en;
			}lang;
		struct _ICON{
			const TCHAR * ICON_LOGO;
			}ICON;
		struct _img{
			const TCHAR * pic_100;
			const TCHAR * pic_125;
			const TCHAR * pic_150;
			const TCHAR * pic_200;
			const TCHAR * chat_9;
			const TCHAR * png_tulips;
			}img;
		struct _smenu{
			const TCHAR * menu_lang;
			}smenu;
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
			_T("values:skin_125"),
			_T("values:skin_150"),
			_T("values:skin_200"),
		},
		{
			_T("lang:cn"),
			_T("lang:en"),
		},
		{
			_T("ICON:ICON_LOGO"),
		},
		{
			_T("img:pic_100"),
			_T("img:pic_125"),
			_T("img:pic_150"),
			_T("img:pic_200"),
			_T("img:chat.9"),
			_T("img:png_tulips"),
		},
		{
			_T("smenu:menu_lang"),
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
		 const wchar_t * btn_menu;
		 const wchar_t * btn_min;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_scale_100;
		 const wchar_t * btn_scale_125;
		 const wchar_t * btn_scale_150;
		 const wchar_t * btn_scale_200;
		 const wchar_t * cbx_flag;
		 const wchar_t * col1;
		 const wchar_t * col2;
		 const wchar_t * col3;
		 const wchar_t * col4;
		 const wchar_t * col5;
		 const wchar_t * edit_name;
		 const wchar_t * lang_cn;
		 const wchar_t * lang_en;
		 const wchar_t * mclv_test;
		 const wchar_t * txt_age;
		 const wchar_t * txt_gender;
		 const wchar_t * txt_name;
		 const wchar_t * txt_score;
		 const wchar_t * txt_test;
	}name;
	struct _id{
		int btn_close;
		int btn_max;
		int btn_menu;
		int btn_min;
		int btn_restore;
		int btn_scale_100;
		int btn_scale_125;
		int btn_scale_150;
		int btn_scale_200;
		int cbx_flag;
		int col1;
		int col2;
		int col3;
		int col4;
		int col5;
		int edit_name;
		int lang_cn;
		int lang_en;
		int mclv_test;
		int txt_age;
		int txt_gender;
		int txt_name;
		int txt_score;
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
		int female;
		int male;
		int title;
		int unknown;
		int ver;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"btn_close",
		L"btn_max",
		L"btn_menu",
		L"btn_min",
		L"btn_restore",
		L"btn_scale_100",
		L"btn_scale_125",
		L"btn_scale_150",
		L"btn_scale_200",
		L"cbx_flag",
		L"col1",
		L"col2",
		L"col3",
		L"col4",
		L"col5",
		L"edit_name",
		L"lang_cn",
		L"lang_en",
		L"mclv_test",
		L"txt_age",
		L"txt_gender",
		L"txt_name",
		L"txt_score",
		L"txt_test"
	}
	,
	{
		65540,
		65538,
		65536,
		65537,
		65539,
		65541,
		65542,
		65543,
		65544,
		65556,
		65547,
		65549,
		65551,
		65553,
		65555,
		65557,
		100,
		101,
		65546,
		65552,
		65550,
		65548,
		65554,
		65545
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
		2,
		3,
		4
	}
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
