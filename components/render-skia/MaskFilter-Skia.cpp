#include "stdafx.h"
#include "MaskFilter-Skia.h"

SNSBEGIN

	SMaskFilter_Blur::SMaskFilter_Blur():m_skBlurMaskFilter(NULL)
	{
	}
	
	SMaskFilter_Blur::~SMaskFilter_Blur()
	{
		if(m_skBlurMaskFilter) m_skBlurMaskFilter->unref();
	}

	void * SMaskFilter_Blur::GetPtr()
	{
		return m_skBlurMaskFilter;
	}


	void SMaskFilter_Blur::Init(float fRadius,BlurStyle eStyle,BlurFlags eFlags)
	{
		if(m_skBlurMaskFilter) m_skBlurMaskFilter->unref();
		m_skBlurMaskFilter = SkBlurMaskFilter::Create((SkBlurStyle)eStyle,fRadius,eFlags);
	}

	//////////////////////////////////////////////////////////////////////////

	SMaskFilter_Emboss::SMaskFilter_Emboss():m_skEmbossMaskFilter(NULL)
	{
	}

	SMaskFilter_Emboss::~SMaskFilter_Emboss()
	{
		if(m_skEmbossMaskFilter) m_skEmbossMaskFilter->unref();
	}

	void * SMaskFilter_Emboss::GetPtr()
	{
		return m_skEmbossMaskFilter;
	}

	void SMaskFilter_Emboss::Init(float direction[3], float ambient, float specular, float blurRadius)
	{
		if(m_skEmbossMaskFilter) m_skEmbossMaskFilter->unref();
		m_skEmbossMaskFilter = SkBlurMaskFilter::CreateEmboss(direction,ambient,specular,blurRadius);
	}

SNSEND