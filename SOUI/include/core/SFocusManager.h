/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SFocusManager.h
 * @brief      Focus management module for DUI windows
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * @details    Manages focus for DUI windows within the SOUI framework.
 */

#ifndef __SFOCUSMANAGER__H__
#define __SFOCUSMANAGER__H__

#include <core/SWnd.h>
#include <interface/SAccelerator-i.h>

SNSBEGIN

/**
 * @class      FocusSearch
 * @brief      Implements the algorithm to find the next view to focus.
 *
 * @details    This class is responsible for finding the next focusable view within a given
 *             view hierarchy. It can cycle through the views and handle focus traversal.
 */
class FocusSearch {
  public:
    /**
     * @brief    Constructor
     * @param    root   Root of the view hierarchy to traverse
     * @param    cycle  TRUE if focus should cycle back to the first view when reaching the end
     *
     * @details  Initializes the FocusSearch object with the specified root view and cycle behavior.
     */
    FocusSearch(SWindow *root, bool cycle);

    /**
     * @brief    Destructor
     */
    virtual ~FocusSearch()
    {
    }

    /**
     * @brief    Finds the next focusable view
     * @param    starting_view      Starting view for the search
     * @param    reverse            TRUE to find the previous view, FALSE for the next view
     * @param    check_starting_view  TRUE if the starting view may obtain focus
     * @return   Next focusable view or NULL if none found
     *
     * @details  Finds the next focusable view based on the specified parameters.
     *           If a FocusTraversable is found, it sets the FocusTraversable and returns NULL.
     */
    SWindow *FindNextFocusableView(SWindow *starting_view, bool reverse, bool check_starting_view);

  private:
    /**
     * @brief    Checks if a view is a focusable candidate
     * @param    v            View to check
     * @param    pGroupOwner  Group owner to skip
     * @return   TRUE if the view is focusable and not part of the specified group, otherwise FALSE
     *
     * @details  Determines if a view is a valid focusable candidate, excluding views in the specified group.
     */
    bool IsViewFocusableCandidate(SWindow *v, SWindow *pGroupOwner);

    /**
     * @brief    Checks if a view is focusable
     * @param    view  View to check
     * @return   TRUE if the view is focusable, otherwise FALSE
     *
     * @details  Determines if a view is focusable by checking its focusability.
     */
    bool IsFocusable(const SWindow *view) const;

    /**
     * @brief    Finds the next focusable view
     * @param    starting_view      Starting view for the search
     * @param    check_starting_view  TRUE if the starting view may obtain focus
     * @param    can_go_up          TRUE if traversal can go up the hierarchy
     * @param    can_go_down        TRUE if traversal can go down the hierarchy
     * @param    pSkipGroupOwner    Group owner to skip
     * @return   Next focusable view or NULL if none found
     *
     * @details  Implements the logic to find the next focusable view based on the specified parameters.
     */
    SWindow *FindNextFocusableViewImpl(SWindow *starting_view, bool check_starting_view, bool can_go_up, bool can_go_down, SWindow *pSkipGroupOwner);

    /**
     * @brief    Finds the previous focusable view
     * @param    starting_view      Starting view for the search
     * @param    check_starting_view  TRUE if the starting view may obtain focus
     * @param    can_go_up          TRUE if traversal can go up the hierarchy
     * @param    can_go_down        TRUE if traversal can go down the hierarchy
     * @param    pSkipGroupOwner    Group owner to skip
     * @return   Previous focusable view or NULL if none found
     *
     * @details  Implements the logic to find the previous focusable view based on the specified parameters.
     */
    SWindow *FindPreviousFocusableViewImpl(SWindow *starting_view, bool check_starting_view, bool can_go_up, bool can_go_down, SWindow *pSkipGroupOwner);

    SWindow *root_; /**< Root of the view hierarchy to traverse. */
    bool cycle_;    /**< TRUE if focus should cycle back to the first view when reaching the end. */
};

/**
 * @class      SFocusManager
 * @brief      Focus management object for DUI windows
 *
 * @details    Manages the focus for DUI windows, handling focus traversal and keyboard accelerators.
 */
class SOUI_EXP SFocusManager : public IAcceleratorMgr {
  public:
    /**
     * @enum       FocusChangeReason
     * @brief      Reason for focus change
     *
     * @details    Enumerates the reasons why the focus might change.
     */
    enum FocusChangeReason
    {
        kReasonDirectFocusChange = 0, /**< Focus changed due to a direct action like a click or shortcut. */
        kReasonFocusTraversal,        /**< Focus changed due to keyboard traversal (e.g., Tab, Shift+Tab). */
        kReasonFocusRestore           /**< Focus changed due to restoring the focus. */
    };

    /**
     * @brief    Constructor
     */
    SFocusManager();

    /**
     * @brief    Destructor
     */
    ~SFocusManager(void);

    /**
     * @brief    Sets the owner window
     * @param    pOwner  Pointer to the owner window
     *
     * @details  Sets the owner window for the focus manager.
     */
    void SetOwner(SWindow *pOwner);

    /**
     * @brief    Checks if a key is a tab traversal key
     * @param    vKey  Virtual key code
     * @return   TRUE if the key is a tab traversal key, otherwise FALSE
     *
     * @details  Determines if the specified virtual key code is a tab traversal key.
     */
    BOOL IsTabTraversalKey(UINT vKey);

    /**
     * @brief    Handles key down events
     * @param    vKey  Virtual key code
     * @return   TRUE if the key event is handled, otherwise FALSE
     *
     * @details  Handles key down events and performs focus traversal if necessary.
     */
    BOOL OnKeyDown(UINT vKey);

    /**
     * @brief    Advances the focus
     * @param    reverse  TRUE to advance backward, FALSE to advance forward
     *
     * @details  Advances the focus to the next or previous focusable view based on the specified direction.
     */
    void AdvanceFocus(bool reverse);

    /**
     * @brief    Sets the focused window with a reason
     * @param    swnd    Handle to the window to focus
     * @param    reason  Reason for the focus change
     *
     * @details  Sets the focused window and records the reason for the focus change.
     */
    void SetFocusedHwndWithReason(SWND swnd, FocusChangeReason reason);

    /**
     * @brief    Sets the focused window
     * @param    swnd  Handle to the window to focus
     *
     * @details  Sets the focused window without specifying a reason.
     */
    void SetFocusedHwnd(SWND swnd);

    /**
     * @brief    Clears the focused window
     *
     * @details  Clears the focused window and sets the native focus to the top root view.
     */
    void ClearFocus();

    /**
     * @brief    Gets the focused window
     * @return   Handle to the focused window
     *
     * @details  Returns the handle to the currently focused window.
     */
    SWND GetFocusedHwnd() const;

    /**
     * @brief    Stores the focused view
     *
     * @details  Stores the currently focused view for later restoration.
     */
    void StoreFocusedView();

    /**
     * @brief    Restores the focused view
     *
     * @details  Restores the focused view from the stored state.
     */
    void RestoreFocusedView();

  protected:
    // IAcceleratorMgr

    /**
     * @brief    Registers a keyboard accelerator for a target
     * @param    pAcc    Pointer to the accelerator
     * @param    target  Pointer to the accelerator target
     *
     * @details  Registers a keyboard accelerator for the specified target.
     */
    STDMETHOD_(void, RegisterAccelerator)
    (THIS_ const IAccelerator *pAcc, IAcceleratorTarget *target) OVERRIDE;

    /**
     * @brief    Unregisters a keyboard accelerator for a target
     * @param    pAcc    Pointer to the accelerator
     * @param    target  Pointer to the accelerator target
     *
     * @details  Unregisters a keyboard accelerator for the specified target.
     */
    STDMETHOD_(void, UnregisterAccelerator)
    (THIS_ const IAccelerator *pAcc, IAcceleratorTarget *target) OVERRIDE;

    /**
     * @brief    Unregisters all keyboard accelerators for a target
     * @param    target  Pointer to the accelerator target
     *
     * @details  Unregisters all keyboard accelerators for the specified target.
     */
    STDMETHOD_(void, UnregisterAccelerators)(THIS_ IAcceleratorTarget *target) OVERRIDE;

  private:
    /**
     * @brief    Processes a keyboard accelerator
     * @param    pAcc  Pointer to the accelerator
     * @return   TRUE if the accelerator was activated, otherwise FALSE
     *
     * @details  Activates the target associated with the specified accelerator.
     */
    bool ProcessAccelerator(const IAccelerator *pAcc);

    /**
     * @brief    Gets the next focusable view
     * @param    pWndStarting       Starting view for the search
     * @param    bReverse           TRUE to find the previous view, FALSE for the next view
     * @param    bLoop              TRUE if focus should cycle back to the first view when reaching the end
     * @return   Next focusable view or NULL if none found
     *
     * @details  Finds the next focusable view based on the specified parameters.
     */
    SWindow *GetNextFocusableView(SWindow *pWndStarting, bool bReverse, bool bLoop);

    /**
     * @brief    Validates the focused view
     *
     * @details  Validates the focused view and clears it if the window is no longer attached to the hierarchy.
     */
    void ValidateFocusedView();

  private:
    SWND focused_view_;                     /**< Handle to the currently focused window. */
    SWND focused_backup_;                   /**< Backup handle to the focused window for restoration. */
    FocusChangeReason focus_change_reason_; /**< Reason for the last focus change. */
    SWindow *m_pOwner;                      /**< Pointer to the owner window. */

    typedef SList<IAcceleratorTarget *> AcceleratorTargetList; /**< List of accelerator targets. */
    typedef SMap<DWORD, AcceleratorTargetList> AcceleratorMap; /**< Map of accelerators to targets. */
    AcceleratorMap accelerators_;                              /**< Map of registered accelerators. */
};

SNSEND

#endif // __SFOCUSMANAGER__H__