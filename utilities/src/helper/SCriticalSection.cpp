#include <helper/SCriticalSection.h>
#include <mutex>
namespace SOUI
{
	class SCriticalSectionImpl: public std::mutex{
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
}