#ifndef __SNATIVEWND__H__
#define __SNATIVEWND__H__
#include <interface/SNativeWnd-i.h>
#include <helper/SCriticalSection.h>
#include <helper/obj-ref-impl.hpp>
#include <windows.h>
#include <soui_exp.h>

SNSBEGIN

class SOUI_EXP SNativeWndHelper {
  public:
    HANDLE GetHeap()
    {
        return m_hHeap;
    }

    void LockSharePtr(void *p);
    void UnlockSharePtr();
    void *GetSharePtr()
    {
        return m_sharePtr;
    }

    HINSTANCE GetAppInstance()
    {
        return m_hInst;
    }
    ATOM GetSimpleWndAtom()
    {
        return m_atom;
    }

    BOOL Init(HINSTANCE hInst, LPCTSTR pszClassName, BOOL bImeApp);

  public:
    static SNativeWndHelper *instance()
    {
        static SNativeWndHelper _this;
        return &_this;
    }

  private:
    SNativeWndHelper();
    ~SNativeWndHelper();

    HANDLE m_hHeap;
    SCriticalSection m_cs;
    void *m_sharePtr;

    ATOM m_atom;
    HINSTANCE m_hInst;
};

struct tagThunk;
class SOUI_EXP SNativeWnd : public TObjRefImpl<INativeWnd> {
  public:
    SNativeWnd();
    virtual ~SNativeWnd(void);

    static ATOM RegisterSimpleWnd(HINSTANCE hInst, LPCTSTR pszSimpleWndName, BOOL bImeWnd);
    static void InitWndClass(HINSTANCE hInst, LPCTSTR pszSimpleWndName, BOOL bImeWnd);

    STDMETHOD_(int, GetID)(THIS) SCONST
    {
        return GetDlgCtrlID();
    }
    STDMETHOD_(void, SetID)(THIS_ int nID)
    {
        SetWindowLongPtr(GWL_ID, nID);
    }

    STDMETHOD_(HWND, CreateNative)
    (THIS_ LPCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, int nID DEF_VAL(0), LPVOID lpParam DEF_VAL(0)) OVERRIDE;

    STDMETHOD_(HWND, GetHwnd)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, SubclassWindow)(THIS_ HWND hWnd) OVERRIDE;

    STDMETHOD_(HWND, UnsubclassWindow)(THIS_ BOOL bForce DEF_VAL(FALSE)) OVERRIDE;

    STDMETHOD_(const MSG *, GetCurrentMessage)(THIS) SCONST OVERRIDE;

    STDMETHOD_(int, GetDlgCtrlID)(THIS) SCONST OVERRIDE;

    STDMETHOD_(DWORD, GetStyle)(THIS) SCONST OVERRIDE;

    STDMETHOD_(DWORD, GetExStyle)(THIS) SCONST OVERRIDE;

    STDMETHOD_(LONG_PTR, GetWindowLongPtr)(THIS_ int nIndex) SCONST OVERRIDE;

    STDMETHOD_(LONG_PTR, SetWindowLongPtr)(THIS_ int nIndex, LONG_PTR dwNewLong) OVERRIDE;

    STDMETHOD_(HWND, GetParent)(THIS) OVERRIDE;

    STDMETHOD_(HWND, SetParent)(THIS_ HWND hWndNewParent) OVERRIDE;

    STDMETHOD_(BOOL, IsWindowEnabled)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, ModifyStyle)
    (THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags DEF_VAL(0)) OVERRIDE;

    STDMETHOD_(BOOL, ModifyStyleEx)
    (THIS_ DWORD dwRemove, DWORD dwAdd, UINT nFlags DEF_VAL(0)) OVERRIDE;

    STDMETHOD_(BOOL, SetWindowPos)
    (THIS_ HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags) OVERRIDE;

    STDMETHOD_(BOOL, CenterWindow)(THIS_ HWND hWndCenter DEF_VAL(0)) OVERRIDE;

    STDMETHOD_(BOOL, DestroyWindow)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, IsWindow)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, Invalidate)(THIS_ BOOL bErase DEF_VAL(TRUE)) OVERRIDE;

    STDMETHOD_(BOOL, InvalidateRect)(THIS_ LPCRECT lpRect, BOOL bErase DEF_VAL(TRUE)) OVERRIDE;

    STDMETHOD_(BOOL, GetWindowRect)(THIS_ LPRECT lpRect) SCONST OVERRIDE;

    STDMETHOD_(BOOL, GetClientRect)(THIS_ LPRECT lpRect) SCONST OVERRIDE;

    STDMETHOD_(BOOL, ClientToScreen)(THIS_ LPPOINT lpPoint) SCONST OVERRIDE;

    STDMETHOD_(BOOL, ClientToScreen2)(THIS_ LPRECT lpRect) SCONST OVERRIDE;

    STDMETHOD_(BOOL, ScreenToClient)(THIS_ LPPOINT lpPoint) SCONST OVERRIDE;

    STDMETHOD_(BOOL, ScreenToClient2)(THIS_ LPRECT lpRect) SCONST OVERRIDE;

    STDMETHOD_(int, MapWindowPoints)
    (THIS_ HWND hWndTo, LPPOINT lpPoint, UINT nCount) SCONST OVERRIDE;

    STDMETHOD_(int, MapWindowRect)(THIS_ HWND hWndTo, LPRECT lpRect) SCONST OVERRIDE;

    STDMETHOD_(UINT_PTR, SetTimer)
    (THIS_ UINT_PTR nIDEvent, UINT nElapse, void(CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD) DEF_VAL(NULL)) OVERRIDE;

    STDMETHOD_(BOOL, KillTimer)(THIS_ UINT_PTR nIDEvent) OVERRIDE;

    STDMETHOD_(HDC, GetDC)(THIS) OVERRIDE;

    STDMETHOD_(HDC, GetWindowDC)(THIS) OVERRIDE;

    STDMETHOD_(int, ReleaseDC)(THIS_ HDC hDC) OVERRIDE;

    STDMETHOD_(BOOL, CreateCaret)(THIS_ HBITMAP hBitmap, int nWidth, int nHeight) OVERRIDE;

    STDMETHOD_(BOOL, HideCaret)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, ShowCaret)(THIS) OVERRIDE;

    STDMETHOD_(HWND, GetCapture)(THIS) OVERRIDE;

    STDMETHOD_(HWND, SetCapture)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, ReleaseCapture)(THIS) OVERRIDE;

    STDMETHOD_(HWND, SetFocus)(THIS) OVERRIDE;

    STDMETHOD_(LRESULT, SendMessage)
    (THIS_ UINT message, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) OVERRIDE;

    STDMETHOD_(BOOL, PostMessage)
    (THIS_ UINT message, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) OVERRIDE;

    STDMETHOD_(BOOL, SendNotifyMessage)
    (THIS_ UINT message, WPARAM wParam DEF_VAL(0), LPARAM lParam DEF_VAL(0)) OVERRIDE;

    STDMETHOD_(BOOL, SetWindowText)(THIS_ LPCTSTR lpszString) OVERRIDE;

    STDMETHOD_(int, GetWindowText)(THIS_ LPTSTR lpszStringBuf, int nMaxCount) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsIconic)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsZoomed)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, IsWindowVisible)(THIS) SCONST OVERRIDE;

    STDMETHOD_(BOOL, MoveWindow)
    (THIS_ int x, int y, int nWidth, int nHeight, BOOL bRepaint DEF_VAL(TRUE)) OVERRIDE;

    STDMETHOD_(BOOL, MoveWindow2)(THIS_ LPCRECT lpRect, BOOL bRepaint DEF_VAL(TRUE)) OVERRIDE;

    STDMETHOD_(BOOL, ShowWindow)(THIS_ int nCmdShow) OVERRIDE;

    STDMETHOD_(int, SetWindowRgn)(THIS_ HRGN hRgn, BOOL bRedraw DEF_VAL(TRUE)) OVERRIDE;

    STDMETHOD_(BOOL, SetLayeredWindowAttributes)
    (THIS_ COLORREF crKey, BYTE bAlpha, DWORD dwFlags) OVERRIDE;

    STDMETHOD_(BOOL, UpdateLayeredWindow)
    (THIS_ HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags) OVERRIDE;

    STDMETHOD_(void, SetMsgHandler)(THIS_ FunMsgHandler fun, void *ctx) OVERRIDE;
    STDMETHOD_(MsgHandlerInfo *, GetMsgHandler)(THIS) OVERRIDE;

    LRESULT DefWindowProc();
    LRESULT ForwardNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    static BOOL DefaultReflectionHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult);

  public: // EXTRACT FROM BEGIN_MSG_MAP_EX and END_MSG_MAP
    virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult, DWORD dwMsgMapID = 0);

  protected:
    LRESULT DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void OnFinalMessage(HWND hWnd);

    const MSG *m_pCurrentMsg;
    BOOL m_bDestoryed;

    MsgHandlerInfo m_msgHandlerInfo;

  public:
    HWND m_hWnd;

  protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // 只执行一次
    static LRESULT CALLBACK StartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    tagThunk *m_pThunk;
    WNDPROC m_pfnSuperWindowProc;
};

SNSEND
#endif // __SNATIVEWND__H__