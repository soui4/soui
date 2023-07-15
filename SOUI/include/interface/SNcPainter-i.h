#ifndef __SNCPAINTER_I__H__
#define __SNCPAINTER_I__H__
#include <interface/SWindow-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE INcPainter
DECLARE_INTERFACE_(INcPainter, IObject)
{
    DEF_OBJ_BASE(INcPainter, Undef)
#include <interface/SobjectApi.h>

    //--------------------------------------------------------------------------------

    /**
     * @brief 获取绘制非客户区的根窗口
     * @return IWindow *--root
     */
    STDMETHOD_(IWindow *, GetRoot)(THIS) PURE;
};

SNSEND
#endif // __SNCPAINTER_I__H__