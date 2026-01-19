#pragma once
#include <string>
#include <vector>
#include <assert.h>

namespace DUGON
{

/**
 * Inheriate this class to make your class cannot be copy and assign.
 *
 */
class NoCopyable
{
public:
    NoCopyable() {}
    ~NoCopyable() {}
private:
    NoCopyable(const NoCopyable & );
    const NoCopyable &operator=(const NoCopyable &);
};


/**
 * This is an type which can represent an value or NULL.
 * For example "Emptyable<int> func();" means this function can return an int or nothing.
 *
 * Code example:
 *      Emptyable<int> a = 3;
 *      Emptyable<int> b(4);
 *      a = 4;
 *      assert(!a.isEmpty());
 */
template <class T>
class Emptyable
{
public:
    Emptyable(T val)
        : m_val(val), m_empty(false)
    {

    }

    Emptyable()
        : m_empty(true)
    {
    }

    bool isEmpty() const
    {
        return m_empty;
    }

    bool operator==(const Emptyable &right) const
    {
        if (this->isEmpty() != right.isEmpty())
        {
            return false;
        }

        // both is empty.
        if (this->isEmpty())
        {
            return true;
        }

        return this->m_val == right.m_val;
    }

    bool operator==(const T &right) const
    {
        if (this->isEmpty())
        {
            return false;
        }

        return this->m_val == right;
    }


    bool operator!=(const Emptyable &right) const
    {
        return !(operator==(right));
    }

    bool operator!=(const T &right) const
    {
        return !(operator==(right));
    }

    bool operator<(const Emptyable &right) const
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

    bool operator<(const T &right) const
    {
        if (this->isEmpty())
        {
            return true;
        }

        return this->m_val < right;
    }

    Emptyable &operator=(const Emptyable &right)
    {
        m_val = right.m_val;
        m_empty = right.m_empty;
        return *this;
    }

    Emptyable &operator=(const T &val)
    {
        m_val = val;
        m_empty = false;
        return *this;
    }

    operator T() const
    {
        assert(m_empty == false);
        return m_val;
    }

    void reset()
    {
        m_empty = true;
    }

private:
    T m_val;
    bool m_empty;
};
}

