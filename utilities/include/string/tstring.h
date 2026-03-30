#ifndef __TSTRING_H__
#define __TSTRING_H__


#include <utilities-def.h>
#include <souicoll.h>
#include "sstringa.h"
#include "sstringw.h"

SNSBEGIN

#ifdef _UNICODE
    typedef SStringW                        SStringT;
#else
    typedef SStringA                        SStringT;
#endif


    template< typename T >
    class SStringElementTraits : public CSafeElementTraitsBase<T>
    {
    public:
        typedef typename T::pctstr INARGTYPE;
        typedef T& OUTARGTYPE;


        static ULONG __cdecl Hash(INARGTYPE  str)
        {
            SASSERT(str != NULL);
            ULONG nHash = 0;
            const typename T::_tchar * pch = str;
            while (*pch != 0)
            {
                nHash = (nHash << 5) + nHash + (*pch);
                pch++;
            }

            return(nHash);
        }

        static bool __cdecl CompareElements(INARGTYPE str1, INARGTYPE str2)
        {
			return (T::_tchar_traits::Compare(str1,str2) == 0);
        }

        static int __cdecl CompareElementsOrdered(INARGTYPE str1, INARGTYPE str2)
        {
            return (T::_tchar_traits::Compare(str1,str2));
        }
    };

    template<>
    class CElementTraits< SStringA > :
        public SStringElementTraits< SStringA >
    {
    };

    template<>
    class CElementTraits< SStringW > :
        public SStringElementTraits< SStringW >
    {
    };


SNSEND


#endif    //    __TSTRING_H__
