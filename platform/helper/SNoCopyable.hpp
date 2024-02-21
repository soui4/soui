#pragma once
#include <platform_exp.h>
#include <sdef.h>

SNSBEGIN
	/**
	* Inheriate this class to make your class cannot be copy and assign.
	*
	*/
	class PLATFORM_API SNoCopyable
	{
	public:
		SNoCopyable() {}
		~SNoCopyable() {}
	private:
		SNoCopyable(const SNoCopyable &);
		const SNoCopyable &operator=(const SNoCopyable &);
	};

SNSEND