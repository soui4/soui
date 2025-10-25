#ifndef __SXML_I__H__
#define __SXML_I__H__

#include <interface/obj-ref-i.h>
#include <interface/sstring-i.h>
#include <stdio.h>

SNSBEGIN

/**
 * @interface IXmlAttr
 * @brief Interface for XML attributes.
 */
#undef INTERFACE
#define INTERFACE IXmlAttr
DECLARE_INTERFACE(IXmlAttr)
{
    /**
     * @brief Gets the private pointer associated with the attribute.
     * @return Pointer to private data.
     */
    STDMETHOD_(LPVOID, GetPrivPtr)(CTHIS) SCONST PURE;

    /**
     * @brief Checks if the attribute is empty.
     * @return TRUE if the attribute is empty, FALSE otherwise.
     */
    STDMETHOD_(BOOL, Empty)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the name of the attribute.
     * @return The name of the attribute, or an empty string if the attribute is empty.
     */
    STDMETHOD_(const wchar_t*, Name)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the value of the attribute.
     * @return The value of the attribute, or an empty string if the attribute is empty.
     */
    STDMETHOD_(const wchar_t*, Value)(CTHIS) SCONST PURE;

    /**
     * @brief Sets user-defined data associated with the attribute.
     * @param data User-defined data.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, set_userdata)(THIS_ int data) PURE;

    /**
     * @brief Gets user-defined data associated with the attribute.
     * @return User-defined data.
     */
    STDMETHOD_(int, get_userdata)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the next attribute in the attribute list of the parent node.
     * @return Pointer to the next attribute, or NULL if there is no next attribute.
     */
    STDMETHOD_(IXmlAttr*, Next)(THIS) PURE;

    /**
     * @brief Gets the previous attribute in the attribute list of the parent node.
     * @return Pointer to the previous attribute, or NULL if there is no previous attribute.
     */
    STDMETHOD_(IXmlAttr*, Prev)(THIS) PURE;

    /**
     * @brief Converts the attribute value to an integer.
     * @param def Default value to return if conversion fails.
     * @return The integer value of the attribute, or the default value if conversion fails.
     */
    STDMETHOD_(int, AsInt)(THIS_ int def DEF_VAL(0)) PURE;

    /**
     * @brief Converts the attribute value to an unsigned integer.
     * @param def Default value to return if conversion fails.
     * @return The unsigned integer value of the attribute, or the default value if conversion fails.
     */
    STDMETHOD_(unsigned int, AsUint)(THIS_ int def DEF_VAL(0)) PURE;

    /**
     * @brief Converts the attribute value to a float.
     * @param def Default value to return if conversion fails.
     * @return The float value of the attribute, or the default value if conversion fails.
     */
    STDMETHOD_(float, AsFloat)(THIS_ float def DEF_VAL(0.0f)) PURE;

    /**
     * @brief Converts the attribute value to a double.
     * @param def Default value to return if conversion fails.
     * @return The double value of the attribute, or the default value if conversion fails.
     */
    STDMETHOD_(double, AsDouble)(THIS_ double def DEF_VAL(0.0)) PURE;

    /**
     * @brief Converts the attribute value to a boolean.
     * @param def Default value to return if conversion fails.
     * @return The boolean value of the attribute, or the default value if conversion fails.
     */
    STDMETHOD_(BOOL, AsBool)(THIS_ BOOL def DEF_VAL(FALSE)) PURE;
};

/**
 * @enum _XmlNodeType
 * @brief Enumerates the types of XML nodes.
 */
typedef enum _XmlNodeType
{
    node_null,          /**< Empty (null) node handle. */
    node_document,      /**< A document tree's absolute root. */
    node_element,       /**< Element tag, e.g., `<node/>`. */
    node_pcdata,        /**< Plain character data, e.g., `text`. */
    node_cdata,         /**< Character data, e.g., `<![CDATA[text]]>`. */
    node_comment,       /**< Comment tag, e.g., `<!-- text -->`. */
    node_pi,            /**< Processing instruction, e.g., `<?name?>`. */
    node_declaration,   /**< Document declaration, e.g., `<?xml version="1.0"?>`. */
    node_doctype        /**< Document type declaration, e.g., `<!DOCTYPE doc>`. */
} XmlNodeType;


/**
 * @interface IXmlNode
 * @brief Interface for XML nodes.
 */
#undef INTERFACE
#define INTERFACE IXmlNode
DECLARE_INTERFACE(IXmlNode)
{
    /**
     * @brief Converts the node to a string representation.
     * @param out Pointer to the output string.
     */
    STDMETHOD_(void, ToString)(CTHIS_ IStringW *out) SCONST PURE;

    /**
     * @brief Gets the private pointer associated with the node.
     * @return Pointer to private data.
     */
    STDMETHOD_(LPVOID, GetPrivPtr)(CTHIS) SCONST PURE;

    /**
     * @brief Checks if the node is empty.
     * @return TRUE if the node is empty, FALSE otherwise.
     */
    STDMETHOD_(BOOL, Empty)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the name of the node.
     * @return The name of the node.
     */
    STDMETHOD_(const wchar_t*, Name)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the value of the node.
     * @return The value of the node.
     */
    STDMETHOD_(const wchar_t*, Value)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the text content of the node.
     * @return The text content of the node.
     */
    STDMETHOD_(const wchar_t*, Text)(CTHIS) SCONST PURE;

    /**
     * @brief Sets the name of the node.
     * @param name The name of the node.
     * @return S_OK if successful, or an error code otherwise.
     */
    STDMETHOD_(BOOL, SetText)(THIS_ const wchar_t* text) PURE;
    
    /**
     * @brief Sets user-defined data associated with the node.
     * @param data User-defined data.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, set_userdata)(THIS_ int data) PURE;

    /**
     * @brief Gets user-defined data associated with the node.
     * @return User-defined data.
     */
    STDMETHOD_(int, get_userdata)(CTHIS) SCONST PURE;

    /**
     * @brief Gets an attribute by name.
     * @param name Name of the attribute.
     * @param bCaseSensitive TRUE if the search is case-sensitive, FALSE otherwise.
     * @return Pointer to the attribute, or NULL if not found.
     */
    STDMETHOD_(IXmlAttr*, Attribute)(CTHIS_ const wchar_t* name, BOOL bCaseSensitive) SCONST PURE;

    /**
     * @brief Gets the first attribute in the attribute list.
     * @return Pointer to the first attribute, or NULL if there are no attributes.
     */
    STDMETHOD_(IXmlAttr*, FirstAttribute)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the last attribute in the attribute list.
     * @return Pointer to the last attribute, or NULL if there are no attributes.
     */
    STDMETHOD_(IXmlAttr*, LastAttribute)(CTHIS) SCONST PURE;

    /**
     * @brief Gets a child node by name.
     * @param name Name of the child node.
     * @param bCaseSensitive TRUE if the search is case-sensitive, FALSE otherwise.
     * @return Pointer to the child node, or NULL if not found.
     */
    STDMETHOD_(IXmlNode*, Child)(CTHIS_ const wchar_t* name, BOOL bCaseSensitive) SCONST PURE;

    /**
     * @brief Gets the first child node.
     * @return Pointer to the first child node, or NULL if there are no child nodes.
     */
    STDMETHOD_(IXmlNode*, FirstChild)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the last child node.
     * @return Pointer to the last child node, or NULL if there are no child nodes.
     */
    STDMETHOD_(IXmlNode*, LastChild)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the next sibling node in the children list of the parent node.
     * @return Pointer to the next sibling node, or NULL if there is no next sibling.
     */
    STDMETHOD_(IXmlNode *, NextSibling)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the previous sibling node in the children list of the parent node.
     * @return Pointer to the previous sibling node, or NULL if there is no previous sibling.
     */
    STDMETHOD_(IXmlNode *, PrevSibling)(CTHIS) SCONST PURE;

    /**
     * @brief Gets the next sibling node with the specified name.
     * @param name Name of the sibling node.
     * @param bCaseSensitive TRUE if the search is case-sensitive, FALSE otherwise.
     * @return Pointer to the next sibling node with the specified name, or NULL if not found.
     */
    STDMETHOD_(IXmlNode *, NextSibling2)(CTHIS_ const wchar_t *name, BOOL bCaseSensitive) SCONST PURE;

    /**
     * @brief Gets the previous sibling node with the specified name.
     * @param name Name of the sibling node.
     * @param bCaseSensitive TRUE if the search is case-sensitive, FALSE otherwise.
     * @return Pointer to the previous sibling node with the specified name, or NULL if not found.
     */
    STDMETHOD_(IXmlNode *, PrevSibling2)(CTHIS_ const wchar_t *name, BOOL bCaseSensitive) SCONST PURE;

    /**
     * @brief Appends a child node with the specified name.
     * @param name Name of the child node.
     * @return Pointer to the newly appended child node.
     */
    STDMETHOD_(IXmlNode*, AppendChild)(THIS_ const wchar_t* name) PURE;

    /**
     * @brief Prepends a child node with the specified name.
     * @param name Name of the child node.
     * @return Pointer to the newly prepended child node.
     */
    STDMETHOD_(IXmlNode*, PrependChild)(THIS_ const wchar_t* name) PURE;

    /**
     * @brief Appends a copy of the specified node as a child.
     * @param proto Pointer to the node to copy.
     * @return Pointer to the newly appended child node.
     */
    STDMETHOD_(IXmlNode*, AppendCopyNode)(THIS_ const IXmlNode* proto) PURE;

    /**
     * @brief Prepends a copy of the specified node as a child.
     * @param proto Pointer to the node to copy.
     * @return Pointer to the newly prepended child node.
     */
    STDMETHOD_(IXmlNode*, PrependCopyNode)(THIS_ const IXmlNode* proto) PURE;

    /**
     * @brief Appends an attribute with the specified name.
     * @param name Name of the attribute.
     * @return Pointer to the newly appended attribute.
     */
    STDMETHOD_(IXmlAttr*, AppendAttribute)(THIS_ const wchar_t* name) PURE;

    /**
     * @brief Prepends an attribute with the specified name.
     * @param name Name of the attribute.
     * @return Pointer to the newly prepended attribute.
     */
    STDMETHOD_(IXmlAttr*, PrependAttribute)(THIS_ const wchar_t* name) PURE;

    /**
     * @brief Appends a copy of the specified attribute.
     * @param proto Pointer to the attribute to copy.
     * @return Pointer to the newly appended attribute.
     */
    STDMETHOD_(IXmlAttr*, AppendCopyAttribute)(THIS_ const IXmlAttr* proto) PURE;

    /**
     * @brief Prepends a copy of the specified attribute.
     * @param proto Pointer to the attribute to copy.
     * @return Pointer to the newly prepended attribute.
     */
    STDMETHOD_(IXmlAttr*, PrependCopyAttribute)(THIS_ const IXmlAttr* proto) PURE;

    /**
     * @brief Removes an attribute by name.
     * @param name Name of the attribute to remove.
     * @return TRUE if the attribute was removed, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RemoveAttribute)(THIS_ const wchar_t* name) PURE;

    /**
     * @brief Removes a child node by name.
     * @param name Name of the child node to remove.
     * @return TRUE if the child node was removed, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RemoveChild)(THIS_ const wchar_t* name) PURE;

    /**
     * @brief Removes all child nodes.
     * @return TRUE if all child nodes were removed, FALSE otherwise.
     */
    STDMETHOD_(BOOL, RemoveAllChilden)(THIS) PURE;
};

/**
 * @enum _XmlStatus
 * @brief Enumerates the possible parsing statuses.
 */
typedef enum _XmlStatus
{
    xml_ok = 0,                 /**< No error. */
    xml_file_not_found,         /**< File was not found during load_file(). */
    xml_io_error,               /**< Error reading from file/stream. */
    xml_out_of_memory,          /**< Could not allocate memory. */
    xml_internal_error,         /**< Internal error occurred. */
    xml_unrecognized_tag,       /**< Parser could not determine tag type. */
    xml_bad_pi,                 /**< Parsing error occurred while parsing document declaration/processing instruction. */
    xml_bad_comment,            /**< Parsing error occurred while parsing comment. */
    xml_bad_cdata,              /**< Parsing error occurred while parsing CDATA section. */
    xml_bad_doctype,            /**< Parsing error occurred while parsing document type declaration. */
    xml_bad_pcdata,             /**< Parsing error occurred while parsing PCDATA section. */
    xml_bad_start_element,      /**< Parsing error occurred while parsing start element tag. */
    xml_bad_attribute,          /**< Parsing error occurred while parsing element attribute. */
    xml_bad_end_element,        /**< Parsing error occurred while parsing end element tag. */
    xml_end_element_mismatch,   /**< There was a mismatch of start-end tags (closing tag had incorrect name, some tag was not closed or there was an excessive closing tag). */
    xml_append_invalid_root,    /**< Unable to append nodes since root type is not node_element or node_document (exclusive to xml_node::append_buffer). */
    xml_no_document_element     /**< Parsing resulted in a document without element nodes. */
} XmlStatus;

/**
 * @enum _XmlEncoding
 * @brief Enumerates the possible XML encodings.
 */
typedef enum _XmlEncoding
{
    enc_auto,       /**< Auto-detect input encoding using BOM or <? / < detection; use UTF8 if BOM is not found. */
    enc_utf8,       /**< UTF8 encoding. */
    enc_utf16_le,   /**< Little-endian UTF16. */
    enc_utf16_be,   /**< Big-endian UTF16. */
    enc_utf16,      /**< UTF16 with native endianness. */
    enc_utf32_le,   /**< Little-endian UTF32. */
    enc_utf32_be,   /**< Big-endian UTF32. */
    enc_utf32,      /**< UTF32 with native endianness. */
    enc_wchar,      /**< The same encoding wchar_t has (either UTF16 or UTF32). */
    enc_latin1,
    enc_bin         /**< Binary XML. */
} XmlEncoding;

/**
 * @enum _XmlParseOpt
 * @brief Enumerates the parsing options.
 */
typedef enum _XmlParseOpt
{
    // Minimal parsing mode (equivalent to turning all other flags off).
    // Only elements and PCDATA sections are added to the DOM tree, no text conversions are performed.
    xml_parse_minimal = 0x0000,

    // This flag determines if processing instructions (node_pi) are added to the DOM tree. This flag is off by default.
    xml_parse_pi = 0x0001,

    // This flag determines if comments (node_comment) are added to the DOM tree. This flag is off by default.
    xml_parse_comments = 0x0002,

    // This flag determines if CDATA sections (node_cdata) are added to the DOM tree. This flag is on by default.
    xml_parse_cdata = 0x0004,

    // This flag determines if plain character data (node_pcdata) that consist only of whitespace are added to the DOM tree.
    // This flag is off by default; turning it on usually results in slower parsing and more memory consumption.
    xml_parse_ws_pcdata = 0x0008,

    // This flag determines if character and entity references are expanded during parsing. This flag is on by default.
    xml_parse_escapes = 0x0010,

    // This flag determines if EOL characters are normalized (converted to #xA) during parsing. This flag is on by default.
    xml_parse_eol = 0x0020,

    // This flag determines if attribute values are normalized using CDATA normalization rules during parsing. This flag is on by default.
    xml_parse_wconv_attribute = 0x0040,

    // This flag determines if attribute values are normalized using NMTOKENS normalization rules during parsing. This flag is off by default.
    xml_parse_wnorm_attribute = 0x0080,

    // This flag determines if document declaration (node_declaration) is added to the DOM tree. This flag is off by default.
    xml_parse_declaration = 0x0100,

    // This flag determines if document type declaration (node_doctype) is added to the DOM tree. This flag is off by default.
    xml_parse_doctype = 0x0200,

    // This flag determines if plain character data (node_pcdata) that is the only child of the parent node and that consists only
    // of whitespace is added to the DOM tree.
    // This flag is off by default; turning it on may result in slower parsing and more memory consumption.
    xml_parse_ws_pcdata_single = 0x0400,

    // This flag determines if leading and trailing whitespace is to be removed from plain character data. This flag is off by default.
    xml_parse_trim_pcdata = 0x0800,
    /**
     * @brief This flag determines if plain character data that does not have a parent node is added to the DOM tree,
     * and if an empty document is a valid document. This flag is off by default.
     */
    xml_parse_fragment = 0x1000,

    /**
     * @brief This flag determines if plain character data is stored in the parent element's value. This significantly changes
     * the structure of the document; this flag is only recommended for parsing documents with many PCDATA nodes in
     * memory-constrained environments. This flag is off by default.
     */
    xml_parse_embed_pcdata = 0x2000,

    /**
     * @brief The default parsing mode.
     * Elements, PCDATA, and CDATA sections are added to the DOM tree, character/reference entities are expanded,
     * End-of-Line characters are normalized, and attribute values are normalized using CDATA normalization rules.
     */
    xml_parse_default = xml_parse_cdata | xml_parse_escapes | xml_parse_wconv_attribute | xml_parse_eol,

    /**
     * @brief The full parsing mode.
     * Nodes of all types are added to the DOM tree, character/reference entities are expanded,
     * End-of-Line characters are normalized, and attribute values are normalized using CDATA normalization rules.
     */
    xml_parse_full = xml_parse_default | xml_parse_pi | xml_parse_comments | xml_parse_declaration | xml_parse_doctype,
} XmlParseOpt;

/**
 * @struct _XmlParseResult
 * @brief Structure containing the result of XML parsing.
 */
typedef struct _XmlParseResult
{
    /**
     * @brief Parsing status (see XmlStatus).
     */
    XmlStatus status;

    /**
     * @brief Last parsed offset (in char_t units from the start of input data).
     */
    ptrdiff_t offset;

    /**
     * @brief Source document encoding.
     */
    XmlEncoding encoding;
} XmlParseResult;

#undef INTERFACE
#define INTERFACE IXmlDoc
/**
 * @interface IXmlDoc
 * @brief Interface for XML Document.
 */
DECLARE_INTERFACE_(IXmlDoc, IObjRef)
{
    /**
     * @brief Adds a reference to the document.
     * @return The new reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Releases a reference to the document.
     * @return The new reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Releases the document object.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Gets the private pointer associated with the document.
     * @return Pointer to private data.
     */
    STDMETHOD_(LPVOID, GetPrivPtr)(CTHIS) SCONST PURE;

    /**
     * @brief Resets the document, removing all nodes.
     */
    STDMETHOD_(void, Reset)(THIS) PURE;

    /**
     * @brief Removes all nodes, then copies the entire contents of the specified document.
     * @param proto Pointer to the document to copy.
     */
    STDMETHOD_(void, Copy)(THIS_ const IXmlDoc* proto) PURE;

    /**
     * @brief Loads the document from a zero-terminated string. No encoding conversions are applied.
     * @param contents Zero-terminated string containing the XML data.
     * @param options Parsing options.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, LoadString)(THIS_ const wchar_t* contents, unsigned int options) PURE;

    /**
     * @brief Loads the document from a file (ANSI version).
     * @param path Path to the file.
     * @param options Parsing options.
     * @param encoding Document encoding.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, LoadFileA)(THIS_ const char* path, unsigned int options, XmlEncoding encoding) PURE;

    /**
     * @brief Loads the document from a file (Unicode version).
     * @param path Path to the file.
     * @param options Parsing options.
     * @param encoding Document encoding.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, LoadFileW)(THIS_ const wchar_t* path, unsigned int options, XmlEncoding encoding) PURE;

    /**
     * @brief Loads the document from a buffer. Copies/converts the buffer, so it may be deleted or changed after the function returns.
     * @param contents Pointer to the buffer containing the XML data.
     * @param size Size of the buffer.
     * @param options Parsing options.
     * @param encoding Document encoding.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, LoadBuffer)(THIS_ const void* contents, size_t size, unsigned int options, XmlEncoding encoding) PURE;

    /**
     * @brief Loads the document from a buffer, using the buffer for in-place parsing (the buffer is modified and used for storage of document data).
     * You should ensure that buffer data will persist throughout the document's lifetime, and free the buffer memory manually once the document is destroyed.
     * @param contents Pointer to the buffer containing the XML data.
     * @param size Size of the buffer.
     * @param options Parsing options.
     * @param encoding Document encoding.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, LoadBufferInplace)(THIS_ void* contents, size_t size, unsigned int options, XmlEncoding encoding) PURE;

    /**
     * @brief Loads the document from a buffer, using the buffer for in-place parsing (the buffer is modified and used for storage of document data).
     * You should allocate the buffer with pugixml allocation function; the document will free the buffer when it is no longer needed (you can't use it anymore).
     * @param contents Pointer to the buffer containing the XML data.
     * @param size Size of the buffer.
     * @param options Parsing options.
     * @param encoding Document encoding.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, LoadBufferInplaceOwn)(THIS_ void* contents, size_t size, unsigned int options, XmlEncoding encoding) PURE;

    /**
     * @brief Gets the parsing result.
     * @param pResult Pointer to the XmlParseResult structure to store the result.
     */
    STDMETHOD_(void, GetParseResult)(CTHIS_ XmlParseResult *pResult) SCONST PURE;

    /**
     * @brief Saves the XML document to a writer (semantics is slightly different from xml_node::print, see documentation for details).
     * @param f Pointer to the file stream.
     */
    STDMETHOD_(void, SaveBinary)(CTHIS_ FILE *f) SCONST PURE;

    /**
     * @brief Saves the XML document to a file (ANSI version).
     * @param path Path to the file.
     * @param indent Indentation string.
     * @param flags Saving flags.
     * @param encoding Document encoding.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, SaveFileA)(CTHIS_ const char* path, const wchar_t* indent, unsigned int flags, XmlEncoding encoding) SCONST PURE;

    /**
     * @brief Saves the XML document to a file (Unicode version).
     * @param path Path to the file.
     * @param indent Indentation string.
     * @param flags Saving flags.
     * @param encoding Document encoding.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, SaveFileW)(CTHIS_ const wchar_t* path, const wchar_t* indent, unsigned int flags, XmlEncoding encoding) SCONST PURE;

    /**
     * @brief Gets the document element (root node).
     * @return Pointer to the root node.
     */
    STDMETHOD_(IXmlNode *, Root)(CTHIS) SCONST PURE;
};

SNSEND

#ifdef __cplusplus
typedef SOUI::IXmlDoc * IXmlDocPtr;
#else
typedef IXmlDoc * IXmlDocPtr;
#endif

#endif // __SXML_I__H__