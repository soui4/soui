#include <helper/SCriticalSection.h>
SNSBEGIN

	class SCriticalSectionImpl{
		public:
		SCriticalSectionImpl(){}
		~SCriticalSectionImpl(){}

		void Lock(){

		}

		void Unlock(){
			
		}
		private:
		CRITICALSECTION m_cs;
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