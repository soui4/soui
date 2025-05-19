/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SAccelerator.h
 * @brief      Accelerator management module
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * @details    SOUI accelerator management module
 */

#ifndef __SACCELERATOR__H__
#define __SACCELERATOR__H__

#include <interface/SAccelerator-i.h>

SNSBEGIN

/**
 * @class      SAccelerator
 * @brief      Accelerator key mapping
 *
 * @details    This class manages accelerator keys, allowing for the creation and manipulation of
 *             accelerator key mappings within the SOUI framework.
 */
class SOUI_EXP SAccelerator : public IAccelerator {
  public:
    /**
     * @brief    Constructor
     * @param    dwAccel  Accelerator key value
     *
     * @details  Initializes the accelerator with a given accelerator key value.
     */
    SAccelerator(DWORD dwAccel);

    /**
     * @brief    Constructor
     * @param    vKey     Key value
     * @param    bCtrl    TRUE if combined with Ctrl
     * @param    bAlt     TRUE if combined with Alt
     * @param    bShift   TRUE if combined with Shift
     *
     * @details  Initializes the accelerator with a key value and modifier keys (Ctrl, Alt, Shift).
     */
    SAccelerator(WORD vKey = 0, bool bCtrl = false, bool bAlt = false, bool bShift = false);

    /**
     * @brief    Destructor
     *
     * @details  Cleans up the accelerator object.
     */
    ~SAccelerator(void);

    /**
     * @brief    Gets the modifier keys
     * @return   Modifier keys as a WORD
     *
     * @details  Returns the modifier keys (Ctrl, Alt, Shift) as a WORD.
     */
    STDMETHOD_(WORD, GetModifier)(THIS) SCONST OVERRIDE;

    /**
     * @brief    Gets the key value
     * @return   Key value as a WORD
     *
     * @details  Returns the key value associated with the accelerator.
     */
    STDMETHOD_(WORD, GetKey)(THIS) SCONST OVERRIDE;

    /**
     * @brief    Gets the accelerator key value
     * @return   Accelerator key value as a DWORD
     *
     * @details  Returns the complete accelerator key value, including modifiers and key.
     */
    STDMETHOD_(DWORD, GetAcc)(THIS) SCONST OVERRIDE;

    /**
     * @brief    Formats the accelerator key as a string
     * @return   SStringT - Formatted string representation of the accelerator key
     *
     * @details  Converts the accelerator key into a human-readable string format.
     */
    SStringT FormatHotkey();

    /**
     * @brief    Converts a string to a virtual key code
     * @param    pszKey   String representation of the key
     * @return   Virtual key code as a WORD
     *
     * @details  Converts a string representation of a key (e.g., "Ctrl+C") to its corresponding virtual key code.
     */
    static WORD VkFromString(LPCTSTR pszKey);

    /**
     * @brief    Converts a virtual key code to its string representation
     * @param    vk       Virtual key code
     * @return   SStringT - String representation of the key
     *
     * @details  Converts a virtual key code to its human-readable string format.
     */
    static SStringT GetKeyName(WORD vk);

    /**
     * @brief    Formats an accelerator key as a string
     * @param    dwAccel  Accelerator key value
     * @return   SStringT - Formatted string representation of the accelerator key
     *
     * @details  Converts an accelerator key value into a human-readable string format.
     */
    static SStringT FormatAccelKey(DWORD dwAccel);

    /**
     * @brief    Translates a string to an accelerator key value
     * @param    pszKeyName String representation of the accelerator key
     * @return   DWORD - Accelerator key value
     *
     * @details  Converts a string representation of an accelerator key (e0.g., "Ctrl+C") to its corresponding accelerator key value.
     */
    static DWORD TranslateAccelKey(LPCTSTR pszKeyName);

  protected:
    WORD m_wModifier; /**< Modifier keys (Ctrl, Alt, Shift) as a WORD. */
    WORD m_wVK;       /**< Key value as a WORD. */
};

SNSEND
#endif // __SACCELERATOR__H__