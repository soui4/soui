/**
 * @file STree.h
 * @brief Tree template for general data types
 * @version v1.0
 * @author SOUI team
 * @date 2014-07-13
 *
 * @details This class provides a tree structure for general data types.
 *          Version history:
 *          - 1.0 2003-10-24: Initial implementation
 *          - 2.0 2004-12-29: Added two traversal interfaces, fixed memory release issues
 *          - 2.1 2006-10-17: Added hChildLast to speed up data insertion
 *          - 2.2 2008-10-16: Fixed an issue in a traversal interface
 *          - 2.3 2011-10-17: Changed data release interface from callback to virtual function
 */

#ifndef __STREE__H__
#define __STREE__H__

#ifndef SASSERT
#define SASSERT(x)
#endif

// typedef ULONG_PTR HSTREEITEM;
#ifdef _WIN64
#define STVN_ROOT  ((HSTREENODE)(ULONG_PTR)0xFFFF000000000000) /**< STVN_ROOT */
#define STVN_FIRST ((HSTREENODE)(ULONG_PTR)0xFFFF000000000001) /**< STVN_FIRST */
#define STVN_LAST  ((HSTREENODE)(ULONG_PTR)0xFFFF000000000002) /**< STVN_LAST */
#define STVL_ROOT  ((PSTREELINK)(ULONG_PTR)0xFFFF000000000000)
#else
#define STVN_ROOT  ((HSTREENODE)(ULONG_PTR)0xFFFF0000) /**< STVN_ROOT */
#define STVN_FIRST ((HSTREENODE)(ULONG_PTR)0xFFFF0001) /**< STVN_FIRST */
#define STVN_LAST  ((HSTREENODE)(ULONG_PTR)0xFFFF0002) /**< STVN_LAST */
#define STVL_ROOT  ((PSTREELINK)(ULONG_PTR)0xFFFF0000)
#endif

#ifndef STVI_ROOT
#ifdef _WIN64
#define STVI_ROOT  ((HSTREEITEM)0xFFFF000000000000) /**< STVI_ROOT */
#define STVI_FIRST ((HSTREEITEM)0xFFFF000000000001) /**< STVI_FIRST */
#define STVI_LAST  ((HSTREEITEM)0xFFFF000000000002) /**< STVI_LAST */
#else
#define STVI_ROOT  ((HSTREEITEM)0xFFFF0000) /**< STVI_ROOT */
#define STVI_FIRST ((HSTREEITEM)0xFFFF0001) /**< STVI_FIRST */
#define STVI_LAST  ((HSTREEITEM)0xFFFF0002) /**< STVI_LAST */
#endif
#endif // STVI_ROOT

/**
 * @class CSTree
 * @brief Template class for a tree structure
 *
 * @details This template class provides a tree structure for general data types.
 */
template <class T>
class CSTree {
    /**
     * @struct _STREENODE
     * @brief Node structure
     *
     * @details Structure representing a node in the tree.
     */
    typedef struct _STREENODE
    {
        struct _STREENODE *hParent;      /**< Parent node */
        struct _STREENODE *hChildFirst;  /**< First child node */
        struct _STREENODE *hChildLast;   /**< Last child node */
        struct _STREENODE *hPrevSibling; /**< Previous sibling node */
        struct _STREENODE *hNextSibling; /**< Next sibling node */
        T data;                          /**< Data stored in the node */
    } STREENODE, *HSTREENODE;

    /**
     * @struct _STREELINK
     * @brief Tree node link structure
     *
     * @details Structure used to link tree nodes.
     */
    typedef struct _STREELINK
    {
        HSTREENODE hParent;      /**< Parent node */
        HSTREENODE hChildFirst;  /**< First child node */
        HSTREENODE hChildLast;   /**< Last child node */
        HSTREENODE hPrevSibling; /**< Previous sibling node */
        HSTREENODE hNextSibling; /**< Next sibling node */
    } STREELINK, *PSTREELINK;

    /**
     * @typedef CBTRAVERSING
     * @brief Callback function for tree traversal
     *
     * @details Prototype for the callback function used in tree traversal.
     * @param T *: Current node data
     * @param LPARAM: Additional parameter for the callback
     * @return FALSE to continue traversal, TRUE to interrupt traversal
     */
    typedef BOOL (*CBTRAVERSING)(T *, LPARAM);

  public:
    struct IDataFreer
    {
        virtual void OnDataFree(T &data) = 0;
    };

  public:
    /**
     * @brief Constructor
     */
    CSTree()
        : m_hRootFirst(NULL)
        , m_hRootLast(NULL)
        , m_dataFreer(NULL)
    {
    }

    /**
     * @brief Destructor
     */
    virtual ~CSTree()
    {
        DeleteAllItems();
    }

    /**
     * @brief Delete all items in the tree
     */
    void DeleteAllItems()
    {
        if (m_hRootFirst)
        {
            FreeNode(STVN_ROOT);
            m_hRootFirst = NULL;
            m_hRootLast = NULL;
        }
    }

    /**
     * @brief Get the next sibling item
     * @param HSTREEITEM hItem: Node handle
     * @return HSTREEITEM: Next sibling node handle
     */
    static HSTREEITEM GetNextSiblingItem(HSTREEITEM hItem)
    {
        PSTREELINK pLink = (PSTREELINK)hItem;
        SASSERT(pLink && pLink != STVL_ROOT);
        return (HSTREEITEM)pLink->hNextSibling;
    }

    /**
     * @brief Get the previous sibling item
     * @param HSTREEITEM hItem: Node handle
     * @return HSTREEITEM: Previous sibling node handle
     */
    static HSTREEITEM GetPrevSiblingItem(HSTREEITEM hItem)
    {
        PSTREELINK pLink = (PSTREELINK)hItem;
        SASSERT(pLink && pLink != STVL_ROOT);
        return (HSTREEITEM)pLink->hPrevSibling;
    }

    /**
     * @brief Get the parent item
     * @param HSTREEITEM hItem: Node handle
     * @return HSTREEITEM: Parent node handle
     */
    static HSTREEITEM GetParentItem(HSTREEITEM hItem)
    {
        PSTREELINK pLink = (PSTREELINK)hItem;
        SASSERT(pLink && pLink != STVL_ROOT);
        return (HSTREEITEM)pLink->hParent;
    }

    /**
     * @brief Get the level of the item
     * @param HSTREEITEM hItem: Node handle
     * @return int: Level of the node
     */
    static int GetItemLevel(HSTREEITEM hItem)
    {
        int nRet = -1;
        if (hItem == STVI_ROOT)
            hItem = 0;
        while (hItem)
        {
            nRet++;
            hItem = GetParentItem(hItem);
        }
        return nRet;
    }

    /**
     * @brief Get the root item of the specified node
     * @param HSTREEITEM hItem: Current node handle
     * @return HSTREEITEM: Root node handle
     */
    static HSTREEITEM GetRootItem(HSTREEITEM hItem)
    {
        HSTREEITEM hParent = hItem;
        while (GetParentItem(hParent))
        {
            hParent = GetParentItem(hParent);
        }
        return hParent;
    }

    /**
     * @brief Get the child item
     * @param HSTREEITEM hItem: Node handle
     * @param BOOL bFirst: Whether to get the first child
     * @return HSTREEITEM: Child node handle
     */
    HSTREEITEM GetChildItem(HSTREEITEM hItem, BOOL bFirst = TRUE) const
    {
        HSTREENODE hsNode = (HSTREENODE)hItem;
        SASSERT(hsNode);
        if (hsNode == STVN_ROOT)
        {
            if (bFirst)
                return (HSTREEITEM)m_hRootFirst;
            else
                return (HSTREEITEM)m_hRootLast;
        }
        else
        {
            if (bFirst)
                return (HSTREEITEM)hsNode->hChildFirst;
            else
                return (HSTREEITEM)hsNode->hChildLast;
        }
    }

    /**
     * @brief Get the number of children
     * @param HSTREEITEM hItem: Node handle
     * @return int: Number of child nodes
     */
    int GetChildrenCount(HSTREEITEM hItem) const
    {
        int nRet = 0;
        HSTREEITEM hChild = GetChildItem(hItem);
        while (hChild)
        {
            nRet++;
            hChild = GetNextSiblingItem(hChild);
        }
        return nRet;
    }

    /**
     * @brief Delete an item
     * @param HSTREEITEM hItem: Node handle
     */
    virtual void DeleteItem(HSTREEITEM hItem)
    {
        HSTREENODE hsNode = (HSTREENODE)hItem;
        SASSERT(hsNode);
        if (hsNode == STVN_ROOT)
        {
            FreeNode(STVN_ROOT);
            m_hRootFirst = NULL;
            m_hRootLast = NULL;
            return;
        }
        STREENODE nodeCopy = *hsNode;
        BOOL bRootFirst = hsNode == m_hRootFirst;
        BOOL bRootLast = hsNode == m_hRootLast;
        FreeNode(hsNode);

        if (nodeCopy.hPrevSibling) // has prevsibling
            nodeCopy.hPrevSibling->hNextSibling = nodeCopy.hNextSibling;
        else if (nodeCopy.hParent) // parent's first child
            nodeCopy.hParent->hChildFirst = nodeCopy.hNextSibling;
        if (nodeCopy.hNextSibling) // update next sibling's previous sibling
            nodeCopy.hNextSibling->hPrevSibling = nodeCopy.hPrevSibling;
        else if (nodeCopy.hParent) // parent's last child
            nodeCopy.hParent->hChildLast = nodeCopy.hPrevSibling;
        // update root item
        if (bRootFirst)
            m_hRootFirst = nodeCopy.hNextSibling;
        if (bRootLast)
            m_hRootLast = nodeCopy.hPrevSibling;
    }

    /**
     * @brief Delete an item and its branch
     * @param HSTREEITEM hItem: Node handle
     * @return BOOL: TRUE if successful, FALSE otherwise
     */
    BOOL DeleteItemEx(HSTREEITEM hItem)
    {
        if (GetChildItem(hItem))
            return FALSE;
        while (hItem && !GetChildItem(hItem))
        {
            HSTREEITEM hParent = GetParentItem(hItem);
            DeleteItem(hItem);
            hItem = hParent;
        }
        return TRUE;
    }

    /**
     * @brief Get the item data
     * @param HSTREEITEM hItem: Node handle
     * @return T: Data stored in the node
     */
    static T GetItem(HSTREEITEM hItem)
    {
        SASSERT(hItem != STVI_ROOT);
        HSTREENODE hsNode = (HSTREENODE)hItem;
        SASSERT(hsNode);
        return hsNode->data;
    }

    /**
     * @brief Get the item data reference
     * @param HSTREEITEM hItem: Node handle
     * @return T&: Reference to data stored in the node
     */
    static T &GetItemRef(HSTREEITEM hItem)
    {
        SASSERT(hItem != STVI_ROOT);
        HSTREENODE hsNode = (HSTREENODE)hItem;
        SASSERT(hsNode);
        return hsNode->data;
    }

    /**
     * @brief Get the item data pointer
     * @param HSTREEITEM hItem: Node handle
     * @return T*: Pointer to data stored in the node
     */
    static T *GetItemPt(HSTREEITEM hItem)
    {
        SASSERT(hItem != STVI_ROOT);
        HSTREENODE hsNode = (HSTREENODE)hItem;
        SASSERT(hsNode);
        return &hsNode->data;
    }

    /**
     * @brief Insert a new item
     * @param const T &data: Data to insert
     * @param HSTREEITEM hParent: Parent node handle
     * @param HSTREEITEM hInsertAfter: Insert after this node
     * @return HSTREEITEM: Handle of the inserted node
     */
    HSTREEITEM InsertItem(const T &data, HSTREEITEM hParent = STVI_ROOT, HSTREEITEM hInsertAfter = STVI_LAST)
    {
        HSTREENODE hParentNode = (HSTREENODE)hParent;
        HSTREENODE hInsertAfterNode = (HSTREENODE)hInsertAfter;
        if (hParentNode == STVN_ROOT)
            hParentNode = NULL;
        SASSERT(hInsertAfter);
        if (hInsertAfterNode != STVN_FIRST && hInsertAfterNode != STVN_LAST)
        {
            if (hInsertAfterNode->hParent != hParentNode)
                return 0;
            if (hInsertAfterNode->hNextSibling == NULL)
                hInsertAfterNode = STVN_LAST;
        }

        HSTREENODE hInserted = new STREENODE;
        hInserted->data = data;
        hInserted->hParent = hParentNode;
        hInserted->hChildFirst = NULL;
        hInserted->hChildLast = NULL;

        if (hInsertAfterNode == STVN_FIRST)
        {
            hInserted->hPrevSibling = NULL;
            if (hParentNode == NULL) // root
            {
                hInserted->hNextSibling = m_hRootFirst;
                if (m_hRootFirst)
                    m_hRootFirst->hPrevSibling = hInserted;
                m_hRootFirst = hInserted;
                if (m_hRootLast == NULL)
                    m_hRootLast = hInserted;
            }
            else // has parent
            {
                hInserted->hNextSibling = hParentNode->hChildFirst;
                if (hInserted->hNextSibling)
                {
                    hInserted->hNextSibling->hPrevSibling = hInserted;
                    hParentNode->hChildFirst = hInserted;
                }
                else
                {
                    hParentNode->hChildLast = hParentNode->hChildFirst = hInserted;
                }
            }
        }
        else if (hInsertAfterNode == STVN_LAST)
        {
            hInserted->hNextSibling = NULL;
            if (hParentNode == NULL) // root
            {
                hInserted->hPrevSibling = m_hRootLast;
                if (m_hRootLast)
                    m_hRootLast->hNextSibling = hInserted;
                m_hRootLast = hInserted;
                if (!m_hRootFirst)
                    m_hRootFirst = hInserted;
            }
            else
            {
                hInserted->hPrevSibling = hParentNode->hChildLast;
                if (hParentNode->hChildLast)
                {
                    hInserted->hPrevSibling->hNextSibling = hInserted;
                    hParentNode->hChildLast = hInserted;
                }
                else
                {
                    hParentNode->hChildLast = hParentNode->hChildFirst = hInserted;
                }
            }
        }
        else
        {
            HSTREENODE hNextSibling = hInsertAfterNode->hNextSibling;
            hInserted->hPrevSibling = hInsertAfterNode;
            hInserted->hNextSibling = hNextSibling;
            hNextSibling->hPrevSibling = hInsertAfterNode->hNextSibling = hInserted;
        }
        return (HSTREEITEM)hInserted;
    }

    /**
     * @brief Traverse the tree recursively
     * @param HSTREEITEM hItem: Starting node handle
     * @param CBTRAVERSING funTraversing: Callback function
     * @param LPARAM lParam: Additional parameter for the callback
     * @return HSTREEITEM: Handle of the node where traversal stopped
     */
    HSTREEITEM TraversingRecursion(HSTREEITEM hItem, CBTRAVERSING funTraversing, LPARAM lParam)
    {
        SASSERT(hItem);
        if (hItem != STVI_ROOT)
        {
            if (funTraversing(GetItemPt(hItem), lParam))
                return hItem;
        }
        HSTREEITEM hChild = GetChildItem(hItem);
        while (hChild)
        {
            HSTREEITEM hTmp = GetChildItem(hChild);
            if (hTmp)
            {
                HSTREEITEM hRet = TraversingRecursion(hTmp, funTraversing, lParam);
                if (hRet)
                    return hRet;
            }
            else
            {
                if (funTraversing(GetItemPt(hChild), lParam))
                    return hChild;
            }
            hChild = GetNextSiblingItem(hChild);
        }
        return NULL;
    }

    /**
     * @brief Traverse the tree in sequence
     * @param HSTREEITEM hItem: Starting node handle
     * @param CBTRAVERSING funTraversing: Callback function
     * @param LPARAM lParam: Additional parameter for the callback
     * @return HSTREEITEM: Handle of the node where traversal stopped
     *
     * @details Traverses the tree in sequence starting from the specified node.
     *          If the callback function returns TRUE, the traversal stops and the current node is returned.
     *          Otherwise, it continues to the next node.
     */
    HSTREEITEM TraversingSequence(HSTREEITEM hItem, CBTRAVERSING funTraversing, LPARAM lParam)
    {
        if (!m_hRootFirst)
            return NULL;
        if (hItem != STVI_ROOT)
        {
            if (funTraversing(GetItemPt(hItem), lParam))
                return hItem;
        }
        HSTREEITEM hNext = GetNextItem(hItem);
        while (hNext)
        {
            if (funTraversing(GetItemPt(hNext), lParam))
                return hNext;
            hNext = GetNextItem(hNext);
        }
        return NULL;
    }

    /**
     * @brief Get the root item
     * @param BOOL bFirst: Whether to get the first root item (default is TRUE)
     * @return HSTREEITEM: Handle of the root item
     *
     * @details Returns the root item of the tree. If `bFirst` is TRUE, it returns the first root item;
     *          otherwise, it returns the last root item.
     */
    HSTREEITEM GetRootItem(BOOL bFirst = TRUE)
    {
        return (HSTREEITEM)(bFirst ? m_hRootFirst : m_hRootLast);
    }

    /**
     * @brief Get the number of descendants of a node
     * @param HSTREEITEM hItem: Starting node handle
     * @return int: Number of descendant nodes
     *
     * @details Recursively counts the number of descendant nodes starting from the specified node.
     */
    int GetDesendants(HSTREEITEM hItem)
    {
        int nRet = 0;
        HSTREEITEM hChild = GetChildItem(hItem);
        while (hChild)
        {
            nRet += 1 + GetDesendants(hChild);
            hChild = GetNextSiblingItem(hChild);
        }
        return nRet;
    }

    /**
     * @brief Get the next item in the tree
     * @param HSTREEITEM hItem: Current node handle
     * @return HSTREEITEM: Handle of the next node
     *
     * @details Returns the next node in the tree. The traversal order is:
     *          - First child of the current node
     *          - Next sibling of the current node
     *          - Next sibling of the parent node, if no children or siblings are available
     */
    HSTREEITEM GetNextItem(HSTREEITEM hItem) const
    {
        if (hItem == STVI_ROOT)
            return (HSTREEITEM)m_hRootFirst;

        HSTREEITEM hRet = GetChildItem(hItem);
        if (hRet)
            return hRet;
        HSTREEITEM hParent = hItem;
        while (hParent)
        {
            hRet = GetNextSiblingItem(hParent);
            if (hRet)
                return hRet;
            hParent = GetParentItem(hParent);
        }
        return 0;
    }

    /**
     * @brief Get the next item in the tree with level relationship
     * @param HSTREEITEM hItem: Current node handle
     * @param int &nLevel: Level relationship between the current node and the returned node
     * @return HSTREEITEM: Handle of the next node
     *
     * @details Returns the next node in the tree and sets the level relationship:
     *          - 1: Parent-child relationship
     *          - 0: Sibling relationship
     *          - -n: Child to parent's sibling relationship
     */
    HSTREEITEM GetNextItem(HSTREEITEM hItem, int &nLevel) const
    {
        if (hItem == STVI_ROOT)
        {
            nLevel = 1;
            return (HSTREEITEM)m_hRootFirst;
        }

        HSTREEITEM hRet = GetChildItem(hItem);
        if (hRet)
        {
            nLevel = 1;
            return hRet;
        }
        HSTREEITEM hParent = hItem;
        nLevel = 0;
        while (hParent)
        {
            hRet = GetNextSiblingItem(hParent);
            if (hRet)
                return hRet;
            nLevel--;
            hParent = GetParentItem(hParent);
        }
        return NULL;
    }

    /**
     * @brief Sort the children of a node
     * @param HSTREEITEM hItem: Node handle
     * @param int(__cdecl *funSort)(void *, const void *, const void *): Comparison function
     * @param void *pCtx: Context for the comparison function
     *
     * @details Sorts the children of the specified node using the provided comparison function.
     *          The comparison function should have the signature `int(__cdecl *)(void *, const void *, const void *)`.
     */
    void SortChildren(HSTREEITEM hItem, int(__cdecl *funSort)(void *, const void *, const void *), void *pCtx)
    {
        int nChilds = GetChildrenCount(hItem);
        if (nChilds > 1)
        {
            HSTREEITEM *pChilds = new HSTREEITEM[nChilds];
            HSTREEITEM hChild = GetChildItem(hItem);
            pChilds[0] = hChild;
            for (int i = 1; i < nChilds; i++)
            {
                hChild = GetNextSiblingItem(hChild);
                pChilds[i] = hChild;
            }
            // Call qsort to sort the children
            qsort_s(pChilds, nChilds, sizeof(HSTREEITEM), funSort, pCtx);
            // Reorganize the linked list
            for (int i = 0; i < nChilds - 1; i++)
            {
                HSTREENODE node = (HSTREENODE)pChilds[i];
                node->hNextSibling = (HSTREENODE)pChilds[i + 1];
            }
            for (int i = 1; i < nChilds; i++)
            {
                HSTREENODE node = (HSTREENODE)pChilds[i];
                node->hPrevSibling = (HSTREENODE)pChilds[i - 1];
            }
            HSTREENODE node = (HSTREENODE)pChilds[0];
            node->hPrevSibling = NULL;
            node = (HSTREENODE)pChilds[nChilds - 1];
            node->hNextSibling = NULL;
            if (hItem != STVI_ROOT)
            {
                HSTREENODE parent = (HSTREENODE)hItem;
                parent->hChildFirst = (HSTREENODE)pChilds[0];
                parent->hChildLast = (HSTREENODE)pChilds[nChilds - 1];
            }
            else
            {
                m_hRootFirst = (HSTREENODE)pChilds[0];
                m_hRootLast = (HSTREENODE)pChilds[nChilds - 1];
            }
            delete[] pChilds;
        }
        // Sort children recursively
        HSTREEITEM hChild = GetChildItem(hItem);
        while (hChild)
        {
            HSTREENODE node = (HSTREENODE)hChild;
            if (node->hChildFirst && node->hChildLast && node->hChildFirst != node->hChildLast)
            {
                SortChildren(hChild, funSort, pCtx);
            }
            hChild = GetNextSiblingItem(hChild);
        }
    }

    /**
     * @brief Set the data freer callback
     * @param IDataFreer *cbFree: Data freer callback object
     *
     * @details Sets the callback object for freeing node data.
     */
    void SetDataFreer(IDataFreer *cbFree)
    {
        m_dataFreer = cbFree;
    }

  private:
    /**
     * @fn void FreeNode(HSTREENODE hsNode)
     * @brief Recursively frees all child nodes of the given node using post-order traversal.
     *
     * @param HSTREENODE hsNode: The current node to free.
     *
     * @details This function uses post-order traversal to ensure that all child nodes are freed before the parent node.
     *          It iterates through all child nodes, recursively calling itself on each child. After freeing all children,
     *          it calls `OnNodeFree` to release any associated data and then deletes the node itself.
     */
    void FreeNode(HSTREENODE hsNode)
    {
        SASSERT(hsNode);
        HSTREENODE hSibling = (HSTREENODE)GetChildItem((HSTREEITEM)hsNode);
        while (hSibling)
        {
            HSTREENODE hNextSibling = hSibling->hNextSibling;
            FreeNode(hSibling);
            hSibling = hNextSibling;
        }
        if (hsNode != STVN_ROOT)
        {
            OnNodeFree(hsNode->data);
            delete hsNode;
        }
    }

  protected:
    /**
     * @fn virtual void OnNodeFree(T &data)
     * @brief Virtual function to handle the freeing of node data.
     *
     * @param T &data: Reference to the data stored in the node.
     *
     * @details This function is intended to be overridden in derived classes to implement specific data cleanup logic.
     *          If a data freer callback (`m_dataFreer`) is set, it will call the callback's `OnDataFree` method to handle
     *          the data cleanup.
     */
    virtual void OnNodeFree(T &data)
    {
        if (m_dataFreer)
            m_dataFreer->OnDataFree(data);
    }
    IDataFreer *m_dataFreer; /**< Pointer to an object implementing the `IDataFreer` interface for custom data cleanup. */
    HSTREENODE m_hRootFirst; /**< Pointer to the first root node of the tree. */
    HSTREENODE m_hRootLast;  /**< Pointer to the last root node of the tree. */
};

#endif // __STREE__H__
