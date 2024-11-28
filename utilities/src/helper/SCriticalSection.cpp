#include <helper/SCriticalSection.h>
#include <windows.h>

SNSBEGIN

class SCriticalSectionImpl{
public:
	SCriticalSectionImpl(){
		InitializeCriticalSection(&m_cs);
	}
	~SCriticalSectionImpl(){
		DeleteCriticalSection(&m_cs);
	}

	void lock(){
		EnterCriticalSection(&m_cs);
	}

	void unlock(){
		LeaveCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION m_cs;
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