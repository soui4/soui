#ifndef _NO_COPYABLE_H_
#define _NO_COPYABLE_H_

#include <utilities-def.h>
SNSBEGIN
	/**
	* Inheriate this class to make your class cannot be copy and assign.
	*
	*/
	class UTILITIES_API SNoCopyable
	{
	public:
		SNoCopyable() {}
		~SNoCopyable() {}
	private:
		SNoCopyable(const SNoCopyable &);
		const SNoCopyable &operator=(const SNoCopyable &);
	};

SNSEND

#endif//_NO_COPYABLE_H_