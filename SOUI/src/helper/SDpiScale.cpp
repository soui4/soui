#include "souistd.h"
#include "helper/SDpiScale.h"

SNSBEGIN
static const int KBuiltinScales[] = { 100, 125, 150, 200, 250, 300 };

int SDpiScale::GetBuiltinScaleCount()
{
    return ARRAYSIZE(KBuiltinScales);
}

const int *SDpiScale::GetBuiltinScales()
{
    return KBuiltinScales;
}

/** Normalized zoom ratio; choose a ratio one step smaller than the specified ratio */
int SDpiScale::NormalizeScale(int nScale)
{
    for (int i = 1; i < ARRAYSIZE(KBuiltinScales); i++)
    {
        if (nScale < KBuiltinScales[i])
        {
            return KBuiltinScales[i - 1];
        }
    }
    return KBuiltinScales[ARRAYSIZE(KBuiltinScales) - 1];
}
SNSEND
