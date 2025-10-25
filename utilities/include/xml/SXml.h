#ifndef __SXML__H__
#define __SXML__H__

#include <utilities.h>
#include <utilities-def.h>
#include <interface/sxml-i.h>
#include <pugixml/pugixml.hpp>
#include <souicoll.h>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

/**
 * @class SXmlAttr
 * @brief Class representing an XML attribute.
 * 
 * This class provides methods to manipulate and access XML attributes.
 */
class UTILITIES_API SXmlAttr : public IXmlAttr
{
    friend class SXmlNode;

private:
    pugi::xml_attribute _attr;

private:
    /**
     * @brief Converts a pugi::xml_attribute to an IXmlAttr pointer.
     * 
     * @param attr The pugi::xml_attribute to convert.
     * @return Pointer to the converted IXmlAttr.
     */
    static IXmlAttr *toIXmlAttr(pugi::xml_attribute attr);

public:
    /**
     * @brief Constructor for SXmlAttr.
     * 
     * @param attr The pugi::xml_attribute to initialize with.
     */
    SXmlAttr(pugi::xml_attribute attr);

    /**
     * @brief Constructor for SXmlAttr from an IXmlAttr pointer.
     * 
     * @param src The IXmlAttr pointer to initialize with.
     */
    SXmlAttr(const IXmlAttr * src);

    /**
     * @brief Copy constructor for SXmlAttr.
     * 
     * @param src The SXmlAttr to copy.
     */
    SXmlAttr(const SXmlAttr& src);

    /**
     * @brief Constructor for SXmlAttr with an optional private data pointer.
     * 
     * @param pData Pointer to private data.
     */
    explicit SXmlAttr(LPVOID pData = NULL);

public:
    /**
     * @brief Gets the private data pointer.
     * 
     * @return Pointer to the private data.
     */
    STDMETHOD_(LPVOID, GetPrivPtr)(THIS) SCONST OVERRIDE;

    /**
     * @brief Checks if the attribute is empty.
     * 
     * @return TRUE if the attribute is empty, FALSE otherwise.
     */
    STDMETHOD_(BOOL, Empty)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the attribute name.
     * 
     * @return Attribute name, or "" if the attribute is empty.
     */
    STDMETHOD_(const wchar_t*, Name)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the attribute value.
     * 
     * @return Attribute value, or "" if the attribute is empty.
     */
    STDMETHOD_(const wchar_t*, Value)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets user data for the attribute.
     * 
     * @param data User data to set.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, set_userdata)(THIS_ int data) OVERRIDE;

    /**
     * @brief Gets user data for the attribute.
     * 
     * @return User data.
     */
    STDMETHOD_(int, get_userdata)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the next attribute in the attribute list of the parent node.
     * 
     * @return Pointer to the next attribute.
     */
    STDMETHOD_(IXmlAttr*, Next)(THIS) OVERRIDE;

    /**
     * @brief Gets the previous attribute in the attribute list of the parent node.
     * 
     * @return Pointer to the previous attribute.
     */
    STDMETHOD_(IXmlAttr*, Prev)(THIS) OVERRIDE;

    /**
     * @brief Converts the attribute value to an integer.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Integer value of the attribute.
     */
    STDMETHOD_(int, AsInt)(THIS_ int def DEF_VAL(0)) OVERRIDE;

    /**
     * @brief Converts the attribute value to an unsigned integer.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Unsigned integer value of the attribute.
     */
    STDMETHOD_(unsigned int, AsUint)(THIS_ int def DEF_VAL(0)) OVERRIDE;

    /**
     * @brief Converts the attribute value to a float.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Float value of the attribute.
     */
    STDMETHOD_(float, AsFloat)(THIS_ float def DEF_VAL(0.0f)) OVERRIDE;

    /**
     * @brief Converts the attribute value to a double.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Double value of the attribute.
     */
    STDMETHOD_(double, AsDouble)(THIS_ double def DEF_VAL(0.0)) OVERRIDE;

    /**
     * @brief Converts the attribute value to a boolean.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Boolean value of the attribute.
     */
    STDMETHOD_(BOOL, AsBool)(THIS_ BOOL def DEF_VAL(FALSE)) OVERRIDE;

public:
    /**
     * @brief Checks if the attribute is empty.
     * 
     * @return TRUE if the attribute is empty, FALSE otherwise.
     */
    operator bool() const;

    /**
     * @brief Checks if the attribute is empty.
     * 
     * @return TRUE if the attribute is empty, FALSE otherwise.
     */
    bool empty() const;

    /**
     * @brief Gets the attribute name.
     * 
     * @return Attribute name, or "" if the attribute is empty.
     */
    const wchar_t* name() const;

    /**
     * @brief Gets the attribute value.
     * 
     * @return Attribute value, or "" if the attribute is empty.
     */
    const wchar_t* value() const;

    /**
     * @brief Gets the attribute value as a string.
     * 
     * @param def Default value if the attribute is empty.
     * @return Attribute value as a string.
     */
    const wchar_t* as_string(const wchar_t* def = L"") const;

    /**
     * @brief Converts the attribute value to an integer.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Integer value of the attribute.
     */
    int as_int(int def = 0) const;

    /**
     * @brief Converts the attribute value to an unsigned integer.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Unsigned integer value of the attribute.
     */
    unsigned int as_uint(unsigned int def = 0) const;

    /**
     * @brief Converts the attribute value to a double.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Double value of the attribute.
     */
    double as_double(double def = 0) const;

    /**
     * @brief Converts the attribute value to a float.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Float value of the attribute.
     */
    float as_float(float def = 0) const;

    /**
     * @brief Converts the attribute value to a boolean.
     * 
     * @param def Default value if conversion fails or attribute is empty.
     * @return Boolean value of the attribute.
     */
    bool as_bool(bool def = false) const;

    /**
     * @brief Sets the attribute name.
     * 
     * @param rhs New attribute name.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_name(const wchar_t* rhs);

    /**
     * @brief Sets the attribute value.
     * 
     * @param rhs New attribute value.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(const wchar_t* rhs);

    /**
     * @brief Sets the attribute value with type conversion (integer).
     * 
     * @param rhs New attribute value.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(int rhs);

    /**
     * @brief Sets the attribute value with type conversion (unsigned integer).
     * 
     * @param rhs New attribute value.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(unsigned int rhs);

    /**
     * @brief Sets the attribute value with type conversion (long).
     * 
     * @param rhs New attribute value.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(long rhs);

    /**
     * @brief Sets the attribute value with type conversion (unsigned long).
     * 
     * @param rhs New attribute value.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(unsigned long rhs);

    /**
     * @brief Sets the attribute value with type conversion (double).
     * 
     * @param rhs New attribute value.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(double rhs);

    /**
     * @brief Sets the attribute value with type conversion (double with precision).
     * 
     * @param rhs New attribute value.
     * @param precision Precision for the conversion.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(double rhs, int precision);

    /**
     * @brief Sets the attribute value with type conversion (float).
     * 
     * @param rhs New attribute value.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(float rhs);

    /**
     * @brief Sets the attribute value with type conversion (float with precision).
     * 
     * @param rhs New attribute value.
     * @param precision Precision for the conversion.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(float rhs, int precision);

    /**
     * @brief Sets the attribute value with type conversion (boolean).
     * 
     * @param rhs New attribute value.
     * @return TRUE if successful, FALSE otherwise.
     */
    bool set_value(bool rhs);

public:
    /**
     * @brief Gets the next attribute in the attribute list of the parent node.
     * 
     * @return Next attribute.
     */
    SXmlAttr next_attribute() const;

    /**
     * @brief Gets the previous attribute in the attribute list of the parent node.
     * 
     * @return Previous attribute.
     */
    SXmlAttr previous_attribute() const;
};

/**
 * @class SXmlNode
 * @brief Class representing an XML node.
 * 
 * This class provides methods to manipulate and access XML nodes.
 */
class UTILITIES_API SXmlNode : public IXmlNode
{
    friend class SXmlDoc;

private:
    mutable pugi::xml_node _node;

private:
    /**
     * @brief Converts a pugi::xml_node to an IXmlNode pointer.
     * 
     * @param node The pugi::xml_node to convert.
     * @return Pointer to the converted IXmlNode.
     */
    static IXmlNode * toIXmlNode(pugi::xml_node node);

public:
    /**
     * @brief Constructor for SXmlNode.
     * 
     * @param node The pugi::xml_node to initialize with.
     */
    SXmlNode(pugi::xml_node node);

    /**
     * @brief Constructor for SXmlNode from an IXmlNode pointer.
     * 
     * @param src The IXmlNode pointer to initialize with.
     */
    SXmlNode(const IXmlNode * src);

    /**
     * @brief Copy constructor for SXmlNode.
     * 
     * @param src The SXmlNode to copy.
     */
    SXmlNode(const SXmlNode& src);

    /**
     * @brief Constructor for SXmlNode with an optional private data pointer.
     * 
     * @param pData Pointer to private data.
     */
    explicit SXmlNode(LPVOID pData = NULL);

public:
    /**
     * @brief Converts the node to a string representation.
     * 
     * @param out Pointer to the output string.
     */
    STDMETHOD_(void, ToString)(THIS_ IStringW *out) SCONST OVERRIDE;

    /**
     * @brief Gets the private data pointer.
     * 
     * @return Pointer to the private data.
     */
    STDMETHOD_(LPVOID, GetPrivPtr)(THIS) SCONST OVERRIDE;

    /**
     * @brief Checks if the node is empty.
     * 
     * @return TRUE if the node is empty, FALSE otherwise.
     */
    STDMETHOD_(BOOL, Empty)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the node name.
     * 
     * @return Node name, or "" if the node is empty.
     */
    STDMETHOD_(const wchar_t*, Name)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the node value.
     * 
     * @return Node value, or "" if the node is empty.
     */
    STDMETHOD_(const wchar_t*, Value)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the node text.
     * 
     * @return Node text, or "" if the node is empty.
     */
    STDMETHOD_(const wchar_t*, Text)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the node text.
     * 
     * @return true if successful, false otherwise.
     */
    STDMETHOD_(BOOL, SetText)(THIS_ const wchar_t* text) OVERRIDE;
    /**
     * @brief Sets user data for the node.
     * 
     * @param data User data to set.
     * @return TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, set_userdata)(THIS_ int data) OVERRIDE;

    /**
     * @brief Gets user data for the node.
     * 
     * @return User data.
     */
    STDMETHOD_(int, get_userdata)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets an attribute by name.
     * 
     * @param name Attribute name.
     * @param bCaseSensitive Flag indicating case sensitivity.
     * @return Pointer to the attribute.
     */
    STDMETHOD_(IXmlAttr*, Attribute)(THIS_ const wchar_t* name, BOOL bCaseSensitive) SCONST OVERRIDE;

    /**
     * @brief Gets the first attribute in the attribute list.
     * 
     * @return Pointer to the first attribute.
     */
    STDMETHOD_(IXmlAttr*, FirstAttribute)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the last attribute in the attribute list.
     * 
     * @return Pointer to the last attribute.
     */
    STDMETHOD_(IXmlAttr*, LastAttribute)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets a child node by name.
     * 
     * @param name Child node name.
     * @param bCaseSensitive Flag indicating case sensitivity.
     * @return Pointer to the child node.
     */
    STDMETHOD_(IXmlNode*, Child)(THIS_ const wchar_t* name, BOOL bCaseSensitive) SCONST OVERRIDE;

    /**
     * @brief Gets the first child node.
     * 
     * @return Pointer to the first child node.
     */
    STDMETHOD_(IXmlNode*, FirstChild)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the last child node.
     * 
     * @return Pointer to the last child node.
     */
    STDMETHOD_(IXmlNode*, LastChild)(THIS) SCONST OVERRIDE;

    /**
     * @brief Gets the next sibling node.
     * 
     * @return Pointer to the next sibling node.
     */
    STDMETHOD_(IXmlNode *, NextSibling)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Gets the previous sibling node.
     * 
     * @return Pointer to the previous sibling node.
     */
    STDMETHOD_(IXmlNode *, PrevSibling)(CTHIS) SCONST OVERRIDE;

    /**
     * @brief Gets the next sibling node by name.
     * 
     * @param name Sibling node name.
     * @param bCaseSensitive Flag indicating case sensitivity.
     * @return Pointer to the next sibling node.
     */
    STDMETHOD_(IXmlNode*, NextSibling2)(THIS_ const wchar_t* name, BOOL bCaseSensitive) SCONST OVERRIDE;

    /**
     * @brief Gets the previous sibling node by name.
     * 
     * @param name Sibling node name.
     * @param bCaseSensitive Flag indicating case sensitivity.
     * @return Pointer to the previous sibling node.
     */
    STDMETHOD_(IXmlNode*, PrevSibling2)(THIS_ const wchar_t* name, BOOL bCaseSensitive) SCONST OVERRIDE;

    /**
     * @brief Appends a child node with the specified name.
     * 
     * @param name Child node name.
     * @return Pointer to the appended child node.
     */
    STDMETHOD_(IXmlNode*, AppendChild)(THIS_ const wchar_t* name) OVERRIDE {
        return toIXmlNode(append_child(name)._node);
    }

    /**
     * @brief Prepends a child node with the specified name.
     * 
     * @param name Child node name.
     * @return Pointer to the prepended child node.
     */
    STDMETHOD_(IXmlNode*, PrependChild)(THIS_ const wchar_t* name) OVERRIDE {
        return toIXmlNode(prepend_child(name)._node);
    }

    /**
     * @brief Appends a copy of the specified node as a child.
     * 
     * @param proto Node to copy.
     * @return Pointer to the appended child node.
     */
    STDMETHOD_(IXmlNode*, AppendCopyNode)(THIS_ const IXmlNode* proto) OVERRIDE {
        return toIXmlNode(append_copy(SXmlNode(proto))._node);
    }

    /**
     * @brief Prepends a copy of the specified node as a child.
     * 
     * @param proto Node to copy.
     * @return Pointer to the prepended child node.
     */
    STDMETHOD_(IXmlNode*, PrependCopyNode)(THIS_ const IXmlNode* proto) OVERRIDE {
        return toIXmlNode(prepend_copy(SXmlNode(proto))._node);
    }

    /**
     * @brief Appends an attribute with the specified name.
     * 
     * @param name Attribute name.
     * @return Pointer to the appended attribute.
     */
    STDMETHOD_(IXmlAttr*, AppendAttribute)(THIS_ const wchar_t* name) OVERRIDE {
        return SXmlAttr::toIXmlAttr(append_attribute(name)._attr);
    }

    /**
     * @brief Prepends an attribute with the specified name.
     * 
     * @param name Attribute name.
     * @return Pointer to the prepended attribute.
     */
    STDMETHOD_(IXmlAttr*, PrependAttribute)(THIS_ const wchar_t* name) OVERRIDE {
        return SXmlAttr::toIXmlAttr(prepend_attribute(name)._attr);
    }

    /// @brief Appends a copy of the specified attribute to the node.
    /// @param proto The attribute to be copied.
    /// @return A pointer to the newly added attribute, or an empty attribute on errors.
	STDMETHOD_(IXmlAttr*, AppendCopyAttribute)(THIS_ const IXmlAttr* proto) OVERRIDE{
    SXmlAttr attr(proto);
    return SXmlAttr::toIXmlAttr(append_copy(attr)._attr);
}

    /// @brief Prepends a copy of the specified attribute to the node.
    /// @param proto The attribute to be copied.
    /// @return A pointer to the newly added attribute, or an empty attribute on errors.
	STDMETHOD_(IXmlAttr*, PrependCopyAttribute)(THIS_ const IXmlAttr* proto) OVERRIDE{
    SXmlAttr attr(proto);
    return SXmlAttr::toIXmlAttr(prepend_copy(attr)._attr);
}

STDMETHOD_(BOOL, RemoveAttribute)(THIS_ const wchar_t* name) OVERRIDE{
    /// @brief Removes the attribute with the specified name.
    /// @param name The name of the attribute to be removed.
    /// @return TRUE if the attribute was successfully removed, FALSE otherwise.
    return !!remove_attribute(name);
}

    /// @brief Removes the child node with the specified name.
    /// @param name The name of the child node to be removed.
    /// @return TRUE if the child node was successfully removed, FALSE otherwise.
	STDMETHOD_(BOOL, RemoveChild)(THIS_ const wchar_t* name) OVERRIDE{
    return !!remove_child(name);
}

    /// @brief Removes all child nodes from the node.
    /// @return TRUE if all child nodes were successfully removed, FALSE otherwise.
	STDMETHOD_(BOOL, RemoveAllChilden)(THIS) OVERRIDE{
    return !!remove_children();
}

public:
    /// @brief Checks if two SXmlNode objects are equal.
    /// @param src The SXmlNode object to compare with.
    /// @return TRUE if the nodes are equal, FALSE otherwise.
    bool operator ==(const SXmlNode &src) const{
        return _node==src._node;
    }

    /// @brief Checks if two SXmlNode objects are not equal.
    /// @param src The SXmlNode object to compare with.
    /// @return TRUE if the nodes are not equal, FALSE otherwise.
    bool operator !=(const SXmlNode &src) const{
        return _node!=src._node;
    }

    /// @brief Checks if the node is empty.
    /// @return TRUE if the node is empty, FALSE otherwise.
    operator bool() const;

    /// @brief Checks if the node is empty.
    /// @return TRUE if the node is empty, FALSE otherwise.
    bool empty() const;

    /// @brief Gets the type of the node.
    /// @return The type of the node.
    XmlNodeType type() const;

    /// @brief Gets the name of the node.
    /// @return The name of the node, or an empty string if the node is empty or has no name.
    const wchar_t* name() const;

    /// @brief Gets the value of the node.
    /// @return The value of the node, or an empty string if the node is empty or has no value.
    /// @note For <node>text</node>, node.value() does not return "text"! Use child_value() or text() methods to access text inside nodes.
    const wchar_t* value() const;

    /// @brief Gets the first attribute of the node.
    /// @return The first attribute of the node.
    SXmlAttr first_attribute() const;

    /// @brief Gets the last attribute of the node.
    /// @return The last attribute of the node.
    SXmlAttr last_attribute() const;

    /// @brief Gets the first child node of the node.
    /// @return The first child node of the node.
    SXmlNode first_child() const;

    /// @brief Gets the last child node of the node.
    /// @return The last child node of the node.
    SXmlNode last_child() const;

    /// @brief Gets the next sibling node in the children list of the parent node.
    /// @return The next sibling node.
    SXmlNode next_sibling() const;

    /// @brief Gets the previous sibling node in the children list of the parent node.
    /// @return The previous sibling node.
    SXmlNode previous_sibling() const;

    /// @brief Gets the parent node of the node.
    /// @return The parent node.
    SXmlNode parent() const;

    /// @brief Gets the root node of the DOM tree this node belongs to.
    /// @return The root node.
    SXmlNode root() const;

    /// @brief Gets the child node, attribute, or next/previous sibling with the specified name.
    /// @param name The name of the child node, attribute, or sibling.
    /// @param bCaseSensitive Whether the search should be case-sensitive.
    /// @param auto_create Whether to create the node if it does not exist.
    /// @return The requested node or attribute.
    SXmlNode child(const wchar_t* name,bool bCaseSensitive=false,bool auto_create=false) const;

    /// @brief Gets the attribute with the specified name.
    /// @param name The name of the attribute.
    /// @param bCaseSensitive Whether the search should be case-sensitive.
    /// @return The requested attribute.
    SXmlAttr attribute(const wchar_t* name,bool bCaseSensitive=false,bool auto_create=false) const;

    /// @brief Gets the next sibling node with the specified name.
    /// @param name The name of the sibling node.
    /// @param bCaseSensitive Whether the search should be case-sensitive.
    /// @return The next sibling node.
    SXmlNode next_sibling(const wchar_t* name,bool bCaseSensitive=false) const;

    /// @brief Gets the previous sibling node with the specified name.
    /// @param name The name of the sibling node.
    /// @param bCaseSensitive Whether the search should be case-sensitive.
    /// @return The previous sibling node.
    SXmlNode previous_sibling(const wchar_t* name,bool bCaseSensitive=false) const;

    /// @brief Gets the attribute with the specified name, starting the search from a hint.
    /// @param name The name of the attribute.
    /// @param hint The hint attribute to start the search from.
    /// @param bCaseSensitive Whether the search should be case-sensitive.
    /// @return The requested attribute.
    SXmlAttr attribute(const wchar_t* name, SXmlAttr& hint,bool bCaseSensitive=false) const;

    /// @brief Gets the child value of the current node.
    /// @return The value of the first child node of type PCDATA/CDATA.
    const wchar_t* child_value() const;

    /// @brief Gets the child value of the child node with the specified name.
    /// @param name The name of the child node.
    /// @param bCaseSensitive Whether the search should be case-sensitive.
    /// @return The value of the child node.
    const wchar_t* child_value(const wchar_t* name,bool bCaseSensitive=false) const;

    /// @brief Sets the name of the node.
    /// @param rhs The new name for the node.
    /// @return TRUE if the name was successfully set, FALSE otherwise.
    bool set_name(const wchar_t* rhs);

    /// @brief Sets the value of the node.
    /// @param rhs The new value for the node.
    /// @return TRUE if the value was successfully set, FALSE otherwise.
    bool set_value(const wchar_t* rhs);

    /// @brief Adds an attribute with the specified name.
    /// @param name The name of the attribute to be added.
    /// @return The added attribute, or an empty attribute on errors.
    SXmlAttr append_attribute(const wchar_t* name);

    /// @brief Prepends an attribute with the specified name.
    /// @param name The name of the attribute to be prepended.
    /// @return The prepended attribute, or an empty attribute on errors.
    SXmlAttr prepend_attribute(const wchar_t* name);

    /// @brief Inserts an attribute with the specified name after the specified attribute.
    /// @param name The name of the attribute to be inserted.
    /// @param attr The attribute after which the new attribute should be inserted.
    /// @return The inserted attribute, or an empty attribute on errors.
    SXmlAttr insert_attribute_after(const wchar_t* name, const SXmlAttr& attr);

    /// @brief Inserts an attribute with the specified name before the specified attribute.
    /// @param name The name of the attribute to be inserted.
    /// @param attr The attribute before which the new attribute should be inserted.
    /// @return The inserted attribute, or an empty attribute on errors.
    SXmlAttr insert_attribute_before(const wchar_t* name, const SXmlAttr& attr);

    /// @brief Adds a copy of the specified attribute.
    /// @param proto The attribute to be copied.
    /// @return The added attribute, or an empty attribute on errors.
    SXmlAttr append_copy(const SXmlAttr& proto);

    /// @brief Prepends a copy of the specified attribute.
    /// @param proto The attribute to be copied.
    /// @return The prepended attribute, or an empty attribute on errors.
    SXmlAttr prepend_copy(const SXmlAttr& proto);

    /// @brief Inserts a copy of the specified attribute after the specified attribute.
    /// @param proto The attribute to be copied.
    /// @param attr The attribute after which the new attribute should be inserted.
    /// @return The inserted attribute, or an empty attribute on errors.
    SXmlAttr insert_copy_after(const SXmlAttr& proto, const SXmlAttr& attr);

    /// @brief Inserts a copy of the specified attribute before the specified attribute.
    /// @param proto The attribute to be copied.
    /// @param attr The attribute before which the new attribute should be inserted.
    /// @return The inserted attribute, or an empty attribute on errors.
    SXmlAttr insert_copy_before(const SXmlAttr& proto, const SXmlAttr& attr);

    /// @brief Adds a child node with the specified type.
    /// @param type The type of the child node to be added.
    /// @return The added node, or an empty node on errors.
    SXmlNode append_child(XmlNodeType type = node_element);

    /// @brief Prepends a child node with the specified type.
    /// @param type The type of the child node to be prepended.
    /// @return The prepended node, or an empty node on errors.
    SXmlNode prepend_child(XmlNodeType type = node_element);

    /// @brief Inserts a child node with the specified type after the specified node.
    /// @param type The type of the child node to be inserted.
    /// @param node The node after which the new child node should be inserted.
    /// @return The inserted node, or an empty node on errors.
    SXmlNode insert_child_after(XmlNodeType type, const SXmlNode& node);

    /// @brief Inserts a child node with the specified type before the specified node.
    /// @param type The type of the child node to be inserted.
    /// @param node The node before which the new child node should be inserted.
    /// @return The inserted node, or an empty node on errors.
    SXmlNode insert_child_before(XmlNodeType type, const SXmlNode& node);

    /// @brief Adds a child element with the specified name.
    /// @param name The name of the child element to be added.
    /// @return The added node, or an empty node on errors.
    SXmlNode append_child(const wchar_t* name);

    /// @brief Prepends a child element with the specified name.
    /// @param name The name of the child element to be prepended.
    /// @return The prepended node, or an empty node on errors.
    SXmlNode prepend_child(const wchar_t* name);

    /// @brief Inserts a child element with the specified name after the specified node.
    /// @param name The name of the child element to be inserted.
    /// @param node The node after which the new child element should be inserted.
    /// @return The inserted node, or an empty node on errors.
    SXmlNode insert_child_after(const wchar_t* name, const SXmlNode& node);

    /// @brief Inserts a child element with the specified name before the specified node.
    /// @param name The name of the child element to be inserted.
    /// @param node The node before which the new child element should be inserted.
    /// @return The inserted node, or an empty node on errors.
    SXmlNode insert_child_before(const wchar_t* name, const SXmlNode& node);

    /// @brief Adds a copy of the specified node as a child.
    /// @param proto The node to be copied.
    /// @return The added node, or an empty node on errors.
    SXmlNode append_copy(const SXmlNode& proto);

    /// @brief Prepends a copy of the specified node as a child.
    /// @param proto The node to be copied.
    /// @return The prepended node, or an empty node on errors.
    SXmlNode prepend_copy(const SXmlNode& proto);

    /// @brief Inserts a copy of the specified node as a child after the specified node.
    /// @param proto The node to be copied.
    /// @param node The node after which the new child node should be inserted.
    /// @return The inserted node, or an empty node on errors.
    SXmlNode insert_copy_after(const SXmlNode& proto, const SXmlNode& node);

    /// @brief Inserts a copy of the specified node as a child before the specified node.
    /// @param proto The node to be copied.
    /// @param node The node before which the new child node should be inserted.
    /// @return The inserted node, or an empty node on errors.
    SXmlNode insert_copy_before(const SXmlNode& proto, const SXmlNode& node);

    /// @brief Moves the specified node to become a child of this node.
    /// @param moved The node to be moved.
    /// @return The moved node, or an empty node on errors.
    SXmlNode append_move(const SXmlNode& moved);

    /// @brief Prepends the specified node to become a child of this node.
    /// @param moved The node to be moved.
    /// @return The moved node, or an empty node on errors.
    SXmlNode prepend_move(const SXmlNode& moved);

    /// @brief Inserts the specified node to become a child of this node after the specified node.
    /// @param moved The node to be moved.
    /// @param node The node after which the new child node should be inserted.
    /// @return The moved node, or an empty node on errors.
    SXmlNode insert_move_after(const SXmlNode& moved, const SXmlNode& node);

    /// @brief Inserts the specified node to become a child of this node before the specified node.
    /// @param moved The node to be moved.
    /// @param node The node before which the new child node should be inserted.
    /// @return The moved node, or an empty node on errors.
    SXmlNode insert_move_before(const SXmlNode& moved, const SXmlNode& node);

    /// @brief Removes the specified attribute.
    /// @param a The attribute to be removed.
    /// @return TRUE if the attribute was successfully removed, FALSE otherwise.
    bool remove_attribute(const SXmlAttr& a);

    /// @brief Removes the attribute with the specified name.
    /// @param name The name of the attribute to be removed.
    /// @return TRUE if the attribute was successfully removed, FALSE otherwise.
    bool remove_attribute(const wchar_t* name);

    /// @brief Removes all attributes from the node.
    /// @return TRUE if all attributes were successfully removed, FALSE otherwise.
    bool remove_attributes();

    /// @brief Removes the specified child node.
    /// @param n The child node to be removed.
    /// @return TRUE if the child node was successfully removed, FALSE otherwise.
    bool remove_child(const SXmlNode& n);

    /// @brief Removes the child node with the specified name.
    /// @param name The name of the child node to be removed.
    /// @return TRUE if the child node was successfully removed, FALSE otherwise.
    bool remove_child(const wchar_t* name);

    /// @brief Removes all child nodes from the node.
    /// @return TRUE if all child nodes were successfully removed, FALSE otherwise.
    bool remove_children();

    SStringW toString() const;
};

/**
 * @brief Implementation of IXmlDoc.
 */
class UTILITIES_API SXmlDoc : public TObjRefImpl<IXmlDoc>
{
    friend class SXmlAttr;
    friend class SXmlNode;
private:
    /// @brief Pointer to the underlying pugi::xml_document.
    pugi::xml_document *_doc;

    /// @brief Result of the last parsing operation.
    mutable pugi::xml_parse_result _result;

    /// @brief Map type for storing attributes.
    typedef SMap<pugi::xml_attribute_struct *, SXmlAttr*> AttrMap;

    /// @brief Map for storing attributes.
    AttrMap *m_attrMap;

    /// @brief Map type for storing nodes.
    typedef SMap<pugi::xml_node_struct *, SXmlNode*> NodeMap;

    /// @brief Map for storing nodes.
    NodeMap *m_nodeMap;

    /// @brief Converts a pugi::xml_attribute_struct to an IXmlAttr.
    /// @param pAttr The attribute structure to convert.
    /// @return A pointer to the converted attribute.
    IXmlAttr * toIXmlAttr(pugi::xml_attribute_struct *pAttr);

    /// @brief Converts a pugi::xml_node_struct to an IXmlNode.
    /// @param pNode The node structure to convert.
    /// @return A pointer to the converted node.
    IXmlNode * toIXmlNode(pugi::xml_node_struct* pNode);

    /// @brief Clears the attribute and node maps.
    void clearMap();

public:
    /// @brief Constructor for SXmlDoc.
    SXmlDoc();

    /// @brief Destructor for SXmlDoc.
    ~SXmlDoc();

public:
    /// @brief Retrieves a private pointer associated with the document.
    /// @return The private pointer.
    STDMETHOD_(LPVOID, GetPrivPtr)(THIS) SCONST OVERRIDE;

    /// @brief Resets the document, removing all nodes.
    STDMETHOD_(void, Reset)(THIS) OVERRIDE;

    /// @brief Copies the contents of another document into this document.
    /// @param proto The document to copy from.
    STDMETHOD_(void, Copy)(THIS_ const IXmlDoc* proto) OVERRIDE;

    /// @brief Loads the document from a zero-terminated string.
    /// @param contents The string containing the XML data.
    /// @param options Parsing options.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    STDMETHOD_(BOOL, LoadString)(THIS_ const wchar_t* contents, unsigned int options) OVERRIDE;

    /// @brief Loads the document from a file (ANSI version).
    /// @param path The path to the file.
    /// @param options Parsing options.
    /// @param encoding The encoding of the file.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    STDMETHOD_(BOOL, LoadFileA)(THIS_ const char* path, unsigned int options, XmlEncoding encoding) OVERRIDE;

    /// @brief Loads the document from a file (Unicode version).
    /// @param path The path to the file.
    /// @param options Parsing options.
    /// @param encoding The encoding of the file.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    STDMETHOD_(BOOL, LoadFileW)(THIS_ const wchar_t* path, unsigned int options, XmlEncoding encoding) OVERRIDE;

    /// @brief Loads the document from a buffer.
    /// @param contents The buffer containing the XML data.
    /// @param size The size of the buffer.
    /// @param options Parsing options.
    /// @param encoding The encoding of the buffer.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    STDMETHOD_(BOOL, LoadBuffer)(THIS_ const void* contents, size_t size, unsigned int options, XmlEncoding encoding) OVERRIDE;

    /// @brief Loads the document from a buffer using in-place parsing.
    /// @param contents The buffer containing the XML data.
    /// @param size The size of the buffer.
    /// @param options Parsing options.
    /// @param encoding The encoding of the buffer.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    /// @note You should ensure that buffer data will persist throughout the document's lifetime and free the buffer memory manually once the document is destroyed.
    STDMETHOD_(BOOL, LoadBufferInplace)(THIS_ void* contents, size_t size, unsigned int options, XmlEncoding encoding) OVERRIDE;

    /// @brief Loads the document from a buffer using in-place parsing with automatic buffer management.
    /// @param contents The buffer containing the XML data.
    /// @param size The size of the buffer.
    /// @param options Parsing options.
    /// @param encoding The encoding of the buffer.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    /// @note You should allocate the buffer with pugixml allocation function; document will free the buffer when it is no longer needed.
    STDMETHOD_(BOOL, LoadBufferInplaceOwn)(THIS_ void* contents, size_t size, unsigned int options, XmlEncoding encoding) OVERRIDE;

    /// @brief Retrieves the result of the last parsing operation.
    /// @param pResult The parse result to store.
    STDMETHOD_(void, GetParseResult)(THIS_ XmlParseResult *pResult) SCONST OVERRIDE;

    /// @brief Saves the XML document to a writer.
    /// @param f The file pointer to write to.
    /// @note Semantics is slightly different from xml_node::print, see documentation for details.
    STDMETHOD_(void, SaveBinary)(THIS_ FILE *f) SCONST OVERRIDE;

    /// @brief Saves the XML document to a file (ANSI version).
    /// @param path The path to the file.
    /// @param indent The indentation string.
    /// @param flags Saving flags.
    /// @param encoding The encoding of the file.
    /// @return TRUE if the document was successfully saved, FALSE otherwise.
    STDMETHOD_(BOOL, SaveFileA)(THIS_ const char* path, const wchar_t* indent, unsigned int flags, XmlEncoding encoding) SCONST OVERRIDE;

    /// @brief Saves the XML document to a file (Unicode version).
    /// @param path The path to the file.
    /// @param indent The indentation string.
    /// @param flags Saving flags.
    /// @param encoding The encoding of the file.
    /// @return TRUE if the document was successfully saved, FALSE otherwise.
    STDMETHOD_(BOOL, SaveFileW)(THIS_ const wchar_t* path, const wchar_t* indent, unsigned int flags, XmlEncoding encoding) SCONST OVERRIDE;

    /// @brief Retrieves the root node of the document.
    /// @return The root node.
    STDMETHOD_(IXmlNode *, Root)(THIS) SCONST OVERRIDE;

public:
    /// @brief Loads the document from a zero-terminated string.
    /// @param contents The string containing the XML data.
    /// @param options Parsing options.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    bool load_string(const wchar_t* contents, unsigned int options = xml_parse_default);

    /// @brief Loads the document from a file (ANSI version).
    /// @param path The path to the file.
    /// @param options Parsing options.
    /// @param encoding The encoding of the file.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    bool load_file(const char* path, unsigned int options = xml_parse_default, XmlEncoding encoding = enc_auto);

    /// @brief Loads the document from a file (Unicode version).
    /// @param path The path to the file.
    /// @param options Parsing options.
    /// @param encoding The encoding of the file.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    bool load_file(const wchar_t* path, unsigned int options = xml_parse_default, XmlEncoding encoding = enc_auto);

    /// @brief Loads the document from a buffer.
    /// @param contents The buffer containing the XML data.
    /// @param size The size of the buffer.
    /// @param options Parsing options.
    /// @param encoding The encoding of the buffer.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    bool load_buffer(const void* contents, size_t size, unsigned int options = xml_parse_default, XmlEncoding encoding = enc_auto);

    /// @brief Loads the document from a buffer using in-place parsing.
    /// @param contents The buffer containing the XML data.
    /// @param size The size of the buffer.
    /// @param options Parsing options.
    /// @param encoding The encoding of the buffer.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    /// @note You should ensure that buffer data will persist throughout the document's lifetime and free the buffer memory manually once the document is destroyed.
    bool load_buffer_inplace(void* contents, size_t size, unsigned int options = xml_parse_default, XmlEncoding encoding = enc_auto);

    /// @brief Loads the document from a buffer using in-place parsing with automatic buffer management.
    /// @param contents The buffer containing the XML data.
    /// @param size The size of the buffer.
    /// @param options Parsing options.
    /// @param encoding The encoding of the buffer.
    /// @return TRUE if the document was successfully loaded, FALSE otherwise.
    /// @note You should allocate the buffer with pugixml allocation function; document will free the buffer when it is no longer needed.
    bool load_buffer_inplace_own(void* contents, size_t size, unsigned int options = xml_parse_default, XmlEncoding encoding = enc_auto);

    /// @brief Saves the XML document to a file (ANSI version).
    /// @param path The path to the file.
    /// @param indent The indentation string.
    /// @param flags Saving flags.
    /// @param encoding The encoding of the file.
    /// @return TRUE if the document was successfully saved, FALSE otherwise.
    bool save_file(const char* path, const wchar_t* indent = L"\t", unsigned int flags = xml_parse_default, XmlEncoding encoding = enc_auto) const;

    /// @brief Saves the XML document to a file (Unicode version).
    /// @param path The path to the file.
    /// @param indent The indentation string.
    /// @param flags Saving flags.
    /// @param encoding The encoding of the file.
    /// @return TRUE if the document was successfully saved, FALSE otherwise.
    bool save_file(const wchar_t* path, const wchar_t* indent = L"\t", unsigned int flags = xml_parse_default, XmlEncoding encoding = enc_auto) const;

    /// @brief Retrieves the root node of the document.
    /// @return The root node.
    SXmlNode root() const;

    /// @brief Retrieves the error description for a given XML status.
    /// @param status The XML status.
    /// @return The error description.
    static const char * GetErrDesc(XmlStatus status);
};


SNSEND

#endif // __SXML__H__