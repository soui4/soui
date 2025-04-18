/**
 * @file       SSkinobj-i.h
 * @brief      Interface for Skin Objects
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/01
 *
 * @details    This file defines the interface for skin objects used in rendering.
 */

#ifndef __SSKINOBJ_I__H__
#define __SSKINOBJ_I__H__

#include <interface/obj-ref-i.h>
#include <interface/SRender-i.h>

SNSBEGIN

/**
 * @struct ISkinObj
 * @brief Interface for Skin Objects
 *
 * @details This interface defines the methods for skin objects, which are used for rendering
 * graphical elements with various states and properties such as transparency and scaling.
 */
#undef INTERFACE
#define INTERFACE ISkinObj
DECLARE_INTERFACE_(ISkinObj, IObject)
{
    DEF_OBJ_BASE(ISkinObj, Skin)
#include <interface/SobjectApi.h>

    /**
     * @brief Draws the skin object to the specified render target with a given state and transparency.
     *
     * @param pRT     Pointer to the render target where the skin will be drawn.
     * @param rcDraw  Rectangle defining the drawing area.
     * @param dwState State of the skin to draw.
     * @param byAlpha Transparency level (0-255).
     */
    STDMETHOD_(void, DrawByState2)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha) SCONST PURE;

    /**
     * @brief Draws the skin object to the specified render target with a given state.
     *
     * @param pRT     Pointer to the render target where the skin will be drawn.
     * @param rcDraw  Rectangle defining the drawing area.
     * @param dwState State of the skin to draw.
     *
     * @details Uses the alpha value obtained from the skin object's GetAlpha method.
     */
    STDMETHOD_(void, DrawByState)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, DWORD dwState) SCONST PURE;

    /**
     * @brief Draws the skin object to the specified render target with a given index and transparency.
     *
     * @param pRT     Pointer to the render target where the skin will be drawn.
     * @param rcDraw  Rectangle defining the drawing area.
     * @param iState  Index of the skin state to draw.
     * @param byAlpha Transparency level (0-255).
     */
    STDMETHOD_(void, DrawByIndex2)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) SCONST PURE;

    /**
     * @brief Draws the skin object to the specified render target with a given index.
     *
     * @param pRT     Pointer to the render target where the skin will be drawn.
     * @param rcDraw  Rectangle defining the drawing area.
     * @param iState  Index of the skin state to draw.
     *
     * @details Uses the alpha value obtained from the skin object's GetAlpha method.
     */
    STDMETHOD_(void, DrawByIndex)
    (CTHIS_ IRenderTarget * pRT, LPCRECT rcDraw, int iState) SCONST PURE;

    /**
     * @brief Retrieves the default size of the skin object.
     *
     * @return SIZE The default size of the skin object.
     *
     * @details Derived classes should implement this method based on the characteristics of the skin.
     */
    STDMETHOD_(SIZE, GetSkinSize)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the number of states contained in the skin object.
     *
     * @return int The number of states.
     *
     * @details Default is 1.
     */
    STDMETHOD_(int, GetStates)(CTHIS) SCONST PURE;

    /**
     * @brief Retrieves the transparency of the skin object.
     *
     * @return BYTE The transparency level (0-255).
     */
    STDMETHOD_(BYTE, GetAlpha)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the transparency of the skin object.
     *
     * @param byAlpha The transparency level (0-255).
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE byAlpha) PURE;

    /**
     * @brief Adjusts the hue of the skin object based on a reference color.
     *
     * @param cr The reference color.
     *
     * @details Default implementation does nothing.
     */
    STDMETHOD_(void, OnColorize)(THIS_ COLORREF cr) PURE;

    /**
     * @brief Retrieves the scaling factor of the skin object.
     *
     * @return int The scaling factor.
     */
    STDMETHOD_(int, GetScale)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the scaling factor of the skin object.
     *
     * @param scale The scaling factor.
     *
     * @details This is an internal interface and should not be called in the business layer.
     */
    STDMETHOD_(void, SetScale)(THIS_ int scale) PURE;

    /**
     * @brief Scales the skin object by a specified factor.
     *
     * @param nScale The scaling factor.
     * @return ISkinObj* Pointer to the scaled skin object.
     */
    STDMETHOD_(ISkinObj *, Scale)(THIS_ int nScale) PURE;
};

SNSEND
#endif // __SSKINOBJ_I__H__