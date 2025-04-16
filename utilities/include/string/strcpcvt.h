#ifndef __STRCPCVT__H__
#define __STRCPCVT__H__

#include <windows.h>

#ifndef CP_ACP
#define CP_ACP 0
#endif // CP_ACP

#include "utilities-def.h"
#include "tstring.h"

SNSBEGIN

/**
 * @class SStrCpCvt
 * @brief Class for string code page conversion.
 * @details Provides static methods to convert strings between different code pages.
 */
class UTILITIES_API SStrCpCvt
{
public:
    /**
     * @brief Converts a wide string (UTF-16) to a multi-byte string (ANSI or specified code page).
     * @param str Wide string to convert.
     * @param cp Code page for the output multi-byte string (default is CP_ACP).
     * @return Converted multi-byte string.
     */
    static SStringA CvtW2A(const SStringW &str, unsigned int cp = CP_ACP);

    /**
     * @brief Converts a multi-byte string (ANSI or specified code page) to a wide string (UTF-16).
     * @param str Multi-byte string to convert.
     * @param cp Code page of the input multi-byte string (default is CP_ACP).
     * @param cp2 Reserved parameter (default is 0).
     * @return Converted wide string.
     */
    static SStringW CvtA2W(const SStringA &str, unsigned int cp = CP_ACP, unsigned int cp2 = 0);

    /**
     * @brief Converts a multi-byte string from one code page to another multi-byte code page.
     * @param str Multi-byte string to convert.
     * @param cpFrom Source code page (default is CP_UTF8).
     * @param cpTo Target code page (default is CP_ACP).
     * @return Converted multi-byte string.
     */
    static SStringA CvtA2A(const SStringA &str, unsigned int cpFrom = CP_UTF8, unsigned int cpTo = CP_ACP);

    /**
     * @brief Converts a wide string (UTF-16) to another wide string (UTF-16) with a specified code page.
     * @param str Wide string to convert.
     * @param cp Code page for the output wide string (default is CP_ACP).
     * @return Converted wide string.
     */
    static SStringW CvtW2W(const SStringW &str, unsigned int cp = CP_ACP);
};

SNSEND

/**
 * @brief Macro for converting a multi-byte string to a wide string.
 * @param str Multi-byte string to convert.
 * @param cp Code page of the input multi-byte string (default is CP_ACP).
 * @param cp2 Reserved parameter (default is 0).
 * @return Converted wide string.
 */
#define S_CA2W SStrCpCvt::CvtA2W

/**
 * @brief Macro for converting a wide string to a multi-byte string.
 * @param str Wide string to convert.
 * @param cp Code page for the output multi-byte string (default is CP_ACP).
 * @return Converted multi-byte string.
 */
#define S_CW2A SStrCpCvt::CvtW2A

/**
 * @brief Macro for converting a multi-byte string from one code page to another multi-byte code page.
 * @param str Multi-byte string to convert.
 * @param cpFrom Source code page (default is CP_UTF8).
 * @param cpTo Target code page (default is CP_ACP).
 * @return Converted multi-byte string.
 */
#define S_CA2A SStrCpCvt::CvtA2A

/**
 * @brief Macro for converting a wide string to another wide string with a specified code page.
 * @param str Wide string to convert.
 * @param cp Code page for the output wide string (default is CP_ACP).
 * @return Converted wide string.
 */
#define S_CW2W SStrCpCvt::CvtW2W

#ifdef UNICODE
/**
 * @brief Macro for converting a multi-byte string to a wide string (Unicode version).
 * @param str Multi-byte string to convert.
 * @param cp Code page of the input multi-byte string (default is CP_ACP).
 * @param cp2 Reserved parameter (default is 0).
 * @return Converted wide string.
 */
#define S_CA2T S_CA2W

/**
 * @brief Macro for converting a wide string to a multi-byte string (Unicode version).
 * @param str Wide string to convert.
 * @param cp Code page for the output multi-byte string (default is CP_ACP).
 * @return Converted multi-byte string.
 */
#define S_CT2A S_CW2A

/**
 * @brief Macro for converting a wide string to another wide string with a specified code page (Unicode version).
 * @param str Wide string to convert.
 * @param cp Code page for the output wide string (default is CP_ACP).
 * @return Converted wide string.
 */
#define S_CW2T S_CW2W

/**
 * @brief Macro for converting a wide string to another wide string with a specified code page (Unicode version).
 * @param str Wide string to convert.
 * @param cp Code page for the output wide string (default is CP_ACP).
 * @return Converted wide string.
 */
#define S_CT2W S_CW2W
#else
/**
 * @brief Macro for converting a multi-byte string to a wide string (ANSI version).
 * @param str Multi-byte string to convert.
 * @param cp Code page of the input multi-byte string (default is CP_ACP).
 * @param cp2 Reserved parameter (default is 0).
 * @return Converted wide string.
 */
#define S_CA2T S_CA2A

/**
 * @brief Macro for converting a wide string to a multi-byte string (ANSI version).
 * @param str Wide string to convert.
 * @param cp Code page for the output multi-byte string (default is CP_ACP).
 * @return Converted multi-byte string.
 */
#define S_CT2A S_CA2A

/**
 * @brief Macro for converting a wide string to another wide string with a specified code page (ANSI version).
 * @param str Wide string to convert.
 * @param cp Code page for the output wide string (default is CP_ACP).
 * @return Converted wide string.
 */
#define S_CW2T S_CW2A

/**
 * @brief Macro for converting a wide string to another wide string with a specified code page (ANSI version).
 * @param str Wide string to convert.
 * @param cp Code page for the output wide string (default is CP_ACP).
 * @return Converted wide string.
 */
#define S_CT2W S_CA2W
#endif // UNICODE

#endif // __STRCPCVT__H__