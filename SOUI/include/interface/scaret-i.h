#ifndef __SCARET_I__H__
#define __SCARET_I__H__
#include <interface/sobject-i.h>

SNSBEGIN

typedef struct IRenderTarget IRenderTarget;

#undef INTERFACE
#define INTERFACE ICaret
DECLARE_INTERFACE_(ICaret, IObject)
{
    DEF_OBJ_BASE(ICaret, Undef)
#include <interface/SobjectApi.h>
    //-----------------------------------------------------------------------------------

    /**
     * @brief Initialize the cursor
     * @param hBmp -- cursor bitmap, may be NULL
     * @param nWid -- cursor width
     * @param nHei -- cursor height
     * @return BOOL TRUE on success
     */
    STDMETHOD_(BOOL, Init)(THIS_ HBITMAP hBmp, int nWid, int nHei) PURE;

    /**
     * @brief Set the cursor position
     * @param x -- X
     * @param y -- Y
     * @return
     */
    STDMETHOD_(void, SetPosition)(THIS_ int x, int y) PURE;

    /**
     * @brief Set the cursor visibility state
     * @param bVisible -- visibility state
     * @param owner -- the cursor's host
     * @return BOOL TRUE on success
     */
    STDMETHOD_(BOOL, SetVisible)(THIS_ BOOL bVisible, SWND owner) PURE;

    /**
     * @brief Get the cursor visibility state
     * @return BOOL TRUE if visible
     */
    STDMETHOD_(BOOL, IsVisible)(CTHIS) SCONST PURE;

    /**
     * @brief Draw the cursor
     * @param pRT -- the RenderTarget for drawing the cursor
     * @return
     */
    STDMETHOD_(void, Draw)(THIS_ IRenderTarget * pRT) PURE;

    /**
     * @brief Get the cursor display rectangle
     * @return RECT
     */
    STDMETHOD_(RECT, GetRect)(CTHIS) SCONST PURE;
};

SNSEND
#endif /**< __SCARET_I__H__ */