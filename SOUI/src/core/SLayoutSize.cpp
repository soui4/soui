#include "souistd.h"
#include <core/SLayoutSize.h>
#include <interface/slayout-i.h>
#pragma warning(push)
#pragma warning(disable : 4985) // disable the warning message during the include
#include <math.h>               // this is where I would normally get the warning message
#pragma warning(pop)

SNSBEGIN

static const wchar_t *kUnitMap[] = {
    LayoutSize_Unit::kUnit_Px,
    LayoutSize_Unit::kUnit_Dp,
    LayoutSize_Unit::kUnit_Dip,
    LayoutSize_Unit::kUnit_Sp,
};

Unit SLayoutSize::defUnit = px;
SLayoutSize::SLayoutSize(float _fSize)
{
    fSize = _fSize;
    unit = defUnit;
}

SLayoutSize::SLayoutSize(float _fSize, Unit _unit)
{
    fSize = _fSize;
    unit = _unit;
}

static int fround(float v)
{
    return (int)floor(v + 0.5f);
}

bool SLayoutSize::fequal(float a, float b)
{
    return fabs(a - b) < 0.00000001f;
}

bool SLayoutSize::valueEqual(float value)
{
    return fequal(fSize, value);
}

SStringW SLayoutSize::toString() const
{
    // copy from stringstream.
    if (isWrapContent())
        return L"-1";
    else if (isMatchParent())
        return L"-2";
    return SStringW().Format(L"%.*g%s", 8, fSize, kUnitMap[unit]);
}

bool SLayoutSize::isMatchParent() const
{
    return fequal(fSize, SIZE_MATCH_PARENT);
}

void SLayoutSize::setMatchParent()
{
    fSize = SIZE_MATCH_PARENT;
}

bool SLayoutSize::isWrapContent() const
{
    return fequal(fSize, SIZE_WRAP_CONTENT);
}

void SLayoutSize::setWrapContent()
{
    fSize = SIZE_WRAP_CONTENT;
}

void SLayoutSize::setSize(float fSize, Unit unit)
{
    this->fSize = fSize;
    this->unit = unit;
}

bool SLayoutSize::isSpecifiedSize() const
{
    return fSize >= (float)(int)SIZE_SPEC;
}

int SLayoutSize::toPixelSize(int scale) const
{
    if (!isValid())
        return 0;
    if (isMatchParent())
        return SIZE_MATCH_PARENT;
    else if (isWrapContent())
        return SIZE_WRAP_CONTENT;
    else if (unit == px)
        return (int)fSize;
    else // if(unit == dp || unit == dip || unit= sp)
        return (int)fround(fSize * scale / 100);
}

void SLayoutSize::setInvalid()
{
    fSize = SIZE_UNDEF;
}

bool SLayoutSize::isValid() const
{
    return !fequal(fSize, SIZE_UNDEF);
}

bool SLayoutSize::isZero() const
{
    return fequal(fSize, 0.0f);
}

void SLayoutSize::parseString(const SStringW &strSize)
{
    if (strSize.IsEmpty())
        return;
    if (strSize.CompareNoCase(LayoutSize_Style::kSize_WrapContent) == 0)
    {
        setWrapContent();
        return;
    }
    else if (strSize.CompareNoCase(LayoutSize_Style::kSize_MatchParent) == 0 || strSize.CompareNoCase(LayoutSize_Style::kSize_FillParent) == 0)
    {
        setMatchParent();
        return;
    }
    int cUnitSize = 0;
    for (int i = strSize.GetLength() - 1; i >= 0; i--)
    {
        wchar_t c = strSize[i];
        if ((c >= 'a' && c < 'z') || (c >= 'A' && c <= 'z'))
        {
            cUnitSize++;
        }
        else
        {
            break;
        }
    }
    if (cUnitSize > 0)
    {
        SStringW strUnit = strSize.Right(cUnitSize);
        unit = unitFromString(strUnit);
        if (unit == unknow)
            unit = defUnit;
        fSize = (float)_wtof(strSize.Left(strSize.GetLength() - cUnitSize));
    }
    else
    {
        unit = defUnit;
        fSize = (float)_wtof(strSize);
    }
}

SLayoutSize &SLayoutSize::operator=(const SLayoutSize &src)
{
    fSize = src.fSize;
    unit = src.unit;
    return *this;
}

SLayoutSize &SLayoutSize::operator=(const LAYOUTSIZE &src)
{
    fSize = src.fSize;
    unit = src.unit;
    return *this;
}

SLayoutSize SLayoutSize::fromString(const SStringW &strSize)
{
    SLayoutSize ret;
    ret.parseString(strSize);
    return ret;
}

Unit SLayoutSize::unitFromString(const SStringW &strUnit)
{
    Unit ret = unknow;
    SStringW strUnit2 = strUnit;
    strUnit2.MakeLower();
    for (int i = 0; i < ARRAYSIZE(kUnitMap); i++)
    {
        if (strUnit2.Compare(kUnitMap[i]) == 0)
        {
            ret = (Unit)i;
            break;
        }
    }
    return ret;
}

SNSEND
