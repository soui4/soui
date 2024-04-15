#ifndef __SGUID_H_
#define __SGUID_H_

#include <memory.h>

typedef struct _GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} GUID;

#define IsEqualGUID(id1,id2) (memcmp(&id1,&id2,sizeof(GUID))==0)

#ifdef __cplusplus

inline bool operator==(const GUID & id1,const GUID &id2){
	return IsEqualGUID(id1,id2);
}

#define DECLARE_CLASS_SIID(l1, w1, w2, w3, w4, l2) \
	static GUID GetIID(){  \
	static GUID id ={l1, w1, w2, \
					{HIBYTE(w3),LOBYTE(w3), HIBYTE(w4),LOBYTE(w4),\
					HIBYTE(HIWORD(l2)),LOBYTE(HIWORD(l2)), HIBYTE(LOWORD(l2)),LOBYTE(LOWORD(l2))}}; \
	return id;\
}
#else

#define DECLARE_CLASS_SIID(l, w1, w2, w3, w4,l2)

#endif

#define __suidof(ClassName) ClassName::GetIID()

#ifdef __cplusplus
#define REFGUID const GUID &
#else
#define REFGUID const GUID *
#endif
#define IID GUID
#define REFIID REFGUID
#define REFCLSID REFGUID


#endif//__SGUID_H_