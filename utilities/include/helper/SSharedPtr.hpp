#ifndef _SSHAREDPTR_H_
#define _SSHAREDPTR_H_

#include <helper/SCriticalSection.h>

SNSBEGIN

/**
 * @class PtrDisposer
 * @brief Base class for disposing of a pointer.
 * @tparam T The type of the pointer to be disposed.
 * @details This class provides a virtual interface for deleting a pointer.
 */
template <class T>
class PtrDisposer
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~PtrDisposer() {}

    /**
     * @brief Deletes the pointer.
     * @param ptr Pointer to be deleted.
     */
    virtual void dispose(T *ptr) = 0;
};

/**
 * @class DefaultPtrDisposer
 * @brief Default implementation of PtrDisposer.
 * @tparam T The type of the pointer to be disposed.
 * @details This class provides a default implementation for deleting a pointer using the `delete` operator.
 */
template <class T>
class DefaultPtrDisposer : public PtrDisposer<T>
{
public:
    /**
     * @brief Deletes the pointer.
     * @param ptr Pointer to be deleted.
     */
    virtual void dispose(T *ptr)
    {
        if (ptr)
        {
            delete ptr;
        }
    }
};

/**
 * @class SSharedCount
 * @brief Manages the reference count for a shared pointer.
 * @tparam T The type of the pointer.
 * @tparam Disposer The disposer class used to delete the pointer.
 * @details This class maintains the reference count and ensures that the pointer is deleted when the reference count reaches zero.
 */
template <class T, class Disposer = DefaultPtrDisposer<T>>
class SSharedCount
{
public:
    /**
     * @brief Constructor.
     * @param ptr Pointer to be managed.
     */
    SSharedCount(T *ptr) : _ptr(ptr), _refCount(1), _cs(), _disposer(new Disposer())
    {
    }

    /**
     * @brief Destructor.
     * @details Deletes the managed pointer using the disposer.
     */
    ~SSharedCount()
    {
        _disposer->dispose(_ptr);
        delete _disposer;
    }

    /**
     * @brief Increments the reference count.
     */
    void incRefCount()
    {
        SAutoLock autoLock(_cs);
        ++_refCount;
    }

    /**
     * @brief Decrements the reference count.
     * @details Deletes the managed pointer if the reference count reaches zero.
     */
    void decRefCount()
    {
        bool shouldDelete = false;

        {
            SAutoLock autoLock(_cs);

            if (_refCount > 0)
            {
                --_refCount;
            }

            if (_refCount == 0)
            {
                shouldDelete = true;
            }
        }

        if (shouldDelete)
        {
            delete this;
        }
    }

private:
    T *_ptr;                /**< Pointer to the managed object. */
    unsigned int _refCount; /**< Reference count. */
    SCriticalSection _cs;   /**< Critical section for thread-safe operations. */
    PtrDisposer<T> *_disposer; /**< Pointer to the disposer. */
};

/**
 * @class SSharedPtr
 * @brief A smart pointer class that manages the lifetime of an object using reference counting.
 * @tparam T The type of the pointer.
 * @tparam Disposer The disposer class used to delete the pointer.
 * @details This class automatically deletes the managed object when the last reference to it is destroyed.
 */
template <class T, class Disposer = DefaultPtrDisposer<T>>
class SSharedPtr
{
public:
    /**
     * @brief Default constructor.
     * @details Initializes the shared pointer with a null pointer.
     */
    SSharedPtr() : _ptr(NULL), _sc(new SSharedCount<T, Disposer>(NULL))
    {
    }

    /**
     * @brief Constructor.
     * @param ptr Pointer to be managed.
     */
    SSharedPtr(T *ptr) : _ptr(ptr), _sc(new SSharedCount<T, Disposer>(ptr))
    {
    }

    /**
     * @brief Copy constructor.
     * @param obj Another SSharedPtr object.
     * @details Increments the reference count of the managed pointer.
     */
    SSharedPtr(const SSharedPtr<T, Disposer> &obj) : _ptr(obj._ptr), _sc(obj._sc)
    {
        obj._sc->incRefCount();
    }

    /**
     * @brief Destructor.
     * @details Decrements the reference count of the managed pointer.
     */
    ~SSharedPtr()
    {
        _sc->decRefCount();
    }

    /**
     * @brief Assignment operator.
     * @param obj Another SSharedPtr object.
     * @return Reference to the current object.
     * @details Decrements the reference count of the original pointer and increments the reference count of the new pointer.
     */
    SSharedPtr &operator=(const SSharedPtr<T, Disposer> &obj)
    {
        if (this != &obj)
        {
            obj._sc->incRefCount();
            _sc->decRefCount();
            _sc = obj._sc;
            _ptr = obj._ptr;
        }

        return *this;
    }

    /**
     * @brief Returns the object pointer.
     * @return Const pointer to the managed object.
     */
    const T *operator->() const
    {
        return _ptr;
    }

    /**
     * @brief Returns the object pointer.
     * @return Pointer to the managed object.
     */
    T *operator->()
    {
        return _ptr;
    }

    /**
     * @brief Returns the object pointer.
     * @return Const pointer to the managed object.
     */
    const T *ptr() const
    {
        return _ptr;
    }

    /**
     * @brief Returns the object pointer.
     * @return Pointer to the managed object.
     */
    T *ptr()
    {
        return _ptr;
    }

    /**
     * @brief Returns the object reference.
     * @return Const reference to the managed object.
     */
    const T &operator*() const
    {
        return *_ptr;
    }

    /**
     * @brief Returns the object reference.
     * @return Reference to the managed object.
     */
    T &operator*()
    {
        return *_ptr;
    }

    /**
     * @brief Compares the pointer with another pointer.
     * @param ptr Pointer to compare with.
     * @return TRUE if the pointers are equal, FALSE otherwise.
     */
    bool operator==(T *ptr) const
    {
        return _ptr == ptr;
    }

    /**
     * @brief Compares the pointer with another SSharedPtr object.
     * @param obj SSharedPtr object to compare with.
     * @return TRUE if the pointers are equal, FALSE otherwise.
     */
    bool operator==(const SSharedPtr<T, Disposer> &obj) const
    {
        return _ptr == obj._ptr;
    }

	void reset(T * ptr){
		if (_ptr != ptr)
		{
			_sc->decRefCount();
			_ptr = NULL;
			_sc = new SSharedCount<T, Disposer>(ptr);
			_ptr = ptr;
		}
	}

	bool operator !() const{
		return _ptr==NULL;
	}

	operator bool() const{
		return _ptr!=NULL;
	}
private:
    T *_ptr;                        /**< Pointer to the managed object. */
    SSharedCount<T, Disposer> *_sc; /**< Pointer to the shared count object. */
};

SNSEND

#endif // _SSHAREDPTR_H_