#pragma once

#include <interface/SRender-i.h>
#include <helper/obj-ref-impl.hpp>
#include <core/SkPathMeasure.h>

SNSBEGIN

class SPathMeasure_Skia : public TObjRefImpl<IPathMeasure>
{
public:
	SPathMeasure_Skia(void);
	~SPathMeasure_Skia(void);

	STDMETHOD_(void,setPath)(THIS_ IPathS * path, BOOL forceClosed) OVERRIDE;

	STDMETHOD_(float,getLength)(THIS)  OVERRIDE;

	STDMETHOD_(BOOL,getPosTan)(THIS_ float distance, float pos[], float tan[])  OVERRIDE;

	STDMETHOD_(BOOL,getSegment)(THIS_ float startD, float stopD, IPathS * dst, BOOL startWithMoveTo)  OVERRIDE;

private:
	SkPathMeasure	m_pathMeasure;
};

SNSEND