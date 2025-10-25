//stamp:077100505bcf7480
/*<------------------------------------------------------------------------------------------------->*/
/*该文件由uiresbuilder生成，请不要手动修改*/
/*<------------------------------------------------------------------------------------------------->*/
#ifndef _UIRES_H_
#define _UIRES_H_
	struct _UIRES{
		struct _UIDEF{
			const TCHAR * XML_INIT;
			}UIDEF;
		struct _values{
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
			}values;
		struct _layout{
			const TCHAR * dlg_student_sms;
			}layout;
		struct _icon{
			const TCHAR * logo;
			}icon;
		struct _imgx{
			const TCHAR * cmn_scrollbar;
			}imgx;
		struct _image{
			const TCHAR * cmn_border;
			const TCHAR * sms_help;
			const TCHAR * sms_record;
			const TCHAR * sms_search;
			}image;
	};
#endif//_UIRES_H_
#ifdef INIT_R_DATA
struct _UIRES UIRES={
		{
			_T("UIDEF:XML_INIT"),
		},
		{
			_T("values:string"),
			_T("values:color"),
			_T("values:skin"),
		},
		{
			_T("layout:dlg.student.sms"),
		},
		{
			_T("icon:logo"),
		},
		{
			_T("imgx:cmn.scrollbar"),
		},
		{
			_T("image:cmn.border"),
			_T("image:sms.help"),
			_T("image:sms.record"),
			_T("image:sms.search"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * btn_send_sms;
		 const wchar_t * btn_sms_record;
		 const wchar_t * chk_select_group;
		 const wchar_t * col_loyal_degree;
		 const wchar_t * col_nick;
		 const wchar_t * col_time_span;
		 const wchar_t * edit_input;
		 const wchar_t * edit_sms_input;
		 const wchar_t * img_sms_help;
		 const wchar_t * lv_sms_record;
		 const wchar_t * scale_100;
		 const wchar_t * scale_150;
		 const wchar_t * scale_200;
		 const wchar_t * search_dropdown;
		 const wchar_t * tgl_tv_expand;
		 const wchar_t * tv_student;
		 const wchar_t * txt_group;
		 const wchar_t * txt_key;
		 const wchar_t * txt_loyal_degree;
		 const wchar_t * txt_nick;
		 const wchar_t * txt_send_time;
		 const wchar_t * txt_sms_checked_num;
		 const wchar_t * txt_sms_content;
		 const wchar_t * txt_sms_number;
		 const wchar_t * txt_sms_receiver;
		 const wchar_t * txt_time_span;
		 const wchar_t * txt_wordcount;
		 const wchar_t * wnd_record_frame;
	}name;
	struct _id{
		int btn_send_sms;
		int btn_sms_record;
		int chk_select_group;
		int col_loyal_degree;
		int col_nick;
		int col_time_span;
		int edit_input;
		int edit_sms_input;
		int img_sms_help;
		int lv_sms_record;
		int scale_100;
		int scale_150;
		int scale_200;
		int search_dropdown;
		int tgl_tv_expand;
		int tv_student;
		int txt_group;
		int txt_key;
		int txt_loyal_degree;
		int txt_nick;
		int txt_send_time;
		int txt_sms_checked_num;
		int txt_sms_content;
		int txt_sms_number;
		int txt_sms_receiver;
		int txt_time_span;
		int txt_wordcount;
		int wnd_record_frame;
	}id;
	struct _color{
		int black_light;
		int blue;
		int border;
		int gray;
		int green;
		int helper;
		int prompt;
		int red;
		int sms_header;
		int text_body;
		int text_disable;
		int text_slave;
		int theme;
		int warning;
		int white;
	}color;
	struct _string{
		int close;
		int col_sms_loyal_degree;
		int col_sms_nick;
		int col_sms_timespan;
		int cue_search;
		int cue_sms;
		int current_student;
		int expired_student;
		int scale;
		int send_sms;
		int sms_help;
		int sms_record;
		int sms_recv_head;
		int sms_recv_tail;
		int sms_tip_head;
		int sms_tip_tail;
		int stuent_sms;
	}string;

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"btn_send_sms",
		L"btn_sms_record",
		L"chk_select_group",
		L"col_loyal_degree",
		L"col_nick",
		L"col_time_span",
		L"edit_input",
		L"edit_sms_input",
		L"img_sms_help",
		L"lv_sms_record",
		L"scale_100",
		L"scale_150",
		L"scale_200",
		L"search_dropdown",
		L"tgl_tv_expand",
		L"tv_student",
		L"txt_group",
		L"txt_key",
		L"txt_loyal_degree",
		L"txt_nick",
		L"txt_send_time",
		L"txt_sms_checked_num",
		L"txt_sms_content",
		L"txt_sms_number",
		L"txt_sms_receiver",
		L"txt_time_span",
		L"txt_wordcount",
		L"wnd_record_frame"
	}
	,
	{
		65563,
		65560,
		65545,
		65549,
		65544,
		65548,
		65540,
		65553,
		65562,
		65556,
		65536,
		65537,
		65538,
		65541,
		65546,
		65543,
		65547,
		65542,
		65552,
		65550,
		65557,
		65539,
		65559,
		65561,
		65558,
		65551,
		65554,
		65555
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
		14
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
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
