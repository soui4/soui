#pragma once

#include <cstddef>

// VS2008 only implements C++03. The Fluent helpers need shared ownership and a
// shared boolean flag, which std::shared_ptr/std::make_shared (C++11) do not
// provide there. These small intrusive types keep the existing lifetime
// contract without C++11 syntax or library features.

namespace SNS
{
namespace FluentDemo
{

template <class T>
class SharedPtr
{
  public:
    SharedPtr()
        : m_ptr(NULL)
    {
    }
    explicit SharedPtr(T *ptr)
        : m_ptr(ptr)
    {
    }
    SharedPtr(const SharedPtr &other)
        : m_ptr(other.m_ptr)
    {
        if (m_ptr)
            m_ptr->AddRef();
    }
    ~SharedPtr()
    {
        if (m_ptr)
            m_ptr->Release();
    }

    SharedPtr &operator=(const SharedPtr &other)
    {
        if (m_ptr != other.m_ptr)
        {
            T *previous = m_ptr;
            m_ptr = other.m_ptr;
            if (m_ptr)
                m_ptr->AddRef();
            if (previous)
                previous->Release();
        }
        return *this;
    }

    T *get() const
    {
        return m_ptr;
    }
    T *operator->() const
    {
        return m_ptr;
    }
    T &operator*() const
    {
        return *m_ptr;
    }
    bool operator!() const
    {
        return m_ptr == NULL;
    }
    operator bool() const
    {
        return m_ptr != NULL;
    }
    bool operator==(const SharedPtr &other) const
    {
        return m_ptr == other.m_ptr;
    }
    bool operator!=(const SharedPtr &other) const
    {
        return m_ptr != other.m_ptr;
    }

  private:
    T *m_ptr;
};

// Shared bool: every popup in one menu tree observes the same keyboard state.
class BoolFlag
{
  public:
    explicit BoolFlag(bool value)
        : m_refs(1)
        , m_value(value)
    {
    }
    void AddRef()
    {
        ++m_refs;
    }
    void Release()
    {
        if (--m_refs == 0)
            delete this;
    }
    BoolFlag &operator=(bool value)
    {
        m_value = value;
        return *this;
    }
    operator bool() const
    {
        return m_value;
    }

  private:
    long m_refs;
    bool m_value;
};

typedef SharedPtr<BoolFlag> BoolFlagPtr;

inline BoolFlagPtr MakeBoolFlag(bool value)
{
    return BoolFlagPtr(new BoolFlag(value));
}

} // namespace FluentDemo
} // namespace SNS
