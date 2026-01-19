#pragma once
#include <vector>
#ifdef WIN32
#include <Windows.h>
#endif

namespace DUGON
{
template <typename T>
class NoLockQueue
{
public:
    NoLockQueue(int capacity):
        _rIndex(0),
        _wIndex(0),
        _readingIndex(0),
        _writingIndex(0),
        _data(capacity + 1)
    {
    }

    bool pushBack(T data)
    {
        uint32_t wIndex = 0;
        uint32_t rIndex = 0;
        uint32_t newIndex = 0;

        do
        {
            wIndex = _wIndex;
            rIndex = _readingIndex;

            if ((wIndex + 1) % _data.size() == rIndex)
            {
                //queue full
                return false;
            }

            newIndex = wIndex + 1 >= _data.size() ? 0 : wIndex + 1;
        }

#ifdef WIN32

        while (wIndex != InterlockedCompareExchange(&_wIndex, newIndex, wIndex));

#else

        while (!__sync_bool_compare_and_swap(&_wIndex, wIndex, newIndex));

#endif

        _data[wIndex] = data;

        do
        {
            wIndex = _writingIndex;
            newIndex = wIndex + 1 >= _data.size() ? 0 : wIndex + 1;
        }

#ifdef WIN32

        while (wIndex != InterlockedCompareExchange(&_writingIndex, newIndex, wIndex));

#else

        while (!__sync_bool_compare_and_swap(&_writingIndex, wIndex, newIndex));

#endif

        return true;
    }

    bool popFront(T &ret)
    {
        uint32_t rIndex = 0;
        uint32_t wIndex = 0;
        uint32_t newIndex = 0;

        do
        {
            rIndex = _rIndex;
            wIndex = _writingIndex;

            if (rIndex == wIndex)
            {
                //empty
                return false;
            }

            newIndex = rIndex + 1 >= _data.size() ? 0 : rIndex + 1;
        }

#ifdef WIN32

        while (rIndex != InterlockedCompareExchange(&_rIndex, newIndex, rIndex));

#else

        while (!__sync_bool_compare_and_swap(&_rIndex, rIndex, newIndex));

#endif

        ret = _data[rIndex];

        do
        {
            rIndex = _readingIndex;
            newIndex = rIndex + 1 >= _data.size() ? 0 : rIndex + 1;
        }

#ifdef WIN32

        while (rIndex != InterlockedCompareExchange(&_readingIndex, newIndex, rIndex));

#else

        while (!__sync_bool_compare_and_swap(&_readingIndex, rIndex, newIndex));

#endif

        return true;
    }

    uint32_t size()
    {
        return (_wIndex + _data.size() - _rIndex) % _data.size();
    }

private:
    uint32_t _rIndex;
    uint32_t _wIndex;
    uint32_t _readingIndex;
    uint32_t _writingIndex;

    std::vector<T> _data;
};

}
