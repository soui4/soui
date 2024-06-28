#ifndef _SDC_H_
#define _SDC_H_
#include <platform.h>
#include <cairo/cairo.h>

typedef struct _SDC{
    HWND      hwnd;
    cairo_t * cairo;
    HBITMAP   bmp;
    COLORREF  crText;
    HGDIOBJ   pen;
    HGDIOBJ   brush;
    int       nSave;
    _SDC():cairo(nullptr),bmp(nullptr),nSave(0),crText(RGB(0,0,0)|0xff000000),pen(nullptr),brush(nullptr){}

    ~_SDC(){
        if(cairo){
            cairo_destroy(cairo);
        }
        DeleteObject(pen);
        DeleteObject(brush);
        DeleteObject(bmp);
    }

    int SaveState(){
        if(!cairo)
            return 0;
        cairo_save(cairo);
        return ++nSave;
    }

    BOOL RestoreState(int nState){
        if(nState>=nSave)
            return FALSE;
        if(nState<0)
        {
            cairo_restore(cairo);
            --nSave;
        }else 
        {
            while(nSave>nState){
                cairo_restore(cairo);
                --nSave;
            }
        }
        return TRUE;
    }
} * HDC;

#endif//_SDC_H_