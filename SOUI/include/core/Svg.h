#ifndef __SVG__H__
#define __SVG__H__
#include <interface/SRender-i.h>

SNSBEGIN

/**
 * CreateSvgObj
 * @brief Create svg object
 * @param strSvg svg data
 * @return ISvgObjPtr svg object
 */
EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgObj(char *strSvg);

/**
 * CreateSvgFromResId
 * @brief Create svg object
 * @param resId Resource ID
 * @return ISvgObjPtr svg object
 */
EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgFromResId(LPCTSTR resId);

/**
 * CreateSvgFromFile
 * @brief Create svg object
 * @param resId File path
 * @return ISvgObjPtr svg object
 */
EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgFromFileA(LPCSTR resId);

/**
 * CreateSvgFromFile
 * @brief Create svg object
 * @param resId File path
 * @return ISvgObjPtr svg object
 */
EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgFromFileW(LPCWSTR resId);

#ifdef UNICODE
#define CreateSvgFromFile CreateSvgFromFileW
#else
#define CreateSvgFromFile CreateSvgFromFileA
#endif /**< UNICODE */

SNSEND

#endif /**< !__SVG__H__ */