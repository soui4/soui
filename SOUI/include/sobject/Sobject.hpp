#ifndef __SOBJECT_I_H_
#define __SOBJECT_I_H_

#include <interface/sobject-i.h>
#include <string/strcpcvt.h>
#include <xml/SXml.h>

SNSBEGIN

#ifndef ENABLE_SOBJ_XML
#ifdef _DEBUG
#define ENABLE_SOBJ_XML 1
#else
#define ENABLE_SOBJ_XML 0
#endif
#endif

/**
 * @class SObjectImpl
 * @brief Template class for implementing the SObject interface.
 *
 * @details This class provides runtime type identification and attribute dispatching based on XML descriptions.
 * @tparam T Base class that implements the IObject interface.
 */
template<class T>
class SObjectImpl : public T
{
public:
    /**
     * @brief Retrieves the class type.
     * @return Class type identifier.
     */
    static int GetClassType() {
        return T::GetClassType();
    }

    /**
     * @brief Retrieves the class name.
     * @return Class name as a wide string.
     */
    static LPCWSTR GetClassName() {
        return T::GetClassName();
    }

    /**
     * @brief Retrieves the class alias.
     * @return Class alias as a wide string.
     */
    static LPCWSTR GetClassAlias() {
        return T::GetClassAlias();
    }

public:
    /**
     * @brief Marks an attribute as handled.
     * @param xmlAttr XML attribute to mark.
     * @param bHandled TRUE if the attribute is handled, FALSE otherwise.
     */
    static void MarkAttributeHandled(SXmlAttr xmlAttr, bool bHandled) {
        xmlAttr.set_userdata(bHandled ? 1 : 0);
    }

    /**
     * @brief Checks if an attribute is handled.
     * @param xmlAttr XML attribute to check.
     * @return TRUE if the attribute is handled, FALSE otherwise.
     */
    static bool IsAttributeHandled(SXmlAttr xmlAttr) {
        return xmlAttr.get_userdata() == 1 ? true : false;
    }

    /**
     * @brief Constructor.
     */
    SObjectImpl() : m_attrHandler(NULL), m_nID(0) {}

public:
    /**
     * @brief Initializes the object from an XML node.
     * @param pXmlNode Pointer to the XML node containing the object's attributes.
     * @return TRUE if initialization is successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode * pXmlNode) OVERRIDE {
        SXmlNode xmlNode(pXmlNode);

        if (!xmlNode) return FALSE;
#if ENABLE_SOBJ_XML
        {
            pXmlNode->ToString(&m_strXml);
        }
#endif

        // Set current object's attributes
        for (SXmlAttr attr = xmlNode.first_attribute(); attr; attr = attr.next_attribute()) {
            if (IsAttributeHandled(attr)) continue;   // Ignore already processed attributes
            SetAttribute(attr.name(), attr.value(), TRUE);
        }

        // Call initialization finished interface
        OnInitFinished(pXmlNode);
        return TRUE;
    }

    /**
     * @brief Retrieves the object's name.
     * @return Object's name as a wide string.
     */
    STDMETHOD_(LPCWSTR, GetName)(THIS) SCONST OVERRIDE {
        return m_strName;
    }

    /**
     * @brief Retrieves the object's name in ANSI format.
     * @return Object's name as an ANSI string.
     */
    STDMETHOD_(LPCSTR, GetNameA)(THIS) SCONST OVERRIDE {
        static SStringA str;
        str = S_CW2A(m_strName, CP_UTF8);
        return str.c_str();
    }

    /**
     * @brief Sets the object's name.
     * @param pszName Name to set.
     */
    STDMETHOD_(void, SetName)(THIS_ LPCWSTR pszName) OVERRIDE {
        m_strName = pszName;
    }

    /**
     * @brief Retrieves the object's ID.
     * @return Object's ID.
     */
    STDMETHOD_(int, GetID)(THIS) SCONST OVERRIDE {
        return m_nID;
    }

    /**
     * @brief Sets the object's ID.
     * @param nID ID to set.
     */
    STDMETHOD_(void, SetID)(THIS_ int nID) OVERRIDE {
        m_nID = nID;
    }

    /**
     * @brief Handles attribute processing after setting an attribute.
     * @param strAttribName Attribute name.
     * @param strValue Attribute value.
     * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
     * @param hr Result of the attribute setting.
     * @return Result of the attribute processing.
     */
    STDMETHOD_(HRESULT, AfterAttribute)(THIS_ LPCWSTR strAttribName, LPCWSTR strValue, BOOL bLoading, HRESULT hr) OVERRIDE {
        UNREFERENCED_PARAMETER(strAttribName);
        UNREFERENCED_PARAMETER(strValue);
        UNREFERENCED_PARAMETER(bLoading);
        return hr;
    }

    /**
     * @brief Sets an attribute using ANSI strings.
     * @param pszAttr Attribute name.
     * @param pszValue Attribute value.
     * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
     * @return Result of setting the attribute.
     */
    STDMETHOD_(HRESULT, SetAttributeA)(THIS_ LPCSTR pszAttr, LPCSTR pszValue, BOOL bLoading) OVERRIDE {
        return SetAttribute(S_CA2W(pszAttr, CP_UTF8), S_CA2W(pszValue, CP_UTF8), bLoading);
    }

    /**
     * @brief Sets an attribute using wide strings.
     * @param pszAttr Attribute name.
     * @param pszValue Attribute value.
     * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
     * @return Result of setting the attribute.
     */
    STDMETHOD_(HRESULT, SetAttribute)(THIS_ LPCWSTR pszAttr, LPCWSTR pszValue, BOOL bLoading) OVERRIDE {
        SStringW strName(pszAttr), strValue(pszValue);
        if (m_attrHandler) {
            HRESULT hr = m_attrHandler(this, &strName, &strValue, bLoading);
            if (SUCCEEDED(hr))
                return hr;
        }

        return SetAttribute(strName, strValue, bLoading);
    }

    /**
     * @brief Sets an attribute using IStringW objects.
     * @param strAttr Attribute name.
     * @param strValue Attribute value.
     * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
     * @return Result of setting the attribute.
     */
    STDMETHOD_(HRESULT, ISetAttribute)(THIS_ const IStringW *strAttr, const IStringW *strValue, BOOL bLoading) OVERRIDE {
        if (m_attrHandler) {
            HRESULT hr = m_attrHandler(this, strAttr, strValue, bLoading);
            if (SUCCEEDED(hr))
                return hr;
        }

        return SetAttribute(SStringW(strAttr), SStringW(strValue), bLoading);
    }

    /**
     * @brief Retrieves the object's class name.
     * @return Class name as a wide string.
     */
    STDMETHOD_(LPCWSTR, GetObjectClass)(THIS_) SCONST OVERRIDE {
        return GetClassName();
    }

    /**
     * @brief Retrieves the object's type.
     * @return Object type identifier.
     */
    STDMETHOD_(int, GetObjectType)(THIS) SCONST OVERRIDE {
        return GetClassType();
    }

    /**
     * @brief Checks if the object is of a specific class.
     * @param lpszName Class name to check.
     * @return TRUE if the object is of the specified class, FALSE otherwise.
     */
    STDMETHOD_(BOOL, IsClass)(THIS_ LPCWSTR lpszName) SCONST OVERRIDE {
        return wcscmp(lpszName, GetClassName()) == 0;
    }

    /**
     * @brief Retrieves an attribute value.
     * @param strAttr Attribute name.
     * @param pValue Pointer to store the attribute value.
     * @return TRUE if the attribute is retrieved successfully, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetAttribute)(THIS_ LPCWSTR strAttr, IStringW *pValue) SCONST OVERRIDE {
        UNREFERENCED_PARAMETER(strAttr);
        UNREFERENCED_PARAMETER(pValue);
        return FALSE;
    }

    /**
     * @brief Handles initialization completion.
     * @param xmlNode XML node containing the object's attributes.
     */
    STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *xmlNode) OVERRIDE {
        UNREFERENCED_PARAMETER(xmlNode);
    }

    /**
     * @brief Sets the attribute handler.
     * @param attrHandler Function pointer to the attribute handler.
     */
    STDMETHOD_(void, SetAttrHandler)(THIS_ FunAttrHandler attrHandler) OVERRIDE {
        m_attrHandler = attrHandler;
    }

public:
    /**
     * @brief Sets an attribute using wide strings.
     * @param strAttr Attribute name.
     * @param strValue Attribute value.
     * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
     * @return Result of setting the attribute.
     */
    virtual HRESULT SetAttribute(const SStringW &strAttr, const SStringW &strValue, BOOL bLoading) {
        return DefAttributeProc(strAttr, strValue, bLoading);
    }

    /**
     * @brief Default attribute processing.
     * @param strAttr Attribute name.
     * @param strValue Attribute value.
     * @param bLoading TRUE if the object is being loaded, FALSE otherwise.
     * @return Result of default attribute processing.
     */
    virtual HRESULT DefAttributeProc(const SStringW &strAttr, const SStringW &strValue, BOOL bLoading) {
        if (strAttr.CompareNoCase(L"name") == 0)
            m_strName = strValue;
        else if (strAttr.CompareNoCase(L"id") == 0)
            m_nID = _wtoi(strValue);
        return E_FAIL;
    }

#if ENABLE_SOBJ_XML
public:
    SStringW m_strXml;  /**< XML string for debugging purposes */
#endif // ENABLE_SOBJ_XML

protected:
    SStringW m_strName;       /**< Object's name */
    int m_nID;                /**< Object's ID */

    FunAttrHandler m_attrHandler;  /**< Attribute handler function */
};

/**
 * @typedef SObject
 * @brief Typedef for SObjectImpl<IObject>.
 */
typedef SObjectImpl<IObject> SObject;

SNSEND

#endif // __SOBJECT_I_H_