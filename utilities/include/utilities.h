#ifndef __UTILITIES__H__
#define __UTILITIES__H__

#include <utilities-def.h>
#include <windows.h>
#include <interface/sstring-i.h>
#include <interface/sxml-i.h>
#ifdef __cplusplus
extern "C" {
#define StringA SNS::IStringA
#define StringW SNS::IStringW
#define XmlDoc SNS::IXmlDoc
#else
#define StringA IStringA
#define StringW IStringW
#define XmlDoc IXmlDoc
#endif//__cplusplus

/**
 * @brief Calculates the width of a rectangle.
 * 
 * @param rc Pointer to a RECT structure containing the coordinates of the rectangle.
 * @return The width of the rectangle.
 */
UTILITIES_API int RectWidth(LPCRECT rc);

/**
 * @brief Calculates the height of a rectangle.
 * 
 * @param rc Pointer to a RECT structure containing the coordinates of the rectangle.
 * @return The height of the rectangle.
 */
UTILITIES_API int RectHeight(LPCRECT rc);

#ifdef _WIN32
/**
 * @brief Converts a size from HIMETRIC units to pixels.
 * 
 * @param lpSizeInHiMetric Pointer to a SIZEL structure containing the size in HIMETRIC units.
 * @param lpSizeInPix Pointer to a SIZEL structure that receives the size in pixels.
 */
UTILITIES_API void SHiMetricToPixel(const SIZEL* lpSizeInHiMetric, LPSIZEL lpSizeInPix);

/**
 * @brief Converts a size from pixels to HIMETRIC units.
 * 
 * @param lpSizeInPix Pointer to a SIZEL structure containing the size in pixels.
 * @param lpSizeInHiMetric Pointer to a SIZEL structure that receives the size in HIMETRIC units.
 */
UTILITIES_API void SPixelToHiMetric(const SIZEL* lpSizeInPix, LPSIZEL lpSizeInHiMetric);

/**
 * @brief Updates a DIB (Device-Independent Bitmap) pixmap.
 * 
 * @param bmp Handle to the bitmap to update.
 * @param wid Width of the bitmap.
 * @param hei Height of the bitmap.
 * @param bitsPixel Number of bits per pixel.
 * @param stride Number of bytes in each scan line.
 * @param pjBits Pointer to the bitmap bits.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL UTILITIES_API UpdateDIBPixmap(HBITMAP bmp, int wid, int hei, int bitsPixel, int stride, CONST VOID* pjBits);
#endif

/**
 * @brief Converts a wide string to an integer.
 * 
 * @param src Pointer to the wide string to convert.
 * @param supportHex TRUE if the string may contain a hexadecimal number, FALSE otherwise.
 * @return The converted integer.
 */
UTILITIES_API int Str2IntW(LPCWSTR src, BOOL supportHex);

/**
 * @brief Converts a narrow string to an integer.
 * 
 * @param src Pointer to the narrow string to convert.
 * @param supportHex TRUE if the string may contain a hexadecimal number, FALSE otherwise.
 * @return The converted integer.
 */
UTILITIES_API int Str2IntA(LPCSTR src, BOOL supportHex);

/**
 * @brief Creates an IStringA object from a narrow string.
 * 
 * @param src Pointer to the narrow string.
 * @return Pointer to the created IStringA object.
 */
UTILITIES_API StringA* CreateIStringA(LPCSTR src);

/**
 * @brief Creates an IStringW object from a wide string.
 * 
 * @param src Pointer to the wide string.
 * @return Pointer to the created IStringW object.
 */
UTILITIES_API StringW* CreateIStringW(LPCWSTR src);
#ifdef _UNICODE
#define Str2Int Str2IntW
#define file_length file_length_W
#else
#define Str2Int Str2IntA
#define file_length file_length_A
#endif
/**
 * @brief Creates an IXmlDoc object.
 * 
 * @return Pointer to the created IXmlDoc object.
 */
UTILITIES_API XmlDoc* CreateIXmlDoc();

#ifdef _WIN32
BOOL UTILITIES_API PEVersion(LPCTSTR pszFileName, WORD* wMajor, WORD* wMinor, WORD* wVer3, WORD* wVer4);
#endif//_WIN32

/**
 * @brief Gets the scaling factor of a window.
 * 
 * @param hWnd Handle to the window.
 * @return The scaling factor of the window.
 */
int UTILITIES_API GetWindowScale(HWND hWnd);

/**
 * @brief Checks if a file path is valid.
 * 
 * @param path Pointer to the file path.
 * @return TRUE if the file path is valid, FALSE otherwise.
 */
BOOL UTILITIES_API IsFilePathValid(LPCTSTR path);

/**
 * @brief Checks if a font is available.
 * 
 * @param fontName Pointer to the font name.
 * @return TRUE if the font is available, FALSE otherwise.
 */
BOOL UTILITIES_API HasFont(LPCTSTR fontName);

/**
 * @brief Checks if the system supports maximizing windows by dragging.
 * 
 * @return TRUE if supported, FALSE otherwise.
 */
BOOL UTILITIES_API IsSupportMaxMove();

/**
 * @brief Gets the length of a file.
 * 
 * @param path Pointer to the file path (narrow string).
 * @return The length of the file in bytes.
 */
long UTILITIES_API file_length_A(const char* path);

/**
 * @brief Gets the length of a file.
 * 
 * @param path Pointer to the file path (wide string).
 * @return The length of the file in bytes.
 */
long UTILITIES_API file_length_W(const wchar_t* path);

/**
 * @brief Loads an icon or cursor from memory.
 * 
 * @param buf Pointer to the memory buffer containing the icon or cursor data.
 * @param cbSize Size of the memory buffer.
 * @param fIcon TRUE if the data is an icon, FALSE if it is a cursor.
 * @param width Width of the icon or cursor.
 * @param height Height of the icon or cursor.
 * @param cFlag Flags for loading the icon or cursor.
 * @return Handle to the loaded icon or cursor.
 */
HANDLE UTILITIES_API LoadIconFromMemory(const void* buf, UINT cbSize, BOOL fIcon, int width, int height, UINT cFlag);

/**
 * @brief Creates a region from a bitmap.
 * 
 * @param hBmp Handle to the bitmap.
 * @param cr Color value for the region.
 * @param crMask Mask color value.
 * @return Handle to the created region.
 */
HRGN UTILITIES_API CreateRegionFromBitmap(HBITMAP hBmp, COLORREF cr, COLORREF crMask);

#ifdef __cplusplus
}
#endif//__cplusplus
#endif // __UTILITIES__H__