//stamp:03b89d8f9a6140c8
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
		struct _IMG{
			const TCHAR * ICON_LOGO;
			}IMG;
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
			_T("IMG:ICON_LOGO"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * animation_target;
		 const wchar_t * btn_close;
		 const wchar_t * btn_height_anim;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_multi_anim;
		 const wchar_t * btn_offset_anim;
		 const wchar_t * btn_reset;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_size_anim;
		 const wchar_t * btn_weight_anim;
		 const wchar_t * btn_width_anim;
		 const wchar_t * linear_container;
	}name;
	struct _id{
		int animation_target;
		int btn_close;
		int btn_height_anim;
		int btn_max;
		int btn_min;
		int btn_multi_anim;
		int btn_offset_anim;
		int btn_reset;
		int btn_restore;
		int btn_size_anim;
		int btn_weight_anim;
		int btn_width_anim;
		int linear_container;
	}id;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"animation_target",
		L"btn_close",
		L"btn_height_anim",
		L"btn_max",
		L"btn_min",
		L"btn_multi_anim",
		L"btn_offset_anim",
		L"btn_reset",
		L"btn_restore",
		L"btn_size_anim",
		L"btn_weight_anim",
		L"btn_width_anim",
		L"linear_container"
	}
	,
	{
		65541,
		65539,
		65543,
		65537,
		65536,
		65546,
		65547,
		65548,
		65538,
		65544,
		65545,
		65542,
		65540
	}
	,
};
#else
extern struct _R R;
#endif//INIT_R_DATA
