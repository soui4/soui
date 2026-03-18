#include "stdafx.h"
#include "ImageFilter-Skia.h"
#include <vector>
#include <effects/SkDropShadowImageFilter.h>
#include <effects/SkOffsetImageFilter.h>
#include <effects/SkColorFilterImageFilter.h>
#include <effects/SkBlurImageFilter.h>
#include <effects/SkComposeImageFilter.h>
#include <effects/SkMergeImageFilter.h>
#include <effects/SkTileImageFilter.h>

#ifndef GetAValue
#define GetAValue(rgb) (rgb>>24)
#endif //GetAValue
SNSBEGIN

SDropShadowImageFilter::SDropShadowImageFilter(void)
{
}

SDropShadowImageFilter::~SDropShadowImageFilter(void)
{
}

STDMETHODIMP_(void) SDropShadowImageFilter::Init(THIS_ float dx, float dy, float sigmaX, float sigmaY, COLORREF color, IImageFilter *input)
{
    SkColor skColor = SkColorSetARGB(
        GetAValue(color),
        GetRValue(color),
        GetGValue(color),
        GetBValue(color)
    );

    SkImageFilter *inputFilter = NULL;
    if (input)
    {
        inputFilter = (SkImageFilter *)input->GetPtr();
        if (inputFilter)
        {
            inputFilter->ref();
        }
    }

    m_pImageFilter.reset(SkDropShadowImageFilter::Create(dx, dy, sigmaX, sigmaY, skColor, inputFilter));

    if (inputFilter)
    {
        inputFilter->unref();
    }
}

SBlurImageFilter::SBlurImageFilter(void)
{
}

SBlurImageFilter::~SBlurImageFilter(void)
{
}

STDMETHODIMP_(void) SBlurImageFilter::Init(THIS_ float sigmaX, float sigmaY, IImageFilter *input)
{
    SkImageFilter *inputFilter = NULL;
    if (input)
    {
        inputFilter = (SkImageFilter *)input->GetPtr();
        if (inputFilter)
        {
            inputFilter->ref();
        }
    }

     m_pImageFilter.reset(SkBlurImageFilter::Create(sigmaX, sigmaY, inputFilter));

    if (inputFilter)
    {
        inputFilter->unref();
    }
}

SComposeImageFilter::SComposeImageFilter(void)
{
}

SComposeImageFilter::~SComposeImageFilter(void)
{
}

STDMETHODIMP_(void) SComposeImageFilter::Init(THIS_ IImageFilter *outer, IImageFilter *inner)
{
    SkImageFilter *outerFilter = NULL;
    if (outer)
    {
        outerFilter = (SkImageFilter *)outer->GetPtr();
        if (outerFilter)
        {
            outerFilter->ref();
        }
    }

    SkImageFilter *innerFilter = NULL;
    if (inner)
    {
        innerFilter = (SkImageFilter *)inner->GetPtr();
        if (innerFilter)
        {
            innerFilter->ref();
        }
    }

     m_pImageFilter.reset( SkComposeImageFilter::Create(outerFilter, innerFilter));

    if (outerFilter)
    {
        outerFilter->unref();
    }

    if (innerFilter)
    {
        innerFilter->unref();
    }
}

SMergeImageFilter::SMergeImageFilter(void)
{
}

SMergeImageFilter::~SMergeImageFilter(void)
{
}

STDMETHODIMP_(void) SMergeImageFilter::Init(THIS_ IImageFilter *filters[], int count)
{
    std::vector<SkImageFilter *> skFilters;
    for (int i = 0; i < count; i++)
    {
        if (filters[i])
        {
            SkImageFilter *filter = (SkImageFilter *)filters[i]->GetPtr();
            if (filter)
            {
                filter->ref();
                skFilters.push_back(filter);
            }
        }
    }

     m_pImageFilter.reset(SkMergeImageFilter::Create(&skFilters[0], skFilters.size()));

    for (int i = 0; i < skFilters.size(); i++)
    {
        skFilters[i]->unref();
    }
}

STileImageFilter::STileImageFilter(void)
{
}

STileImageFilter::~STileImageFilter(void)
{
}

STDMETHODIMP_(void) STileImageFilter::Init(THIS_ RECT srcRect, RECT dstRect, IImageFilter *input)
{
    SkRect skSrcRect = SkRect::MakeXYWH(srcRect.left, srcRect.top, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top);
    SkRect skDstRect = SkRect::MakeXYWH(dstRect.left, dstRect.top, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top);

    SkImageFilter *inputFilter = NULL;
    if (input)
    {
        inputFilter = (SkImageFilter *)input->GetPtr();
        if (inputFilter)
        {
            inputFilter->ref();
        }
    }

     m_pImageFilter.reset(SkTileImageFilter::Create(skSrcRect, skDstRect, inputFilter));

    if (inputFilter)
    {
        inputFilter->unref();
    }
}

SOffsetImageFilter::SOffsetImageFilter(void)
{
}

SOffsetImageFilter::~SOffsetImageFilter(void)
{
}

STDMETHODIMP_(void) SOffsetImageFilter::Init(THIS_ float dx, float dy, IImageFilter *input)
{
    SkImageFilter *inputFilter = NULL;
    if (input)
    {
        inputFilter = (SkImageFilter *)input->GetPtr();
        if (inputFilter)
        {
            inputFilter->ref();
        }
    }

     m_pImageFilter.reset( SkOffsetImageFilter::Create(dx, dy, inputFilter));

    if (inputFilter)
    {
        inputFilter->unref();
    }
}


SNSEND