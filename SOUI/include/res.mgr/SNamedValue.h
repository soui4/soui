#ifndef __SNAMEDVALUE__H__
#define __SNAMEDVALUE__H__

#include "helper/SColor.h"
#include "layout/SLayoutSize.h"
#include <stdlib.h>

SNSBEGIN

#define MAX_NAME 50

/**
 * @struct TNAMEDVALUE
 * @brief Structure to hold a named value.
 *
 * @details This structure contains a name (up to `MAX_NAME` characters) and a value of type `T`.
 * @tparam T Type of the value.
 */
template <class T>
struct TNAMEDVALUE
{
    wchar_t strName[MAX_NAME + 1]; // Name of the value
    T value;                       // Value associated with the name
};

/**
 * @class SNamedValue
 * @brief Template class for managing named values.
 *
 * @details This class manages a list of named values, allowing for initialization from XML, retrieval by name, and merging of values.
 * @tparam T Type of the value.
 * @tparam ValueParser Class responsible for parsing the value from a string.
 */
template <class T, class ValueParser>
class SNamedValue {
  public:
    typedef TNAMEDVALUE<T> NAMEDVALUE; // Type alias for TNAMEDVALUE

    /**
     * @brief Initializes the named values from an XML node.
     * @param xmlNode XML node containing the named values.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    virtual BOOL Init(SXmlNode xmlNode)
    {
        if (!xmlNode)
            return FALSE;

        for (SXmlNode xmlStr = xmlNode.first_child(); xmlStr; xmlStr = xmlStr.next_sibling())
        {
            if (xmlStr.type() != node_element)
                continue;

            NAMEDVALUE namedValue;
            wcscpy_s(namedValue.strName, MAX_NAME, xmlStr.name());
            if (ValueParser::ParseValue(xmlStr.attribute(L"value").as_string(), namedValue.value))
            {
                m_lstNamedValue.Add(namedValue);
            }
            else
            {
                SStringW msg = SStringW().Format(L"parse value failed, name=%s,value=%s", xmlStr.name(), xmlStr.attribute(L"value").as_string());
                SASSERT_MSGW(false, msg);
            }
        }
        qsort(m_lstNamedValue.GetData(), m_lstNamedValue.GetCount(), sizeof(NAMEDVALUE), Compare);
        return TRUE;
    }

    /**
     * @brief Converts a string name to its corresponding value.
     * @param strName Name of the value.
     * @return Value associated with the name, or a null value if not found.
     */
    T String2Value(const SStringW &strName) const
    {
        T ret;
        if (FindValue(strName, ret))
            return ret;
        static const T nullValue = ValueParser::GetNullValue();
        return nullValue;
    }

    /**
     * @brief Finds the value associated with a given name.
     * @param strName Name of the value.
     * @param value Variable to store the found value.
     * @return TRUE if the value is found, FALSE otherwise.
     */
    BOOL FindValue(const SStringW &strName, T &value) const
    {
        NAMEDVALUE target;
        wcscpy_s(target.strName, MAX_NAME, strName);
        NAMEDVALUE *pFind = (NAMEDVALUE *)bsearch(&target, m_lstNamedValue.GetData(), m_lstNamedValue.GetCount(), sizeof(NAMEDVALUE), Compare);
        if (!pFind)
            return FALSE;
        value = pFind->value;
        return TRUE;
    }

    /**
     * @brief Retrieves the value at a specific index.
     * @param idx Index of the value.
     * @return Value at the specified index.
     */
    T GetAt(const int idx) const
    {
        SASSERT(idx >= 0 && idx < (int)m_lstNamedValue.GetCount());
        return m_lstNamedValue[idx].value;
    }

    /**
     * @brief Merges the named values from another SNamedValue object.
     * @param src Source SNamedValue object.
     * @return Number of new values added.
     */
    size_t Merge(const SNamedValue &src)
    {
        if (m_lstNamedValue.IsEmpty())
        {
            m_lstNamedValue.Copy(src.m_lstNamedValue);
            return src.m_lstNamedValue.GetCount();
        }
        else
        {
            SArray<NAMEDVALUE> data;
            for (size_t i = 0; i < src.m_lstNamedValue.GetCount(); i++)
            {
                NAMEDVALUE *pFind = (NAMEDVALUE *)bsearch(&src.m_lstNamedValue[i], m_lstNamedValue.GetData(), m_lstNamedValue.GetCount(), sizeof(NAMEDVALUE), Compare);
                if (!pFind)
                {
                    data.Add(src.m_lstNamedValue[i]);
                }
            }
            m_lstNamedValue.Append(data);
            qsort(m_lstNamedValue.GetData(), m_lstNamedValue.GetCount(), sizeof(NAMEDVALUE), Compare);
            return data.GetCount();
        }
    }

    /**
     * @brief Retrieves the number of named values.
     * @return Number of named values.
     */
    UINT GetCount() const
    {
        return (UINT)m_lstNamedValue.GetCount();
    }

  protected:
    /**
     * @brief Comparison function for sorting and searching.
     * @param p1 Pointer to the first NAMEDVALUE.
     * @param p2 Pointer to the second NAMEDVALUE.
     * @return Comparison result.
     */
    static int Compare(const void *p1, const void *p2)
    {
        const NAMEDVALUE *pData1 = (const NAMEDVALUE *)p1;
        const NAMEDVALUE *pData2 = (const NAMEDVALUE *)p2;
        return wcscmp(pData1->strName, pData2->strName);
    }

    SArray<NAMEDVALUE> m_lstNamedValue; // Array to store named values
};

/**
 * @class SIntParser
 * @brief Parser for integer values.
 */
class SOUI_EXP SIntParser {
  public:
    /**
     * @brief Parses a string to an integer value.
     * @param strValue String representation of the value.
     * @param value Variable to store the parsed integer value.
     * @return TRUE if parsing is successful, FALSE otherwise.
     */
    static bool ParseValue(const SStringW &strValue, int &value);

    /**
     * @brief Retrieves the null value for integers.
     * @return Null value for integers.
     */
    static int GetNullValue();
};

/**
 * @class SNamedID
 * @brief Manages named integer IDs.
 */
class SOUI_EXP SNamedID : public SNamedValue<int, SIntParser> {
  public:
    /**
     * @brief Initializes named IDs from an array.
     * @param pValue Array of named values.
     * @param nCount Number of elements in the array.
     * @param bSorted TRUE if the array is already sorted, FALSE otherwise.
     */
    void Init2(const NAMEDVALUE *pValue, int nCount, BOOL bSorted);

    /**
     * @brief Initializes named IDs from arrays of names and IDs.
     * @param pNames Array of names.
     * @param nIDs Array of IDs.
     * @param nCount Number of elements in the arrays.
     * @param bSorted TRUE if the arrays are already sorted, FALSE otherwise.
     */
    void Init3(const LPCWSTR *pNames, const int *nIDs, int nCount, BOOL bSorted);
};

/**
 * @class SColorParser
 * @brief Parser for color values.
 */
class SOUI_EXP SColorParser {
  public:
    /**
     * @brief Parses a string to a color value.
     * @param strValue String representation of the color.
     * @param value Variable to store the parsed color value.
     * @return TRUE if parsing is successful, FALSE otherwise.
     */
    static bool ParseValue(const SStringW &strValue, COLORREF &value);

    /**
     * @brief Retrieves the null value for colors.
     * @return Null value for colors.
     */
    static COLORREF GetNullValue();
};

/**
 * @class SNamedColor
 * @brief Manages named colors.
 */
class SOUI_EXP SNamedColor : public SNamedValue<COLORREF, SColorParser> {
  public:
    /**
     * @brief Retrieves a color value by name, automatically converting named colors.
     * @param strValue Name of the color.
     * @param cr Variable to store the retrieved color value.
     * @return TRUE if the color is found, FALSE otherwise.
     */
    BOOL Get(const SStringW &strValue, COLORREF &cr) const;

    /**
     * @brief Retrieves a color value by index.
     * @param idx Index of the color.
     * @return Color value at the specified index.
     */
    COLORREF Get(int idx) const
    {
        return GetAt(idx);
    }
};

/**
 * @class SStringParser
 * @brief Parser for string values.
 */
class SOUI_EXP SStringParser {
  public:
    /**
     * @brief Parses a string to a string value.
     * @param strValue String representation of the value.
     * @param value Variable to store the parsed string value.
     * @return TRUE if parsing is successful, FALSE otherwise.
     */
    static bool ParseValue(const SStringW &strValue, SStringW &value);

    /**
     * @brief Retrieves the null value for strings.
     * @return Null value for strings.
     */
    static SStringW GetNullValue();
};

/**
 * @class SNamedString
 * @brief Manages named strings.
 */
class SOUI_EXP SNamedString : public SNamedValue<SStringW, SStringParser> {
  public:
    /**
     * @brief Retrieves a string value by name, automatically converting named strings.
     * @param strValue Name of the string.
     * @param ret Variable to store the retrieved string value.
     * @return TRUE if the string is found, FALSE otherwise.
     */
    BOOL Get(const SStringW &strValue, SStringW &ret) const;

    /**
     * @brief Retrieves a string value by index.
     * @param idx Index of the string.
     * @return String value at the specified index.
     */
    SStringW Get(int idx) const
    {
        return GetAt(idx);
    }
};

/**
 * @class SNamedFont
 * @brief Manages named fonts.
 */
class SOUI_EXP SNamedFont : public SNamedValue<SStringW, SStringParser> {
  public:
    /**
     * @brief Retrieves a font value by name, automatically converting named fonts.
     * @param strValue Name of the font.
     * @param ret Variable to store the retrieved font value.
     * @return TRUE if the font is found, FALSE otherwise.
     */
    BOOL Get(const SStringW &strValue, SStringW &ret) const;

    /**
     * @brief Retrieves a font value by index.
     * @param idx Index of the font.
     * @return Font value at the specified index.
     */
    SStringW Get(int idx) const
    {
        return GetAt(idx);
    }
};

/**
 * @class SDimensionParser
 * @brief Parser for dimension values.
 */
class SOUI_EXP SDimensionParser {
  public:
    /**
     * @brief Parses a string to a dimension value.
     * @param strValue String representation of the dimension.
     * @param value Variable to store the parsed dimension value.
     * @return TRUE if parsing is successful, FALSE otherwise.
     */
    static bool ParseValue(const SStringW &strValue, SLayoutSize &value);

    /**
     * @brief Retrieves the null value for dimensions.
     * @return Null value for dimensions.
     */
    static SLayoutSize GetNullValue();
};

/**
 * @class SNamedDimension
 * @brief Manages named dimensions.
 */
class SOUI_EXP SNamedDimension : public SNamedValue<SLayoutSize, SDimensionParser> {
  public:
    /**
     * @brief Retrieves a dimension value by name, automatically converting named dimensions.
     * @param strValue Name of the dimension.
     * @param ret Variable to store the retrieved dimension value.
     * @return TRUE if the dimension is found, FALSE otherwise.
     */
    BOOL Get(const SStringW &strValue, SLayoutSize &ret) const;

    /**
     * @brief Retrieves a dimension value by index.
     * @param idx Index of the dimension.
     * @return Dimension value at the specified index.
     */
    SLayoutSize Get(int idx) const
    {
        return GetAt(idx);
    }
};

SNSEND

#endif // __SNAMEDVALUE__H__