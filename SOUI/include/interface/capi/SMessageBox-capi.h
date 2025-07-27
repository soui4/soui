#ifndef __SMESSAGEBOX_CAPI_H__
#define __SMESSAGEBOX_CAPI_H__

#include "../SMessageBox-i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API Helper Macros for SMessageBox
 * These macros provide C-style function call syntax for SOUI message box functionality
 */

/* Direct function call - already C-compatible */
#define SOUI_MessageBox(hWnd, lpText, lpCaption, uType) \
    SMessageBox(hWnd, lpText, lpCaption, uType)

/*
 * Convenience macros for common message box types
 */

/* Information message box */
#define SOUI_MessageBox_Info(hWnd, text, caption) \
    SMessageBox(hWnd, text, caption, MB_OK | MB_ICONINFORMATION)

#define SOUI_MessageBox_InfoSimple(hWnd, text) \
    SMessageBox(hWnd, text, _T("Information"), MB_OK | MB_ICONINFORMATION)

/* Warning message box */
#define SOUI_MessageBox_Warning(hWnd, text, caption) \
    SMessageBox(hWnd, text, caption, MB_OK | MB_ICONWARNING)

#define SOUI_MessageBox_WarningSimple(hWnd, text) \
    SMessageBox(hWnd, text, _T("Warning"), MB_OK | MB_ICONWARNING)

/* Error message box */
#define SOUI_MessageBox_Error(hWnd, text, caption) \
    SMessageBox(hWnd, text, caption, MB_OK | MB_ICONERROR)

#define SOUI_MessageBox_ErrorSimple(hWnd, text) \
    SMessageBox(hWnd, text, _T("Error"), MB_OK | MB_ICONERROR)

/* Question message box */
#define SOUI_MessageBox_Question(hWnd, text, caption) \
    SMessageBox(hWnd, text, caption, MB_YESNO | MB_ICONQUESTION)

#define SOUI_MessageBox_QuestionSimple(hWnd, text) \
    SMessageBox(hWnd, text, _T("Question"), MB_YESNO | MB_ICONQUESTION)

/* Confirmation message box */
#define SOUI_MessageBox_Confirm(hWnd, text, caption) \
    SMessageBox(hWnd, text, caption, MB_OKCANCEL | MB_ICONQUESTION)

#define SOUI_MessageBox_ConfirmSimple(hWnd, text) \
    SMessageBox(hWnd, text, _T("Confirm"), MB_OKCANCEL | MB_ICONQUESTION)

/* Yes/No/Cancel message box */
#define SOUI_MessageBox_YesNoCancel(hWnd, text, caption) \
    SMessageBox(hWnd, text, caption, MB_YESNOCANCEL | MB_ICONQUESTION)

#define SOUI_MessageBox_YesNoCancelSimple(hWnd, text) \
    SMessageBox(hWnd, text, _T("Choose"), MB_YESNOCANCEL | MB_ICONQUESTION)

/* Retry/Cancel message box */
#define SOUI_MessageBox_RetryCancel(hWnd, text, caption) \
    SMessageBox(hWnd, text, caption, MB_RETRYCANCEL | MB_ICONWARNING)

#define SOUI_MessageBox_RetryCancelSimple(hWnd, text) \
    SMessageBox(hWnd, text, _T("Retry"), MB_RETRYCANCEL | MB_ICONWARNING)

/* Abort/Retry/Ignore message box */
#define SOUI_MessageBox_AbortRetryIgnore(hWnd, text, caption) \
    SMessageBox(hWnd, text, caption, MB_ABORTRETRYIGNORE | MB_ICONERROR)

#define SOUI_MessageBox_AbortRetryIgnoreSimple(hWnd, text) \
    SMessageBox(hWnd, text, _T("Error"), MB_ABORTRETRYIGNORE | MB_ICONERROR)

/*
 * Helper functions for common scenarios
 */

/* Show simple message with automatic parent window detection */
static inline INT_PTR SOUI_ShowMessage_C(LPCTSTR text)
{
    return SMessageBox(NULL, text, _T("Message"), MB_OK);
}

static inline INT_PTR SOUI_ShowInfo_C(LPCTSTR text)
{
    return SMessageBox(NULL, text, _T("Information"), MB_OK | MB_ICONINFORMATION);
}

static inline INT_PTR SOUI_ShowWarning_C(LPCTSTR text)
{
    return SMessageBox(NULL, text, _T("Warning"), MB_OK | MB_ICONWARNING);
}

static inline INT_PTR SOUI_ShowError_C(LPCTSTR text)
{
    return SMessageBox(NULL, text, _T("Error"), MB_OK | MB_ICONERROR);
}

static inline BOOL SOUI_AskQuestion_C(LPCTSTR text)
{
    return SMessageBox(NULL, text, _T("Question"), MB_YESNO | MB_ICONQUESTION) == IDYES;
}

static inline BOOL SOUI_AskConfirm_C(LPCTSTR text)
{
    return SMessageBox(NULL, text, _T("Confirm"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK;
}

/* Message box with formatted text */
static inline INT_PTR SOUI_MessageBoxF_C(HWND hWnd, LPCTSTR caption, UINT uType, LPCTSTR format, ...)
{
    TCHAR buffer[1024];
    va_list args;
    va_start(args, format);
    _vsntprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);
    va_end(args);
    return SMessageBox(hWnd, buffer, caption, uType);
}

static inline INT_PTR SOUI_ShowInfoF_C(HWND hWnd, LPCTSTR format, ...)
{
    TCHAR buffer[1024];
    va_list args;
    va_start(args, format);
    _vsntprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);
    va_end(args);
    return SMessageBox(hWnd, buffer, _T("Information"), MB_OK | MB_ICONINFORMATION);
}

static inline INT_PTR SOUI_ShowWarningF_C(HWND hWnd, LPCTSTR format, ...)
{
    TCHAR buffer[1024];
    va_list args;
    va_start(args, format);
    _vsntprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);
    va_end(args);
    return SMessageBox(hWnd, buffer, _T("Warning"), MB_OK | MB_ICONWARNING);
}

static inline INT_PTR SOUI_ShowErrorF_C(HWND hWnd, LPCTSTR format, ...)
{
    TCHAR buffer[1024];
    va_list args;
    va_start(args, format);
    _vsntprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);
    va_end(args);
    return SMessageBox(hWnd, buffer, _T("Error"), MB_OK | MB_ICONERROR);
}

/*
 * Result checking helpers
 */
#define SOUI_MessageBox_IsOK(result) \
    ((result) == IDOK)

#define SOUI_MessageBox_IsCancel(result) \
    ((result) == IDCANCEL)

#define SOUI_MessageBox_IsYes(result) \
    ((result) == IDYES)

#define SOUI_MessageBox_IsNo(result) \
    ((result) == IDNO)

#define SOUI_MessageBox_IsAbort(result) \
    ((result) == IDABORT)

#define SOUI_MessageBox_IsRetry(result) \
    ((result) == IDRETRY)

#define SOUI_MessageBox_IsIgnore(result) \
    ((result) == IDIGNORE)

/*
 * Safe message box operations
 */
#define SOUI_SafeMessageBox(hWnd, text, caption, uType) \
    ((text) ? SMessageBox(hWnd, text, caption ? caption : _T(""), uType) : IDCANCEL)

#define SOUI_SafeShowInfo(text) \
    ((text) ? SOUI_ShowInfo_C(text) : IDCANCEL)

#define SOUI_SafeShowWarning(text) \
    ((text) ? SOUI_ShowWarning_C(text) : IDCANCEL)

#define SOUI_SafeShowError(text) \
    ((text) ? SOUI_ShowError_C(text) : IDCANCEL)

#define SOUI_SafeAskQuestion(text) \
    ((text) ? SOUI_AskQuestion_C(text) : FALSE)

#define SOUI_SafeAskConfirm(text) \
    ((text) ? SOUI_AskConfirm_C(text) : FALSE)

/*
 * Message box type combinations
 */
#define SOUI_MB_INFO \
    (MB_OK | MB_ICONINFORMATION)

#define SOUI_MB_WARNING \
    (MB_OK | MB_ICONWARNING)

#define SOUI_MB_ERROR \
    (MB_OK | MB_ICONERROR)

#define SOUI_MB_QUESTION \
    (MB_YESNO | MB_ICONQUESTION)

#define SOUI_MB_CONFIRM \
    (MB_OKCANCEL | MB_ICONQUESTION)

#define SOUI_MB_YESNOCANCEL \
    (MB_YESNOCANCEL | MB_ICONQUESTION)

#define SOUI_MB_RETRYCANCEL \
    (MB_RETRYCANCEL | MB_ICONWARNING)

#define SOUI_MB_ABORTRETRYIGNORE \
    (MB_ABORTRETRYIGNORE | MB_ICONERROR)

/*
 * Convenience macros using predefined types
 */
#define SOUI_MessageBox_WithType(hWnd, text, caption, type) \
    SMessageBox(hWnd, text, caption, SOUI_MB_##type)

#define SOUI_ShowMessageType(text, type) \
    SMessageBox(NULL, text, _T(#type), SOUI_MB_##type)

/*
 * Debug and development helpers
 */
#ifdef _DEBUG
#define SOUI_DebugMessage(text) \
    SOUI_ShowInfo_C(_T("DEBUG: ") text)

#define SOUI_DebugMessageF(format, ...) \
    SOUI_ShowInfoF_C(NULL, _T("DEBUG: ") format, __VA_ARGS__)
#else
#define SOUI_DebugMessage(text) \
    ((void)0)

#define SOUI_DebugMessageF(format, ...) \
    ((void)0)
#endif

/*
 * Error reporting helpers
 */
static inline void SOUI_ReportError_C(LPCTSTR operation, DWORD errorCode)
{
    TCHAR buffer[512];
    _sntprintf_s(buffer, _countof(buffer), _TRUNCATE,
        _T("Operation '%s' failed with error code: %lu (0x%08lX)"),
        operation ? operation : _T("Unknown"),
        errorCode, errorCode);
    SOUI_ShowError_C(buffer);
}

static inline void SOUI_ReportLastError_C(LPCTSTR operation)
{
    SOUI_ReportError_C(operation, GetLastError());
}

/*
 * User choice helpers
 */
typedef enum {
    SOUI_CHOICE_OK,
    SOUI_CHOICE_CANCEL,
    SOUI_CHOICE_YES,
    SOUI_CHOICE_NO,
    SOUI_CHOICE_ABORT,
    SOUI_CHOICE_RETRY,
    SOUI_CHOICE_IGNORE
} SOUI_Choice;

static inline SOUI_Choice SOUI_GetUserChoice_C(INT_PTR result)
{
    switch (result) {
        case IDOK:     return SOUI_CHOICE_OK;
        case IDCANCEL: return SOUI_CHOICE_CANCEL;
        case IDYES:    return SOUI_CHOICE_YES;
        case IDNO:     return SOUI_CHOICE_NO;
        case IDABORT:  return SOUI_CHOICE_ABORT;
        case IDRETRY:  return SOUI_CHOICE_RETRY;
        case IDIGNORE: return SOUI_CHOICE_IGNORE;
        default:       return SOUI_CHOICE_CANCEL;
    }
}

static inline BOOL SOUI_IsPositiveChoice_C(SOUI_Choice choice)
{
    return (choice == SOUI_CHOICE_OK || choice == SOUI_CHOICE_YES || choice == SOUI_CHOICE_RETRY);
}

static inline BOOL SOUI_IsNegativeChoice_C(SOUI_Choice choice)
{
    return (choice == SOUI_CHOICE_CANCEL || choice == SOUI_CHOICE_NO || choice == SOUI_CHOICE_ABORT);
}

#ifdef __cplusplus
}
#endif

#endif /* __SMESSAGEBOX_CAPI_H__ */
