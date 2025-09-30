#ifndef __SSelRangeMgr_H__
#define __SSelRangeMgr_H__

#include "SCellID.h"
#include <souicoll.h>
SNSBEGIN
    class SSelRangeMgr
    {
    public:
        SSelRangeMgr(){}

        void Clear();

        void AddRange(const SCellRange &range);

        void RemoveRange(const SCellRange &range);

        BOOL IsSelected(int row, int col) const;

        BOOL IsSelected(const SCellID &cellID) const;

        const SList<SCellRange> &GetRanges() const;

        SCellRange GetBoundingRange() const;

        BOOL IsEmpty() const;
    private:
        BOOL IsRangesIntersect(const SCellRange &range1, const SCellRange &range2) const;

        SCellRange MergeTwoRanges(const SCellRange &range1, const SCellRange &range2) const;

        BOOL CanMergeRanges(const SCellRange &range1, const SCellRange &range2) const;

        void MergeRanges();

    private:
        SList<SCellRange> m_lstSelRanges;
    };
SNSEND

#endif//__SSelRangeMgr_H__