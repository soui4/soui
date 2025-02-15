#ifndef __SGUID_H_
#define __SGUID_H_

#define __suidof(ClassName) IID_##ClassName

#ifdef __cplusplus

#define DECLARE_CLASS_SIID(cls_id) \
	static GUID GetIID(){ return cls_id;}

template<class T>
GUID TGuidOf(const T * obj=NULL)// specialize by class
{
    return T::GetIID();
}

#else

#define DECLARE_CLASS_SIID(cls_id)

#endif


#endif//__SGUID_H_