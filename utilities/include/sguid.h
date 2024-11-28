#ifndef __SGUID_H_
#define __SGUID_H_

#ifdef __cplusplus

#define DECLARE_CLASS_SIID(cls_id) \
	static GUID GetIID(){ return cls_id;}

#else

#define DECLARE_CLASS_SIID(cls_id)

#endif

#define __suidof(ClassName) ClassName::GetIID()

#endif//__SGUID_H_