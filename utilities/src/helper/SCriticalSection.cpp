#include <helper/SCriticalSection.h>

namespace SOUI
{
	SCriticalSection::SCriticalSection()
	{
	}
	SCriticalSection::~SCriticalSection()
	{
	}

	void SCriticalSection::Enter()
	{
		m_cs.lock();
	}

	void SCriticalSection::Leave()
	{
		m_cs.unlock();
	}
}