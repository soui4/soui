 //! Add reference
 /*!
  * @brief Increments the reference count of the object.
  *
  * @return long The new reference count.
  */
 STDMETHOD_(long, AddRef)(THIS) PURE;
 
 //! Release reference
 /*!
  * @brief Decrements the reference count of the object.
  *
  * @return long The new reference count.
  */
 STDMETHOD_(long, Release)(THIS) PURE;
 
 //! Final release of the object
 /*!
  * @brief Final release of the object, performing cleanup if necessary.
  */
 STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
 
 /**
  * @brief Checks if the object is of a specified class type.
  *
  * @param lpszName Name of the class type to test.
  * @return BOOL TRUE if the object is of the specified class type, FALSE otherwise.
  */
 STDMETHOD_(BOOL, IsClass)(CTHIS_ LPCWSTR lpszName) SCONST PURE;
 
 /**
  * @brief Retrieves the class name of the object.
  *
  * @return LPCWSTR Class name of the object.
  *
  * @details This is a virtual function. Note the difference between this and GetClassName.
  */
 STDMETHOD_(LPCWSTR, GetObjectClass)(CTHIS) SCONST PURE;
 
 /**
  * @brief Retrieves the type of the object.
  *
  * @return int Type of the object.
  *
  * @details This is a virtual function. Note the difference between this and GetClassType.
  */
 STDMETHOD_(int, GetObjectType)(CTHIS) SCONST PURE;
 
 /**
  * @brief Retrieves the ID of the object.
  *
  * @return int ID of the object.
  */
 STDMETHOD_(int, GetID)(CTHIS) SCONST PURE;
 
 /**
  * @brief Sets the ID of the object.
  *
  * @param nID ID to set for the object.
  */
 STDMETHOD_(void, SetID)(THIS_ int nID) PURE;
 
 /**
  * @brief Retrieves the name of the object.
  *
  * @return LPCWSTR Name of the object.
  */
 STDMETHOD_(LPCWSTR, GetName)(CTHIS) SCONST PURE;
 
 /**
  * @brief Retrieves the name of the object as an ANSI string.
  *
  * @return LPCSTR Name of the object.
  */
 STDMETHOD_(LPCSTR, GetNameA)(CTHIS) SCONST PURE;
 
 /**
  * @brief Sets the name of the object.
  *
  * @param pszName Name to set for the object.
  */
 STDMETHOD_(void, SetName)(THIS_ LPCWSTR pszName) PURE;
 
 /**
  * @brief Initializes the object from an XML node.
  *
  * @param xmlNode XML node containing the initialization data.
  * @return BOOL TRUE if initialization is successful, FALSE otherwise.
  */
 STDMETHOD_(BOOL, InitFromXml)(THIS_ IXmlNode *xmlNode) PURE;
 
 /**
  * @brief Handles the completion of attribute initialization.
  *
  * @param xmlNode XML node containing the attribute data.
  */
 STDMETHOD_(void, OnInitFinished)(THIS_ IXmlNode *xmlNode) PURE;
 
 /**
  * @brief Sets an attribute of the object.
  *
  * @param strAttribName Name of the attribute.
  * @param strValue Value of the attribute.
  * @param bLoading Flag indicating if the object is being created by the system.
  * @return HRESULT Result of the attribute setting operation.
  */
 STDMETHOD_(HRESULT, ISetAttribute)
 (THIS_ const IStringW *strAttribName, const IStringW *strValue, BOOL bLoading) PURE;
 
 /**
  * @brief Sets an attribute of the object using ANSI strings.
  *
  * @param pszAttr Name of the attribute.
  * @param pszValue Value of the attribute.
  * @param bLoading Flag indicating if the object is being created by the system.
  * @return HRESULT Result of the attribute setting operation.
  */
 STDMETHOD_(HRESULT, SetAttributeA)(THIS_ LPCSTR pszAttr, LPCSTR pszValue, BOOL bLoading DEF_VAL(FALSE)) PURE;
 
 /**
  * @brief Sets an attribute of the object using wide strings.
  *
  * @param pszAttr Name of the attribute.
  * @param pszValue Value of the attribute.
  * @param bLoading Flag indicating if the object is being created by the system.
  * @return HRESULT Result of the attribute setting operation.
  */
 STDMETHOD_(HRESULT, SetAttribute)(THIS_ LPCWSTR pszAttr, LPCWSTR pszValue, BOOL bLoading DEF_VAL(FALSE)) PURE;
 
 /**
  * @brief Retrieves the value of an attribute by its name.
  *
  * @param strAttr Name of the attribute.
  * @param pValue Pointer to receive the attribute value.
  * @return BOOL TRUE if the attribute is successfully retrieved, FALSE otherwise.
  */
 STDMETHOD_(BOOL, GetAttribute)(CTHIS_ LPCWSTR strAttr, IStringW *pValue) SCONST PURE;
 
 /**
  * @brief Handles the completion of attribute processing.
  *
  * @param pszAttr Name of the attribute.
  * @param pszValue Value of the attribute.
  * @param bLoading Flag indicating if the object is being created by the system.
  * @param hr Result of the attribute processing.
  * @return HRESULT Result of the attribute processing.
  */
 STDMETHOD_(HRESULT, AfterAttribute)
 (THIS_ LPCWSTR pszAttr, LPCWSTR pszValue, BOOL bLoading, HRESULT hr) PURE;
 
 /**
  * @brief Sets the attribute handler callback function.
  *
  * @param attrHandler Callback function for attribute handling.
  */
 STDMETHOD_(void, SetAttrHandler)(THIS_ FunAttrHandler attrHandler) PURE;
 