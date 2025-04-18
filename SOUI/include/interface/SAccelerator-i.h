#ifndef __SACCELERATOR_I__H__
#define __SACCELERATOR_I__H__
#include <utilities-def.h>

SNSBEGIN

/**
 * @brief Modifier keys for accelerators.
 */
enum
{
    Mod_None = 0,  /**< No modifier key. */
    Mod_Alt = 1,   /**< Alt modifier key. */
    Mod_Ctrl = 2,  /**< Ctrl modifier key. */
    Mod_Shift = 4, /**< Shift modifier key. */
};

/**
 * @interface IAccelerator
 * @brief Interface for an accelerator key.
 */
#undef INTERFACE
#define INTERFACE IAccelerator
DECLARE_INTERFACE(IAccelerator)
{
    /**
     * @brief Get the modifier keys of the accelerator.
     * @return WORD representing the modifier keys.
     */
    STDMETHOD_(WORD, GetModifier)(CTHIS) SCONST PURE;

    /**
     * @brief Get the main key of the accelerator.
     * @return WORD representing the main key.
     */
    STDMETHOD_(WORD, GetKey)(CTHIS) SCONST PURE;

    /**
     * @brief Get the accelerator key combination.
     * @return DWORD representing the accelerator key combination.
     */
    STDMETHOD_(DWORD, GetAcc)(CTHIS) SCONST PURE;
};

/**
 * @interface IAcceleratorTarget
 * @brief Interface for handling accelerator key presses.
 *
 * Classes that want to register keyboard accelerators need to implement this interface.
 */
#undef INTERFACE
#define INTERFACE IAcceleratorTarget
DECLARE_INTERFACE(IAcceleratorTarget)
{
    /**
     * @brief Handle an accelerator key press.
     * @param acc Pointer to the accelerator key that was pressed.
     * @return TRUE if the accelerator key was handled, FALSE otherwise.
     */
    STDMETHOD_(BOOL, OnAcceleratorPressed)(THIS_ const IAccelerator *acc) PURE;
};

/**
 * @interface IAcceleratorMgr
 * @brief Interface for managing accelerator keys.
 */
#undef INTERFACE
#define INTERFACE IAcceleratorMgr
DECLARE_INTERFACE(IAcceleratorMgr)
{
    /**
     * @brief Register a keyboard accelerator for the specified target.
     *
     * If multiple targets are registered for an accelerator, a target registered later has higher priority.
     * Note that we are currently limited to accelerators that are either:
     * - a key combination including Ctrl or Alt
     * - the escape key
     * - the enter key
     * - any F key (F1, F2, F3 ...)
     * - any browser-specific keys (as available on special keyboards)
     *
     * @param pAcc Pointer to the accelerator key to register.
     * @param target Pointer to the target that will handle the accelerator key press.
     */
    STDMETHOD_(void, RegisterAccelerator)(THIS_ const IAccelerator *pAcc, IAcceleratorTarget *target) PURE;

    /**
     * @brief Unregister a keyboard accelerator for the specified target.
     * @param pAcc Pointer to the accelerator key to unregister.
     * @param target Pointer to the target that was handling the accelerator key press.
     */
    STDMETHOD_(void, UnregisterAccelerator)(THIS_ const IAccelerator *pAcc, IAcceleratorTarget *target) PURE;

    /**
     * @brief Unregister all keyboard accelerators for the specified target.
     * @param target Pointer to the target whose accelerators should be unregistered.
     */
    STDMETHOD_(void, UnregisterAccelerators)(THIS_ IAcceleratorTarget * target) PURE;
};

SNSEND
#endif // __SACCELERATOR_I__H__