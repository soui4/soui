#ifndef _SSHAREDPTR_H_
#define _SSHAREDPTR_H_

#include <helper/SCriticalSection.h>

SNSBEGIN

/**
	* This class is used in SSharedPtr to delete a object pointer.
	*/
	template <class T>
	class PtrDisposer
	{
	public:
		virtual ~PtrDisposer(){}
		/**
		* Delete pointer ptr.
		* @param ptr pointer to be deleted.
		*/
		virtual void dispose(T *ptr) = 0;
	};

	template<class T>
	class DefaultPtrDisposer : public PtrDisposer<T>
	{
	public:
		virtual void dispose(T *ptr)
		{
			if (ptr)
			{
				delete ptr;
			}
		}
	};

	template <class T,class Disposer=DefaultPtrDisposer<T>>
	class SSharedCount
	{
	public:
		SSharedCount(T *ptr) : _ptr(ptr), _refCount(1), _cs(), _disposer(new Disposer())
		{
		}

		~SSharedCount()
		{
			_disposer->dispose(_ptr);
			delete _disposer;
		}

		void incRefCount()
		{
			SAutoLock autoLock(_cs);
			++_refCount;
		}

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
		T *_ptr;
		unsigned int _refCount;
		SCriticalSection _cs;
		PtrDisposer<T> *_disposer;
	};


	/**
	 * SharedPtr is a class to encapsulate an object pointer. When a pointer is
	 * put into SharedPtr object, the object reference count will increase. When
	 * a SharedPtr object is destructed, the object reference count will decrease.
	 * If the object reference count is decreased to 0, the object will be deleted
	 * automatically.
	 */
	template <class T,class Disposer=DefaultPtrDisposer<T> >
	class SSharedPtr
	{
	public:
		/**
		 * Default constructor.
		 * @param ptr object pointer.
		 */
		SSharedPtr() : _ptr(NULL), _sc(new SSharedCount<T,Disposer>(NULL))
		{
		}

		/**
		 * Constructor.
		 * @param ptr object pointer.
		 */
		SSharedPtr(T *ptr) : _ptr(ptr), _sc(new SSharedCount<T,Disposer>(ptr))
		{
		}

		/**
		 * Copy constructor.
		 * @param obj another SharedPtr object.
		 */
		SSharedPtr(const SSharedPtr<T,Disposer> &obj) : _ptr(obj._ptr), _sc(obj._sc)
		{
			obj._sc->incRefCount();
		}

		/**
		 * Destructor.
		 */
		~SSharedPtr()
		{
			_sc->decRefCount();
		}

		/**
		 * Assign. The original object reference count will be decreased, and
		 * the new object reference count will be increased.
		 * @param obj another SharedPtr object.
		 * @return self object.
		 */
		SSharedPtr &operator=(const SSharedPtr<T,Disposer> &obj)
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
		 * Return the object pointer.
		 * @return object pointer.
		 */
		const T *operator->() const
		{
			return _ptr;
		}

		/**
		 * Return the object pointer.
		 * @return object pointer.
		 */
		T *operator->()
		{
			return _ptr;
		}

		/**
		 * Return the object pointer.
		 * @return object pointer.
		 */
		const T *ptr() const
		{
			return _ptr;
		}

		/**
		 * Return the object pointer.
		 * @return object pointer.
		 */
		T *ptr()
		{
			return _ptr;
		}

		/**
		 * Return the object reference.
		 * @return object reference.
		 */
		const T &operator*() const
		{
			return *_ptr;
		}

		/**
		 * Return the object reference.
		 * @return object reference.
		 */
		T &operator*()
		{
			return *_ptr;
		}

		/**
		 * Return if the pointer equals to another.
		 * @return true if equal.
		 */
		bool operator==(T *ptr) const
		{
			return _ptr == ptr;
		}

		/**
		 * Return if the pointer equals to another.
		 * @return true if equal.
		 */
		bool operator==(SSharedPtr<T,Disposer> obj) const
		{
			return _ptr == obj._ptr;
		}

	private:
		T *_ptr;
		SSharedCount<T,Disposer> *_sc;
	};

SNSEND

#endif//_SSHAREDPTR_H_