#ifndef _SDC_H_
#define _SDC_H_
#include <platform.h>
#include <cairo/cairo.h>
#include <region.h>

typedef struct _SDC {
	HWND      hwnd;
	cairo_t* cairo;
	HBITMAP   bmp;
	COLORREF  crText;
	HGDIOBJ   pen;
	HGDIOBJ   brush;
	HGDIOBJ   hfont;
	HGDIOBJ   rgn;

	int       bkMode;
	int       nSave;
	_SDC(HWND _hwnd, HBITMAP _bmp) :hwnd(_hwnd), bmp(_bmp), nSave(0), crText(RGBA(0, 0, 0, 0xff))
		, pen(GetStockObject(BLACK_PEN))
		, brush(GetStockObject(WHITE_BRUSH))
		, hfont(GetStockObject(SYSTEM_FONT))
		, bkMode(0)
		, rgn(0)
	{
		assert(bmp);
		cairo_surface_t* surface = (cairo_surface_t*)GetGdiObjPtr(bmp);
		cairo = cairo_create(surface);
		rgn = CreateRectRgn(0, 0, 0, 0);
	}

	~_SDC() {
		cairo_destroy(cairo);
		DeleteObject(bmp);
		DeleteObject(rgn);
	}

	int SaveState() {
		if (!cairo)
			return 0;
		cairo_save(cairo);
		return nSave++;
	}

	BOOL RestoreState(int nState) {
		if (nState >= nSave)
			return FALSE;
		if (nState < 0)
		{
			cairo_restore(cairo);
			--nSave;
		}
		else
		{
			while (nSave > nState) {
				cairo_restore(cairo);
				--nSave;
			}
		}
		return TRUE;
	}
} *HDC;

#endif//_SDC_H_