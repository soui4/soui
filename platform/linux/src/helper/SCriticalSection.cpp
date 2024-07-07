#include <helper/SCriticalSection.h>
#include <mutex>
SNSBEGIN

	class SCriticalSectionImpl: public std::recursive_mutex{
	};

	SCriticalSection::SCriticalSection()
	{
		m_cs = new SCriticalSectionImpl;
	}
	SCriticalSection::~SCriticalSection()
	{
		delete m_cs;
	}

	void SCriticalSection::Enter()
	{
		m_cs->lock();
	}

	void SCriticalSection::Leave()
	{
		m_cs->unlock();
	}

SNSEND