//stamp:09532ff18f6632ad
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
			const TCHAR * fund_detail;
			}LAYOUT;
		struct _ICON{
			const TCHAR * ICON_LOGO;
			}ICON;
		struct _values{
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
			const TCHAR * font;
			}values;
		struct _svg{
			const TCHAR * logo;
			const TCHAR * scrollbar;
			}svg;
	};
#endif//_UIRES_H_
#ifdef INIT_R_DATA
struct _UIRES UIRES={
		{
			_T("UIDEF:XML_INIT"),
		},
		{
			_T("LAYOUT:dlg_main"),
			_T("LAYOUT:fund_detail"),
		},
		{
			_T("ICON:ICON_LOGO"),
		},
		{
			_T("values:string"),
			_T("values:color"),
			_T("values:skin"),
			_T("values:font"),
		},
		{
			_T("svg:logo"),
			_T("svg:scrollbar"),
		},
	};
#else
extern struct _UIRES UIRES;
#endif//INIT_R_DATA

#ifndef _R_H_
#define _R_H_
struct _R{
	struct _name{
		 const wchar_t * about_view;
		 const wchar_t * btn_about;
		 const wchar_t * btn_about_back;
		 const wchar_t * btn_add_sample;
		 const wchar_t * btn_close;
		 const wchar_t * btn_filter;
		 const wchar_t * btn_log_clear;
		 const wchar_t * btn_max;
		 const wchar_t * btn_min;
		 const wchar_t * btn_query;
		 const wchar_t * btn_refresh;
		 const wchar_t * btn_reset;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_sim_off;
		 const wchar_t * btn_sim_on;
		 const wchar_t * btn_watch_add;
		 const wchar_t * chart_est;
		 const wchar_t * contrib_list;
		 const wchar_t * edit_code;
		 const wchar_t * edit_filter;
		 const wchar_t * fund_swipe;
		 const wchar_t * grid_est;
		 const wchar_t * grid_holdings;
		 const wchar_t * index_bar;
		 const wchar_t * kpi_cover;
		 const wchar_t * kpi_drag;
		 const wchar_t * kpi_hero;
		 const wchar_t * kpi_nav;
		 const wchar_t * kpi_pull;
		 const wchar_t * pane_change;
		 const wchar_t * pane_est;
		 const wchar_t * pane_log;
		 const wchar_t * pane_swipe;
		 const wchar_t * re_log;
		 const wchar_t * tabs;
		 const wchar_t * txt_chart_sub;
		 const wchar_t * txt_fund_chip;
		 const wchar_t * txt_market;
		 const wchar_t * txt_market_off;
		 const wchar_t * txt_status;
		 const wchar_t * txt_upd;
		 const wchar_t * txt_watch_cnt;
		 const wchar_t * view_about;
		 const wchar_t * view_empty;
		 const wchar_t * view_main;
		 const wchar_t * watch_tabs;
		 const wchar_t * workspace;
	}name;
	struct _id{
		int about_view;
		int btn_about;
		int btn_about_back;
		int btn_add_sample;
		int btn_close;
		int btn_filter;
		int btn_log_clear;
		int btn_max;
		int btn_min;
		int btn_query;
		int btn_refresh;
		int btn_reset;
		int btn_restore;
		int btn_sim_off;
		int btn_sim_on;
		int btn_watch_add;
		int chart_est;
		int contrib_list;
		int edit_code;
		int edit_filter;
		int fund_swipe;
		int grid_est;
		int grid_holdings;
		int index_bar;
		int kpi_cover;
		int kpi_drag;
		int kpi_hero;
		int kpi_nav;
		int kpi_pull;
		int pane_change;
		int pane_est;
		int pane_log;
		int pane_swipe;
		int re_log;
		int tabs;
		int txt_chart_sub;
		int txt_fund_chip;
		int txt_market;
		int txt_market_off;
		int txt_status;
		int txt_upd;
		int txt_watch_cnt;
		int view_about;
		int view_empty;
		int view_main;
		int watch_tabs;
		int workspace;
	}id;
	struct _color{
		int accent;
		int bg_0;
		int bg_1;
		int bg_2;
		int bg_3;
		int bg_4;
		int black;
		int blue;
		int down;
		int gray;
		int green;
		int line;
		int line_2;
		int red;
		int text_1;
		int text_2;
		int text_3;
		int up;
		int white;
	}color;
	struct _string{
		int status_empty;
		int status_loading;
		int status_ready;
		int tip_code;
		int title;
		int ver;
	}string;
#ifdef __cplusplus
	struct {
		enum {
			about_view = 65560,
			btn_about = 65562,
			btn_about_back = 65559,
			btn_add_sample = 65557,
			btn_close = 65542,
			btn_filter = 65573,
			btn_log_clear = 65575,
			btn_max = 65540,
			btn_min = 65539,
			btn_query = 65544,
			btn_refresh = 65546,
			btn_reset = 65574,
			btn_restore = 65541,
			btn_sim_off = 65547,
			btn_sim_on = 65548,
			btn_watch_add = 65545,
			chart_est = 65569,
			contrib_list = 65570,
			edit_code = 65543,
			edit_filter = 65572,
			fund_swipe = 65555,
			grid_est = 65580,
			grid_holdings = 65578,
			index_bar = 65550,
			kpi_cover = 65565,
			kpi_drag = 65567,
			kpi_hero = 65563,
			kpi_nav = 65564,
			kpi_pull = 65566,
			pane_change = 65577,
			pane_est = 65579,
			pane_log = 65581,
			pane_swipe = 65576,
			re_log = 65582,
			tabs = 65571,
			txt_chart_sub = 65568,
			txt_fund_chip = 65536,
			txt_market = 65537,
			txt_market_off = 65538,
			txt_status = 65561,
			txt_upd = 65549,
			txt_watch_cnt = 65554,
			view_about = 65558,
			view_empty = 65556,
			view_main = 65552,
			watch_tabs = 65553,
			workspace = 65551
		};
	}ID;
#endif//__cplusplus

};
#endif//_R_H_
#ifdef INIT_R_DATA
struct _R R={
	{
		L"about_view",
		L"btn_about",
		L"btn_about_back",
		L"btn_add_sample",
		L"btn_close",
		L"btn_filter",
		L"btn_log_clear",
		L"btn_max",
		L"btn_min",
		L"btn_query",
		L"btn_refresh",
		L"btn_reset",
		L"btn_restore",
		L"btn_sim_off",
		L"btn_sim_on",
		L"btn_watch_add",
		L"chart_est",
		L"contrib_list",
		L"edit_code",
		L"edit_filter",
		L"fund_swipe",
		L"grid_est",
		L"grid_holdings",
		L"index_bar",
		L"kpi_cover",
		L"kpi_drag",
		L"kpi_hero",
		L"kpi_nav",
		L"kpi_pull",
		L"pane_change",
		L"pane_est",
		L"pane_log",
		L"pane_swipe",
		L"re_log",
		L"tabs",
		L"txt_chart_sub",
		L"txt_fund_chip",
		L"txt_market",
		L"txt_market_off",
		L"txt_status",
		L"txt_upd",
		L"txt_watch_cnt",
		L"view_about",
		L"view_empty",
		L"view_main",
		L"watch_tabs",
		L"workspace"
	}
	,
	{
		65560,
		65562,
		65559,
		65557,
		65542,
		65573,
		65575,
		65540,
		65539,
		65544,
		65546,
		65574,
		65541,
		65547,
		65548,
		65545,
		65569,
		65570,
		65543,
		65572,
		65555,
		65580,
		65578,
		65550,
		65565,
		65567,
		65563,
		65564,
		65566,
		65577,
		65579,
		65581,
		65576,
		65582,
		65571,
		65568,
		65536,
		65537,
		65538,
		65561,
		65549,
		65554,
		65558,
		65556,
		65552,
		65553,
		65551
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
		18
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
	
};
#else
extern struct _R R;
#endif//INIT_R_DATA
