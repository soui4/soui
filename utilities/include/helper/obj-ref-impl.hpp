// IObjRef的实现类
#ifndef __OBJ_REF_IMPL_HPP
#define __OBJ_REF_IMPL_HPP
#include <windows.h>
#include <sdef.h>
#include <objbase.h>
#include <interface/obj-ref-i.h>

SNSBEGIN

/// @brief Template class implementing the IObjRef interface.
/// @tparam T The base class that implements the IObjRef interface.
template<class T>
class TObjRefImpl : public T
{
public:
    /// @brief Constructor that initializes the reference count to 1.
    TObjRefImpl() : m_cRef(1)
    {
    }

    /// @brief Virtual destructor.
    virtual ~TObjRefImpl()
    {
    }

    /// @brief Increments the reference count.
    /// @return The new reference count.
    STDMETHOD_(long, AddRef)(THIS) override
    {
        return InterlockedIncrement(&m_cRef);
    }

    /// @brief Decrements the reference count and deletes the object if the count reaches zero.
    /// @return The new reference count.
    STDMETHOD_(long, Release)(THIS) override
    {
        long lRet = InterlockedDecrement(&m_cRef);
        if (lRet == 0)
        {
            OnFinalRelease();
        }
        return lRet;
    }

    /// @brief Deletes the object.
    STDMETHOD_(void, OnFinalRelease)(THIS) override
    {
        delete this;
    }

protected:
    /// @brief Reference count.
    LONG m_cRef;
};

/// @brief Template class extending TObjRefImpl with a specific final release behavior.
/// @tparam T The base class that implements the IObjRef interface.
/// @tparam T2 The derived class that should be deleted in OnFinalRelease.
template<class T, class T2>
class TObjRefImpl2 : public TObjRefImpl<T>
{
public:
    /// @brief Deletes the object as the derived type T2.
    STDMETHOD_(void, OnFinalRelease)(THIS) override
    {
        delete static_cast<T2*>(this);
    }
};

/// @brief Smart pointer class for managing COM-style reference-counted objects.
/// @tparam T The type of the object being managed.
template <class T>
class SAutoRefPtr
{
public:
    /// @brief Default constructor that initializes the pointer to NULL.
    SAutoRefPtr()
    {
        p = NULL;
    }

    /// @brief Constructor that takes a pointer and optionally adds a reference.
    /// @param lp The pointer to the object.
    /// @param bAddRef Whether to add a reference to the object.
    SAutoRefPtr(T* lp, BOOL bAddRef = TRUE)
    {
        p = lp;
        if (p != NULL && bAddRef)
        {
            p->AddRef();
        }
    }

    /// @brief Copy constructor that adds a reference to the object.
    /// @param src The source smart pointer.
    SAutoRefPtr(const SAutoRefPtr & src)
    {
        p = src.p;
        if (p)
        {
            p->AddRef();
        }
    }

    /// @brief Destructor that releases the object.
    ~SAutoRefPtr()
    {
        if (p)
        {
            p->Release();
        }
    }

    /// @brief Overloaded operator-> to access members of the object.
    /// @return The pointer to the object.
    T* operator->() const
    {
        return p;
    }

    /// @brief Overloaded cast operator to T*.
    /// @return The pointer to the object.
    operator T*() const
    {
        return p;
    }

    /// @brief Overloaded dereference operator.
    /// @return The reference to the object.
    T& operator*() const
    {
        return *p;
    }

    /// @brief Overloaded address-of operator.
    /// @return The address of the pointer.
    T** operator&()
    {
        SASSERT(p == NULL);
        return &p;
    }

    /// @brief Overloaded not operator to check if the pointer is NULL.
    /// @return TRUE if the pointer is NULL, FALSE otherwise.
    bool operator!() const
    {
        return (p == NULL);
    }

    /// @brief Overloaded less-than operator.
    /// @param pT The pointer to compare with.
    /// @return TRUE if the current pointer is less than pT, FALSE otherwise.
    bool operator<(T* pT) const
    {
        return p < pT;
    }

    /// @brief Overloaded not-equal operator.
    /// @param pT The pointer to compare with.
    /// @return TRUE if the current pointer is not equal to pT, FALSE otherwise.
    bool operator!=(T* pT) const
    {
        return !operator==(pT);
    }

    /// @brief Overloaded equal operator.
    /// @param pT The pointer to compare with.
    /// @return TRUE if the current pointer is equal to pT, FALSE otherwise.
    bool operator==(T* pT) const
    {
        return p == pT;
    }

    /// @brief Overloaded assignment operator from a raw pointer.
    /// @param lp The pointer to assign.
    /// @return The pointer to the object.
    T* operator=(T* lp)
    {
        if (*this != lp)
        {
            if (p)
            {
                p->Release();
            }
            p = lp;
            if (p)
            {
                p->AddRef();
            }
        }
        return *this;
    }

    /// @brief Overloaded assignment operator from another SAutoRefPtr.
    /// @param lp The source smart pointer.
    /// @return The pointer to the object.
    T* operator=(const SAutoRefPtr<T>& lp)
    {
        if (*this != lp)
        {
            if (p)
            {
                p->Release();
            }
            p = lp.p;
            if (p)
            {
                p->AddRef();
            }
        }
        return *this;
    }

    /// @brief Releases the object and sets the pointer to NULL.
    void Release()
    {
        T* pTemp = p;
        if (pTemp)
        {
            p = NULL;
            pTemp->Release();
        }
    }

    /// @brief Attaches to an existing object without adding a reference.
    /// @param p2 The pointer to attach.
    void Attach(T* p2)
    {
        if (p)
        {
            p->Release();
        }
        p = p2;
    }

    /// @brief Detaches the object without releasing it.
    /// @return The pointer to the object.
    T* Detach()
    {
        T* pt = p;
        p = NULL;
        return pt;
    }

    /// @brief Copies the pointer to another location and adds a reference.
    /// @param ppT The destination pointer.
    /// @return S_OK if successful, E_POINTER if ppT is NULL.
    HRESULT CopyTo(T** ppT)
    {
        if (ppT == NULL)
            return E_POINTER;
        *ppT = p;
        if (p)
        {
            p->AddRef();
        }
        return S_OK;
    }

protected:
    /// @brief Pointer to the managed object.
    T* p;
};

/// @brief Macro for compatibility with CAutoRefPtr.
#define CAutoRefPtr SAutoRefPtr

SNSEND

#endif //__OBJ_REF_IMPL_HPP