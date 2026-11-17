#pragma once

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <string>
#include "GalleryCatalog.h"

// Demo-local, renderer-independent contracts shared by the UI and its tests.
namespace Gallery
{
struct LowerChar
{
    wchar_t operator()(wchar_t c) const
    {
        return static_cast<wchar_t>(std::towlower(c));
    }
};

inline std::wstring Lower(const std::wstring &text)
{
    std::wstring result(text);
    std::transform(result.begin(), result.end(), result.begin(), LowerChar());
    return result;
}

inline bool Matches(int index, const std::wstring &query)
{
    if (index < 0 || index >= CategoryCount)
        return false;
    // Ignore leading/trailing whitespace; an empty filter reveals every category.
    const size_t begin = query.find_first_not_of(L" \t\r\n");
    if (begin == std::wstring::npos)
        return true;
    const size_t end = query.find_last_not_of(L" \t\r\n");
    const std::wstring needle = Lower(query.substr(begin, end - begin + 1));
    const Category &category = Categories()[index];
    if (Lower(category.title).find(needle) != std::wstring::npos || Lower(category.key).find(needle) != std::wstring::npos || Lower(category.api).find(needle) != std::wstring::npos)
        return true;
    for (int control = 0; control < RegisteredControlCount; ++control)
        if (Controls()[control].category == index && Lower(Controls()[control].tag).find(needle) != std::wstring::npos)
            return true;
    return false;
}

inline int AnimateValue(int from, int to, unsigned elapsed, unsigned duration)
{
    if (!duration || elapsed >= duration)
        return to;
    // Short, monotonic ease-out; finish exactly at the target, independent of timer frequency.
    const double t = static_cast<double>(elapsed) / duration;
    const double eased = 1.0 - (1.0 - t) * (1.0 - t) * (1.0 - t);
    // std::lround is C++11; round half away from zero like it does.
    const double value = (to - from) * eased;
    return from + static_cast<int>(value < 0 ? value - 0.5 : value + 0.5);
}

inline int CustomRangePercent(int value)
{
    return ((std::max)(20, (std::min)(value, 80)) - 20) * 100 / 60;
}
} // namespace Gallery
