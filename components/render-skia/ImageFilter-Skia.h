#pragma once

#include <interface/SImageFilter-i.h>
#include <helper/obj-ref-impl.hpp>
#include <core/SkRefCnt.h>
#include <core/SkImageFilter.h>

SNSBEGIN

template <class T>
class TImageFilterBase : public TObjRefImpl<T>{
  public:

    STDMETHOD_(void *, GetPtr)(THIS) OVERRIDE
    {
        return m_pImageFilter.get();
    }
  protected:
    SkAutoTUnref<SkImageFilter> m_pImageFilter;
};

class SDropShadowImageFilter : public TImageFilterBase<IDropShadowImageFilter> {
public:
    SDropShadowImageFilter(void);
    ~SDropShadowImageFilter(void);

    STDMETHOD_(void, Init)(THIS_ float dx, float dy, float sigmaX, float sigmaY, COLORREF color, IImageFilter *input) OVERRIDE;
};

class SBlurImageFilter : public TImageFilterBase<IBlurImageFilter> {
public:
    SBlurImageFilter(void);
    ~SBlurImageFilter(void);

    STDMETHOD_(void, Init)(THIS_ float sigmaX, float sigmaY, IImageFilter *input) OVERRIDE;
};

class SComposeImageFilter : public TImageFilterBase<IComposeImageFilter> {
public:
    SComposeImageFilter(void);
    ~SComposeImageFilter(void);

    STDMETHOD_(void, Init)(THIS_ IImageFilter *outer, IImageFilter *inner) OVERRIDE;
};

class SMergeImageFilter : public TImageFilterBase<IMergeImageFilter> {
public:
    SMergeImageFilter(void);
    ~SMergeImageFilter(void);
    STDMETHOD_(void, Init)(THIS_ IImageFilter *filters[], int count) OVERRIDE;
};

class STileImageFilter : public TImageFilterBase<ITileImageFilter> {
public:
    STileImageFilter(void);
    ~STileImageFilter(void);

    STDMETHOD_(void, Init)(THIS_ RECT srcRect, RECT dstRect, IImageFilter *input) OVERRIDE;
};

class SOffsetImageFilter : public TImageFilterBase<IOffsetImageFilter> {
public:
    SOffsetImageFilter(void);
    ~SOffsetImageFilter(void);

    STDMETHOD_(void, Init)(THIS_ float dx, float dy, IImageFilter *input) OVERRIDE;
};

SNSEND