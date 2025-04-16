#ifndef _SEMPTYABLE_H_
#define _SEMPTYABLE_H_

#include <sdef.h>

SNSBEGIN

/**
 * @class SEmptyable
 * @brief A template class that can represent a value or be empty (NULL).
 * @details This class allows a variable to either hold a value of type `T` or be in an empty state.
 * @tparam T The type of the value to be stored.
 *
 * @example
 *      SEmptyable<int> a = 3;
 *      SEmptyable<int> b(4);
 *      a = 4;
 *      assert(!a.isEmpty());
 */
template <class T>
class SEmptyable
{
public:
    /**
     * @brief Constructor with initial value.
     * @param val The initial value to store.
     */
    SEmptyable(T val)
        : m_val(val), m_empty(false)
    {
    }

    /**
     * @brief Default constructor.
     * Initializes the object in an empty state.
     */
    SEmptyable()
        : m_empty(true)
    {
    }

    /**
     * @brief Checks if the object is empty.
     * @return TRUE if the object is empty, FALSE otherwise.
     */
    bool isEmpty() const
    {
        return m_empty;
    }

    /**
     * @brief Equality operator with another SEmptyable object.
     * @param right The SEmptyable object to compare with.
     * @return TRUE if both objects are equal, FALSE otherwise.
     */
    bool operator==(const SEmptyable &right) const
    {
        if (this->isEmpty() != right.isEmpty())
        {
            return false;
        }

        // both are empty
        if (this->isEmpty())
        {
            return true;
        }

        return this->m_val == right.m_val;
    }

    /**
     * @brief Equality operator with a value of type T.
     * @param right The value to compare with.
     * @return TRUE if the object holds the same value, FALSE otherwise.
     */
    bool operator==(const T &right) const
    {
        if (this->isEmpty())
        {
            return false;
        }

        return this->m_val == right;
    }

    /**
     * @brief Inequality operator with another SEmptyable object.
     * @param right The SEmptyable object to compare with.
     * @return TRUE if the objects are not equal, FALSE otherwise.
     */
    bool operator!=(const SEmptyable &right) const
    {
        return !(operator==(right));
    }

    /**
     * @brief Inequality operator with a value of type T.
     * @param right The value to compare with.
     * @return TRUE if the object does not hold the same value, FALSE otherwise.
     */
    bool operator!=(const T &right) const
    {
        return !(operator==(right));
    }

    /**
     * @brief Less-than operator with another SEmptyable object.
     * @param right The SEmptyable object to compare with.
     * @return TRUE if this object is less than the other object, FALSE otherwise.
     */
    bool operator<(const SEmptyable &right) const
    {
        if (this->isEmpty() != right.isEmpty())
        {
            return this->isEmpty();
        }

        if (this->isEmpty())
        {
            return false;
        }

        return this->m_val < right.m_val;
    }

    /**
     * @brief Less-than operator with a value of type T.
     * @param right The value to compare with.
     * @return TRUE if this object holds a value less than the given value, FALSE otherwise.
     */
    bool operator<(const T &right) const
    {
        if (this->isEmpty())
        {
            return true;
        }

        return this->m_val < right;
    }

    /**
     * @brief Assignment operator from another SEmptyable object.
     * @param right The SEmptyable object to assign from.
     * @return Reference to the current object.
     */
    SEmptyable &operator=(const SEmptyable &right)
    {
        m_val = right.m_val;
        m_empty = right.m_empty;
        return *this;
    }

    /**
     * @brief Assignment operator from a value of type T.
     * @param val The value to assign.
     * @return Reference to the current object.
     */
    SEmptyable &operator=(const T &val)
    {
        m_val = val;
        m_empty = false;
        return *this;
    }

    /**
     * @brief Conversion operator to type T.
     * @return The stored value.
     * @note Asserts that the object is not empty.
     */
    operator T() const
    {
        assert(m_empty == false);
        return m_val;
    }

    /**
     * @brief Resets the object to an empty state.
     */
    void reset()
    {
        m_empty = true;
    }

private:
    T m_val;       /**< The stored value. */
    bool m_empty;  /**< Flag indicating if the object is empty. */
};

SNSEND

#endif // _SEMPTYABLE_H_