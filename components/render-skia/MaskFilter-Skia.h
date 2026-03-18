#pragma once

#include <interface/SMaskFilter-i.h>
#include <helper/obj-ref-impl.hpp>
#include <effects/SkBlurMaskFilter.h>
#include <effects/SkEmbossMaskFilter.h>

SNSBEGIN

	class SMaskFilter_Blur : public TObjRefImpl<IMaskFilterBlur>
	{
	public:
		SMaskFilter_Blur();
		~SMaskFilter_Blur();

		STDMETHOD_(void*,GetPtr)(THIS) OVERRIDE;
		STDMETHOD_(void,Init)(THIS_ float fRadius,BlurStyle eStyle,BlurFlags eFlags) OVERRIDE;
	private:
        SkMaskFilter *m_skBlurMaskFilter;
	};

	class SMaskFilter_Emboss : public TObjRefImpl<IMaskFilterEmboss>
	{
	public:
		SMaskFilter_Emboss();
		~SMaskFilter_Emboss();

		STDMETHOD_(void*,GetPtr)(THIS) OVERRIDE;
		
		STDMETHOD_(void,Init)(THIS_ float direction[3], float ambient, float specular, float blurRadius) OVERRIDE;

	private:
        SkMaskFilter *m_skEmbossMaskFilter;
	};

SNSEND

