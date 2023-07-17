//IObjRef的实现类
#ifndef __OBJ_REF_IMPL_HPP
#define __OBJ_REF_IMPL_HPP
#include <sdef.h>

#ifndef SASSERT
#include <assert.h>
#define SASSERT(x) assert(x);
#endif

SNSBEGIN

template<class T>
class TObjRefImpl :  public T
{
public:
	TObjRefImpl():m_cRef(1)
	{
	}

	virtual ~TObjRefImpl(){
	}

	virtual long WINAPI AddRef () override{
		return InterlockedIncrement(&m_cRef);
	}

	virtual long WINAPI Release () override
	{
		long lRet = InterlockedDecrement(&m_cRef);
		if(lRet==0)
		{
			OnFinalRelease();
		}
		return lRet;
	}

	virtual void WINAPI OnFinalRelease () override
    {
        delete this;
    }
protected:
	volatile LONG m_cRef;
};



template<class T,class T2>
class TObjRefImpl2 :  public TObjRefImpl<T>
{
public:
	virtual void WINAPI OnFinalRelease () override
	{
		delete static_cast<T2*>(this);
	}
};

//SAutoRefPtr provides the basis for all other smart pointers
template <class T>
class SAutoRefPtr
{
public:
	SAutoRefPtr() 
	{
		p = NULL;
	}

	SAutoRefPtr(T* lp) 
	{
		p = lp;
		if (p != NULL)
		{
			p->AddRef();
		}
	}

	SAutoRefPtr(const SAutoRefPtr & src)
	{
		p=src.p;
		if(p)
		{
			p->AddRef();
		}
	}

	~SAutoRefPtr()
	{
		if (p)
		{
			p->Release();
		}
	}

	T* operator->() const
	{
		return p;
	}

	operator T*() const 
	{
		return p;
	}
	T& operator*() const
	{
		return *p;
	}
	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T** operator&() 
	{
	    SASSERT(p==NULL);
		return &p;
	}
	bool operator!() const 
	{
		return (p == NULL);
	}
	bool operator<(T* pT) const 
	{
		return p < pT;
	}
	bool operator!=(T* pT) const
	{
		return !operator==(pT);
	}
	bool operator==(T* pT) const 
	{
		return p == pT;
	}

	T* operator=(T* lp) 
	{
		if(*this!=lp)
		{
			if(p)
			{
				p->Release();
			}
			p=lp;
			if(p)
			{
				p->AddRef();
			}
		}
		return *this;
	}

	T* operator=(const SAutoRefPtr<T>& lp) 
	{
		if(*this!=lp)
		{
			if(p)
			{
				p->Release();
			}
			p=lp;
			if(p)
			{
				p->AddRef();
			}
		}
		return *this;	
	}

	// Release the interface and set to NULL
	void Release() 
	{
		T* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}
	}

	// Attach to an existing interface (does not AddRef)
	void Attach(T* p2) 
	{
		if (p)
		{
			p->Release();
		}
		p = p2;
	}
	// Detach the interface (does not Release)
	T* Detach() 
	{
		T* pt = p;
		p = NULL;
		return pt;
	}
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
	T* p;
};



#define CAutoRefPtr SAutoRefPtr	//for compatible

SNSEND

#endif //__OBJ_REF_IMPL_HPP