#ifndef __SGUID_H_
#define __SGUID_H_

#define GuidEqual(id1,id2) (memcmp(&id1,&id2,sizeof(GUID))==0)

#ifdef __cplusplus

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



#endif//__SGUID_H_