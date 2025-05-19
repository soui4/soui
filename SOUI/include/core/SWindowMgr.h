/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SWindowMgr.h
 * @brief      SOUI系统中的DUI窗口管理模块
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Description: This file defines the SWindowMgr class, which manages DUI windows in the SOUI system.
 */

#ifndef __SWINDOWMGR__H__
#define __SWINDOWMGR__H__

#include <core/SSingletonMap.h>
#include <core/SDefine.h>
#include <helper/SCriticalSection.h>

SNSBEGIN

class SWindow;

/**
 * @class SWindowMgr
 * @brief Manages DUI windows in the SOUI system.
 *
 * This class is responsible for managing the creation, retrieval, and destruction of DUI windows.
 * It uses a singleton pattern to ensure a single instance of the manager and provides methods
 * to interact with the windows.
 */
class SOUI_EXP SWindowMgr : public SSingletonMap<SWindowMgr, SWindow *, SWND> {
    SINGLETON2_TYPE(SINGLETON_SWNDMGR)

  public:
    /**
     * @brief Invalid window handle constant.
     */
    enum
    {
        SWND_INVALID = 0
    };

    /**
     * @brief Constructor.
     */
    SWindowMgr();

    /**
     * @brief Destructor.
     */
    ~SWindowMgr();

    /**
     * @brief Retrieves the SWindow pointer from a given handle.
     * @param swnd Handle of the window to retrieve.
     * @return Pointer to the SWindow object, or nullptr if the handle is invalid.
     */
    static SWindow *GetWindow(SWND swnd);

    /**
     * @brief Checks if a given handle is a valid window handle.
     * @param swnd Handle to check.
     * @return TRUE if the handle is valid, FALSE otherwise.
     */
    static bool IsWindow(SWND swnd);

    /**
     * @brief Assigns a handle to a new SWindow.
     * @param pWnd Pointer to the SWindow to assign a handle to.
     * @return Handle assigned to the SWindow, or SWND_INVALID if assignment fails.
     */
    static SWND NewWindow(SWindow *pWnd);

    /**
     * @brief Destroys a window with the specified handle.
     * @param swnd Handle of the window to destroy.
     * @return TRUE if the window is successfully destroyed, FALSE otherwise.
     */
    static BOOL DestroyWindow(SWND swnd);

  protected:
    SCriticalSection m_lockWndMap; // Critical section for thread-safe access to the window map.
    SWND m_hNextWnd;               // Next available window handle.
};

SNSEND
#endif // __SWINDOWMGR__H__