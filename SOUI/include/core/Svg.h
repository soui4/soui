#ifndef __SVG__H__
#define __SVG__H__
#include <interface/SRender-i.h>

SNSBEGIN

/**
 * CreateSvgObj
 * @brief 创建svg对象
 * @param strSvg svg数据
 * @return ISvgObjPtr svg对象
 */
EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgObj(char *strSvg);

/**
 * CreateSvgFromResId
 * @brief 创建svg对象
 * @param resId 资源ID
 * @return ISvgObjPtr svg对象
 */
EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgFromResId(LPCTSTR resId);

/**
 * CreateSvgFromFile
 * @brief 创建svg对象
 * @param resId 文件路径
 * @return ISvgObjPtr svg对象
 */
EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgFromFileA(LPCSTR resId);

/**
 * CreateSvgFromFile
 * @brief 创建svg对象
 * @param resId 文件路径
 * @return ISvgObjPtr svg对象
 */
EXTERN_C ISvgObjPtr SOUI_EXP CreateSvgFromFileW(LPCWSTR resId);

#ifdef UNICODE
#define CreateSvgFromFile CreateSvgFromFileW
#else
#define CreateSvgFromFile CreateSvgFromFileA
#endif // UNICODE

SNSEND

#endif // !__SVG__H__