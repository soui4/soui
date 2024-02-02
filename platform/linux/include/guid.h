#ifndef __SGUID_H_
#define __SGUID_H_

typedef struct _GUID {
    uint32_t  Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[ 8 ];
} GUID;

#define GuidEqual(id1,id2) (memcmp(&id1,&id2,sizeof(GUID))==0)

#ifdef __cplusplus

inline bool operator==(const GUID & id1,const GUID &id2){
	return GuidEqual(id1,id2);
}

#define DECLARE_CLASS_SIID(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	static GUID GetIID(){  static GUID id ={l, w1, w2, {b1, b2, b3, b4, b5, b6, b7, b8}};return id;}

#else

#define DECLARE_CLASS_SIID(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)

#endif

#define __suidof(ClassName) ClassName::GetIID()

#ifdef __cplusplus
#define REFGUID const GUID &
#else
#define REFGUID const GUID *
#endif



#endif//__SGUID_H_