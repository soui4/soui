#ifndef __SGUID_H_
#define __SGUID_H_

typedef struct _SGUID{
	int a,b,c,d;
}SGUID,*PSGUID;

#define SGuidEqual(id1,id2) (id1.a==id2.a && id1.b==id2.b && id1.c==id2.c && id1.d==id2.d)

#ifdef __cplusplus

inline bool operator==(const SGUID & id1,const SGUID &id2){
	return SGuidEqual(id1,id2);
}

#define DECLARE_CLASS_SIID(id0, id1, id2, id3) \
	static SGUID GetIID(){  static SGUID id ={id0,id1,id2,id3};return id;}

#else

#define DECLARE_CLASS_SIID(id0, id1, id2, id3)

#endif

#define __suidof(ClassName) ClassName::GetIID()

#ifdef __cplusplus
#define REFSGUID const SGUID &
#else
#define REFSGUID const SGUID *
#endif



#endif//__SGUID_H_