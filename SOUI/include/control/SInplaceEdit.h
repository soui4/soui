/**
 * @file       SInplaceEdit.h
 * @brief      Shared in-place label editor used by STreeCtrl / SListCtrl.
 * @version    1.0
 *
 * @copyright  Copyright (C) 2012-2050 SOUI Team
 *             All rights reserved.
 */

#ifndef __SINPLACEEDIT__H__
#define __SINPLACEEDIT__H__

#include <control/SEdit.h>

SNSBEGIN

/**
 * @class IInplaceEditHost
 * @brief Callback interface implemented by the control hosting an in-place edit.
 */
class SOUI_EXP IInplaceEditHost {
  public:
    /**
     * @brief Called when the in-place edit session finishes.
     * @param bSave TRUE to commit the edited text, FALSE to discard it.
     * @note The edited text is read by the host from the edit window
     *       (GetWindowText) before it destroys the editor.
     */
    virtual void OnInplaceEditEnd(BOOL bSave) = 0;

  protected:
    ~IInplaceEditHost()
    {
    }
};

/**
 * @class SInplaceEdit
 * @brief A single-line edit covering an item label while it is being edited.
 *
 * Created dynamically as a child of the host control (InsertIChild +
 * InitFromXml(NULL), the SGridCtrl inplace-edit pattern). RETURN / TAB hand
 * focus back to the host and commit, ESC discards, losing focus commits.
 * The editor's life cycle is managed by the host: it owns the initial
 * reference created with the object, and Destroy() releases it.
 */
class SOUI_EXP SInplaceEdit : public SEdit {
  public:
    /**
     * @brief Constructor.
     * @param pHost Host control notified when the session ends.
     */
    explicit SInplaceEdit(IInplaceEditHost *pHost);

    /**
     * @brief Apply the default inplace edit look (opaque white background,
     *        black text, word-select mode), matching SGridCtrl.
     */
    void ApplyDefaultStyle();

  protected:
    /**
     * @brief Intercept the keys that end the session and the focus change
     *        that commits/discards the edit.
     */
    virtual BOOL ProcessSwndMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult) OVERRIDE;

    IInplaceEditHost *m_pHost;
    BOOL m_bUpdateData;
};

SNSEND

#endif /**< __SINPLACEEDIT__H__ */
