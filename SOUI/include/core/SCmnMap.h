#ifndef __SCMNMAP__H__
#define __SCMNMAP__H__

#include <souicoll.h>

SNSBEGIN

/**
 * @class      SCmnMap
 * @brief      Template class for managing a map of objects with keys
 *
 * @details    This template class provides a map-like structure to store and manage objects
 *             associated with keys. It includes methods for adding, retrieving, and removing
 *             objects, as well as handling key removal callbacks.
 *
 * @tparam     TObj  Type of the objects stored in the map
 * @tparam     TKey  Type of the keys used to identify objects (default is SStringA)
 */
template <class TObj, class TKey = SStringA>
class SCmnMap {
  public:
    /**
     * @brief    Constructor
     * @param    funOnKeyRemoved  Callback function to be called when a key is removed
     *
     * @details  Initializes the map and sets the callback function for key removal.
     */
    SCmnMap(void (*funOnKeyRemoved)(const TObj &) = NULL)
        : m_pFunOnKeyRemoved(funOnKeyRemoved)
    {
        m_mapNamedObj = new SMap<TKey, TObj>;
    }

    /**
     * @brief    Destructor
     *
     * @details  Cleans up the map and deletes the internal map object.
     */
    virtual ~SCmnMap()
    {
        RemoveAll();
        delete m_mapNamedObj;
    }

    /**
     * @brief    Checks if a key exists in the map
     * @param    key  Key to check
     * @return   TRUE if the key exists, otherwise FALSE
     *
     * @details  Returns whether the specified key is present in the map.
     */
    bool HasKey(const TKey &key) const
    {
        return m_mapNamedObj->Lookup(key) != NULL;
    }

    /**
     * @brief    Retrieves an object associated with a key
     * @param    key  Key to look up
     * @param    obj  Object to store the retrieved value
     * @return   TRUE if the key exists and the object is retrieved, otherwise FALSE
     *
     * @details  Retrieves the object associated with the specified key and stores it in the provided object.
     */
    bool GetKeyObject(const TKey &key, TObj &obj) const
    {
        if (!HasKey(key))
            return false;
        obj = (*m_mapNamedObj)[key];
        return true;
    }

    /**
     * @brief    Retrieves an object associated with a key
     * @param    key  Key to look up
     * @return   Reference to the object associated with the key
     *
     * @details  Returns a reference to the object associated with the specified key.
     *           Note: This method does not check if the key exists.
     */
    TObj &GetKeyObject(const TKey &key) const
    {
        return (*m_mapNamedObj)[key];
    }

    /**
     * @brief    Adds a key-object pair to the map
     * @param    key  Key to add
     * @param    obj  Object to associate with the key
     * @return   TRUE if the key was added successfully, otherwise FALSE
     *
     * @details  Adds a new key-object pair to the map. Returns FALSE if the key already exists.
     */
    bool AddKeyObject(const TKey &key, const TObj &obj)
    {
        if (HasKey(key))
            return false;
        (*m_mapNamedObj)[key] = obj;
        return true;
    }

    /**
     * @brief    Sets an object for a key, replacing any existing object
     * @param    key  Key to set
     * @param    obj  Object to associate with the key
     *
     * @details  Sets the object for the specified key, removing any existing object associated with the key.
     */
    void SetKeyObject(const TKey &key, const TObj &obj)
    {
        RemoveKeyObject(key);
        (*m_mapNamedObj)[key] = obj;
    }

    /**
     * @brief    Removes a key-object pair from the map
     * @param    key  Key to remove
     * @return   TRUE if the key was removed successfully, otherwise FALSE
     *
     * @details  Removes the key-object pair associated with the specified key.
     *           Calls the callback function if it is set.
     */
    bool RemoveKeyObject(const TKey &key)
    {
        if (!HasKey(key))
            return false;
        if (m_pFunOnKeyRemoved)
        {
            m_pFunOnKeyRemoved((*m_mapNamedObj)[key]);
        }
        m_mapNamedObj->RemoveKey(key);
        return true;
    }

    /**
     * @brief    Removes all key-object pairs from the map
     *
     * @details  Removes all key-object pairs from the map and calls the callback function
     *           for each removed object if it is set.
     */
    void RemoveAll()
    {
        if (m_pFunOnKeyRemoved)
        {
            SPOSITION pos = m_mapNamedObj->GetStartPosition();
            while (pos)
            {
                typename SMap<TKey, TObj>::CPair *p = m_mapNamedObj->GetNext(pos);
                m_pFunOnKeyRemoved(p->m_value);
            }
        }
        m_mapNamedObj->RemoveAll();
    }

    /**
     * @brief    Gets the number of key-object pairs in the map
     * @return   Number of key-object pairs
     *
     * @details  Returns the total number of key-object pairs in the map.
     */
    size_t GetCount()
    {
        return m_mapNamedObj->GetCount();
    }

  protected:
    void (*m_pFunOnKeyRemoved)(const TObj &obj); /**< Callback function for key removal. */
    SMap<TKey, TObj> *m_mapNamedObj;             /**< Internal map object. */
};

SNSEND

#endif // __SCMNMAP__H__