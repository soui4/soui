#ifndef __PROPBAG_H__
#define __PROPBAG_H__

#include <core/SSingleton.h>
#include <cnchessProtocol.h>
#include <windows.h>
SNSBEGIN
class PropBag : public SSingleton<PropBag>
{
    WORD m_wPort;
public:
    enum{
        DEF_PORT = 3311,
    };
    DWORD m_dwProps[MAX_PROP];

    DWORD GetProp(DWORD dwProp) const;
    
    void Init(LPCTSTR pszPropXml);

    WORD GetPort() const;
public:
    PropBag();
    ~PropBag();

};

SNSEND

#endif//__PROPBAG_H__
