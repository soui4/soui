#ifndef __SSTRING_I__H__
#define __SSTRING_I__H__

#include <windows.h>
#include <utilities-def.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IStringA
DECLARE_INTERFACE(IStringA)
{
    /**
     * @brief Gets the length of the string.
     * @return The length of the string.
     */
    STDMETHOD_(int, GetLength)() SCONST PURE;

    /**
     * @brief Checks if the string is empty.
     * @return TRUE if the string is empty, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsEmpty)(CTHIS) SCONST PURE;

    /**
     * @brief Empties the string.
     */
    STDMETHOD_(void, Empty)(THIS) PURE;

    /**
     * @brief Gets the character at the specified index.
     * @param nIndex Index of the character.
     * @return The character at the specified index.
     */
    STDMETHOD_(char, GetAt)(CTHIS_ int nIndex) SCONST PURE;

    /**
     * @brief Sets the character at the specified index.
     * @param nIndex Index of the character.
     * @param ch Character to set.
     */
    STDMETHOD_(void, SetAt)(THIS_ int nIndex, char ch) PURE;

    /**
     * @brief Gets a C-style string representation of the string.
     * @return Pointer to the C-style string.
     */
    STDMETHOD_(const char *, c_str)(CTHIS) SCONST PURE;

    /**
     * @brief Compares the string with another string.
     * @param psz String to compare with.
     * @return 0 if equal, < 0 if less, > 0 if greater.
     */
    STDMETHOD_(int, Compare)(CTHIS_ const char* psz) SCONST PURE;

    /**
     * @brief Compares the string with another string, ignoring case.
     * @param psz String to compare with.
     * @return 0 if equal, < 0 if less, > 0 if greater.
     */
    STDMETHOD_(int, CompareNoCase)(CTHIS_ const char* psz) SCONST PURE;

    /**
     * @brief Trims leading and trailing whitespace from the string.
     */
    STDMETHOD_(void, TrimBlank)(THIS) PURE;

    /**
     * @brief Inserts a character at the specified index.
     * @param nIndex Index to insert at.
     * @param ch Character to insert.
     * @return The new length of the string.
     */
    STDMETHOD_(int, InsertChar)(THIS_ int nIndex, char ch) PURE;

    /**
     * @brief Inserts a substring at the specified index.
     * @param nIndex Index to insert at.
     * @param psz Substring to insert.
     * @return The new length of the string.
     */
    STDMETHOD_(int, Insert)(THIS_ int nIndex, const char* psz) PURE;

    /**
     * @brief Deletes a substring from the specified index.
     * @param nIndex Index to start deletion.
     * @param nCount Number of characters to delete.
     * @return The new length of the string.
     */
    STDMETHOD_(int, Delete)(THIS_ int nIndex, int nCount) PURE;

    /**
     * @brief Replaces all occurrences of a character with another character.
     * @param chOld Character to replace.
     * @param chNew Character to replace with.
     * @return The number of replacements made.
     */
    STDMETHOD_(int, ReplaceChar)(THIS_ char chOld, char chNew) PURE;

    /**
     * @brief Replaces all occurrences of a substring with another substring.
     * @param pszOld Substring to replace.
     * @param pszNew Substring to replace with.
     * @return The number of replacements made.
     */
    STDMETHOD_(int, Replace)(THIS_ const char* pszOld, const char* pszNew) PURE;

    /**
     * @brief Removes all occurrences of a character.
     * @param chRemove Character to remove.
     * @return The number of characters removed.
     */
    STDMETHOD_(int, Remove)(THIS_ char chRemove) PURE;

    /**
     * @brief Finds the first occurrence of a character.
     * @param ch Character to find.
     * @param nStart Starting index for the search.
     * @return Index of the first occurrence, or -1 if not found.
     */
    STDMETHOD_(int, FindChar)(CTHIS_ char ch, int nStart DEF_VAL(0)) SCONST PURE;

    /**
     * @brief Finds the last occurrence of a character.
     * @param ch Character to find.
     * @return Index of the last occurrence, or -1 if not found.
     */
    STDMETHOD_(int, ReverseFind)(CTHIS_ char ch) SCONST PURE;

    /**
     * @brief Finds the first occurrence of a substring.
     * @param pszSub Substring to find.
     * @param nStart Starting index for the search.
     * @return Index of the first occurrence, or -1 if not found.
     */
    STDMETHOD_(int, Find)(CTHIS_ const char* pszSub, int nStart DEF_VAL(0)) SCONST PURE;

    /**
     * @brief Gets a modifiable buffer for the string.
     * @param nMinBufLength Minimum buffer length.
     * @return Pointer to the buffer.
     */
    STDMETHOD_(char*, GetBuffer)(THIS_ int nMinBufLength DEF_VAL(-1)) PURE;

    /**
     * @brief Releases the buffer and sets the new length of the string.
     * @param nNewLength New length of the string.
     */
    STDMETHOD_(void, ReleaseBuffer)(THIS_ int nNewLength DEF_VAL(-1)) PURE;

    /**
     * @brief Gets a buffer with the specified length.
     * @param nNewLength New length of the buffer.
     * @return Pointer to the buffer.
     */
    STDMETHOD_(char*, GetBufferSetLength)(THIS_ int nNewLength) PURE;

    /**
     * @brief Sets the length of the string.
     * @param nLength New length of the string.
     */
    STDMETHOD_(void, SetLength)(THIS_ int nLength) PURE;

    /**
     * @brief Copies the contents of another string.
     * @param src Source string.
     */
    STDMETHOD_(void, Copy)(THIS_ const IStringA * src) PURE;

    /**
     * @brief Assigns a C-style string to the string.
     * @param src C-style string to assign.
     */
    STDMETHOD_(void, Assign)(THIS_ LPCSTR src) PURE;

    /**
     * @brief Assigns a substring of a C-style string to the string.
     * @param src C-style string to assign.
     * @param nLen Length of the substring.
     */
    STDMETHOD_(void, Assign2)(THIS_ LPCSTR src, int nLen DEF_VAL(-1)) PURE;

    /**
     * @brief Gets private data associated with the string.
     * @return Pointer to private data.
     */
    STDMETHOD_(LPVOID, GetPrivData)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to uppercase.
     */
    STDMETHOD_(void, ToUpper)(THIS) PURE;

    /**
     * @brief Converts the string to lowercase.
     */
    STDMETHOD_(void, ToLower)(THIS) PURE;

    /**
     * @brief Trims trailing characters from the string.
     * @param chTarget Character to trim.
     */
    STDMETHOD_(void, TrimRight)(THIS_ char chTarget DEF_VAL(VK_SPACE)) PURE;

    /**
     * @brief Trims leading characters from the string.
     * @param chTarget Character to trim.
     */
    STDMETHOD_(void, TrimLeft)(THIS_ char chTarget DEF_VAL(VK_SPACE)) PURE;

    /**
     * @brief Trims leading and trailing characters from the string.
     * @param chTarget Character to trim.
     */
    STDMETHOD_(void, Trim)(THIS_ char chTarget DEF_VAL(VK_SPACE)) PURE;

    /**
     * @brief Appends a character to the string.
     * @param ch Character to append.
     */
    STDMETHOD_(void, AppendChar)(THIS_ char ch) PURE;

    /**
     * @brief Appends a substring to the string.
     * @param pszStr Substring to append.
     * @param nLen Length of the substring.
     */
    STDMETHOD_(void, AppendStr)(THIS_ const char *pszStr, int nLen DEF_VAL(-1)) PURE;

    /**
     * @brief Releases the string.
     */
    STDMETHOD_(void, Release)(THIS) PURE;

    /**
     * @brief Converts the string to an unsigned integer.
     * @return The unsigned integer value.
     */
    STDMETHOD_(UINT, ToUint)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to a long integer.
     * @return The long integer value.
     */
    STDMETHOD_(long, ToLong)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to an integer.
     * @return The integer value.
     */
    STDMETHOD_(int, ToInt)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to a float.
     * @return The float value.
     */
    STDMETHOD_(float, ToFloat)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to a double.
     * @return The double value.
     */
    STDMETHOD_(double, ToDouble)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to a boolean.
     * @return The boolean value.
     */
    STDMETHOD_(BOOL, ToBool)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IStringW
DECLARE_INTERFACE(IStringW)
{
    /**
     * @brief Gets the length of the string.
     * @return The length of the string.
     */
    STDMETHOD_(int, GetLength)(CTHIS) SCONST PURE;

    /**
     * @brief Checks if the string is empty.
     * @return TRUE if the string is empty, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsEmpty)(CTHIS) SCONST PURE;

    /**
     * @brief Empties the string.
     */
    STDMETHOD_(void, Empty)(THIS) PURE;

    /**
     * @brief Gets the character at the specified index.
     * @param nIndex Index of the character.
     * @return The character at the specified index.
     */
    STDMETHOD_(wchar_t, GetAt)(CTHIS_ int nIndex) SCONST PURE;

    /**
     * @brief Sets the character at the specified index.
     * @param nIndex Index of the character.
     * @param ch Character to set.
     */
    STDMETHOD_(void, SetAt)(THIS_ int nIndex, wchar_t ch) PURE;

    /**
     * @brief Gets a C-style string representation of the string.
     * @return Pointer to the C-style string.
     */
    STDMETHOD_(const wchar_t *, c_str)(CTHIS) SCONST PURE;

    /**
     * @brief Compares the string with another string.
     * @param psz String to compare with.
     * @return 0 if equal, < 0 if less, > 0 if greater.
     */
    STDMETHOD_(int, Compare)(CTHIS_ const wchar_t* psz) SCONST PURE;

    /**
     * @brief Compares the string with another string, ignoring case.
     * @param psz String to compare with.
     * @return 0 if equal, < 0 if less, > 0 if greater.
     */
    STDMETHOD_(int, CompareNoCase)(CTHIS_ const wchar_t* psz) SCONST PURE;

    /**
     * @brief Trims leading and trailing whitespace from the string.
     */
    STDMETHOD_(void, TrimBlank)(THIS) PURE;

    /**
     * @brief Inserts a character at the specified index.
     * @param nIndex Index to insert at.
     * @param ch Character to insert.
     * @return The new length of the string.
     */
    STDMETHOD_(int, InsertChar)(THIS_ int nIndex, wchar_t ch) PURE;

    /**
     * @brief Inserts a substring at the specified index.
     * @param nIndex Index to insert at.
     * @param psz Substring to insert.
     * @return The new length of the string.
     */
    STDMETHOD_(int, Insert)(THIS_ int nIndex, const wchar_t* psz) PURE;

    /**
     * @brief Deletes a substring from the specified index.
     * @param nIndex Index to start deletion.
     * @param nCount Number of characters to delete.
     * @return The new length of the string.
     */
    STDMETHOD_(int, Delete)(THIS_ int nIndex, int nCount) PURE;

    /**
     * @brief Replaces all occurrences of a character with another character.
     * @param chOld Character to replace.
     * @param chNew Character to replace with.
     * @return The number of replacements made.
     */
    STDMETHOD_(int, ReplaceChar)(THIS_ wchar_t chOld, wchar_t chNew) PURE;

    /**
     * @brief Replaces all occurrences of a substring with another substring.
     * @param pszOld Substring to replace.
     * @param pszNew Substring to replace with.
     * @return The number of replacements made.
     */
    STDMETHOD_(int, Replace)(THIS_ const wchar_t* pszOld, const wchar_t* pszNew) PURE;

    /**
     * @brief Removes all occurrences of a character.
     * @param chRemove Character to remove.
     * @return The number of characters removed.
     */
    STDMETHOD_(int, Remove)(THIS_ wchar_t chRemove) PURE;

    /**
     * @brief Finds the first occurrence of a character.
     * @param ch Character to find.
     * @param nStart Starting index for the search.
     * @return Index of the first occurrence, or -1 if not found.
     */
    STDMETHOD_(int, FindChar)(CTHIS_ wchar_t ch, int nStart DEF_VAL(0)) SCONST PURE;

    /**
     * @brief Finds the last occurrence of a character.
     * @param ch Character to find.
     * @return Index of the last occurrence, or -1 if not found.
     */
    STDMETHOD_(int, ReverseFind)(CTHIS_ wchar_t ch) SCONST PURE;

    /**
     * @brief Finds the first occurrence of a substring.
     * @param pszSub Substring to find.
     * @param nStart Starting index for the search.
     * @return Index of the first occurrence, or -1 if not found.
     */
    STDMETHOD_(int, Find)(CTHIS_ const wchar_t* pszSub, int nStart DEF_VAL(0)) SCONST PURE;

    /**
     * @brief Gets a modifiable buffer for the string.
     * @param nMinBufLength Minimum buffer length.
     * @return Pointer to the buffer.
     */
    STDMETHOD_(wchar_t*, GetBuffer)(THIS_ int nMinBufLength DEF_VAL(-1)) PURE;

    /**
     * @brief Releases the buffer and sets the new length of the string.
     * @param nNewLength New length of the string.
     */
    STDMETHOD_(void, ReleaseBuffer)(THIS_ int nNewLength DEF_VAL(-1)) PURE;

    /**
     * @brief Gets a buffer with the specified length.
     * @param nNewLength New length of the buffer.
     * @return Pointer to the buffer.
     */
    STDMETHOD_(wchar_t*, GetBufferSetLength)(THIS_ int nNewLength) PURE;

    /**
     * @brief Sets the length of the string.
     * @param nLength New length of the string.
     */
    STDMETHOD_(void, SetLength)(THIS_ int nLength) PURE;

    /**
     * @brief Copies the contents of another string.
     * @param src Source string.
     */
    STDMETHOD_(void, Copy)(THIS_ const IStringW * src) PURE;

    /**
     * @brief Assigns a C-style string to the string.
     * @param src C-style string to assign.
     */
    STDMETHOD_(void, Assign)(THIS_ LPCWSTR src) PURE;

    /**
     * @brief Assigns a substring of a C-style string to the string.
     * @param src C-style string to assign.
     * @param nLen Length of the substring.
     */
    STDMETHOD_(void, Assign2)(THIS_ LPCWSTR src, int nLen DEF_VAL(-1)) PURE;

    /**
     * @brief Gets private data associated with the string.
     * @return Pointer to private data.
     */
    STDMETHOD_(LPVOID, GetPrivData)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to uppercase.
     */
    STDMETHOD_(void, ToUpper)(THIS) PURE;

    /**
     * @brief Converts the string to lowercase.
     */
    STDMETHOD_(void, ToLower)(THIS) PURE;

    /**
     * @brief Trims trailing characters from the string.
     * @param chTarget Character to trim.
     */
    STDMETHOD_(void, TrimRight)(THIS_ wchar_t chTarget DEF_VAL(VK_SPACE)) PURE;

    /**
     * @brief Trims leading characters from the string.
     * @param chTarget Character to trim.
     */
    STDMETHOD_(void, TrimLeft)(THIS_ wchar_t chTarget DEF_VAL(VK_SPACE)) PURE;

    /**
     * @brief Trims leading and trailing characters from the string.
     * @param chTarget Character to trim.
     */
    STDMETHOD_(void, Trim)(THIS_ wchar_t chTarget DEF_VAL(VK_SPACE)) PURE;

    /**
     * @brief Appends a character to the string.
     * @param ch Character to append.
     */
    STDMETHOD_(void, AppendChar)(THIS_ wchar_t ch) PURE;

    /**
     * @brief Appends a substring to the string.
     * @param pszStr Substring to append.
     * @param nLen Length of the substring.
     */
    STDMETHOD_(void, AppendStr)(THIS_ const wchar_t *pszStr, int nLen DEF_VAL(-1)) PURE;

    /**
     * @brief Releases the string.
     */
    STDMETHOD_(void, Release)(THIS) PURE;

    /**
     * @brief Converts the string to an unsigned integer.
     * @return The unsigned integer value.
     */
    STDMETHOD_(UINT, ToUint)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to a long integer.
     * @return The long integer value.
     */
    STDMETHOD_(long, ToLong)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to an integer.
     * @return The integer value.
     */
    STDMETHOD_(int, ToInt)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to a float.
     * @return The float value.
     */
    STDMETHOD_(float, ToFloat)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to a double.
     * @return The double value.
     */
    STDMETHOD_(double, ToDouble)(CTHIS) SCONST PURE;

    /**
     * @brief Converts the string to a boolean.
     * @return The boolean value.
     */
    STDMETHOD_(BOOL, ToBool)(CTHIS) SCONST PURE;
};

#ifdef _UNICODE
#define IStringT IStringW
#else
#define IStringT IStringA
#endif

SNSEND
#endif // __SSTRING_I__H__
