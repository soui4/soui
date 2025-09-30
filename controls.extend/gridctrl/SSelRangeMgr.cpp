#include <stdafx.h>
#include "SSelRangeMgr.h"

SNSBEGIN
    void SSelRangeMgr::Clear()
    {
        m_lstSelRanges.RemoveAll();
    }

    void SSelRangeMgr::AddRange(const SCellRange &range)
    {
        if (!range.IsValid())
            return;

        // 先检查是否已存在相同的range
        SPOSITION pos = m_lstSelRanges.GetHeadPosition();
        while (pos)
        {
            SCellRange &curRange = m_lstSelRanges.GetNext(pos);
            if (curRange.minRow == range.minRow && curRange.minCol == range.minCol &&
                curRange.maxRow == range.maxRow && curRange.maxCol == range.maxCol)
            {
                return; // 已存在相同范围，无需添加
            }
        }

        // 添加新范围
        m_lstSelRanges.AddTail(range);
        MergeRanges();
    }

    void SSelRangeMgr::RemoveRange(const SCellRange &range)
    {
        if (!range.IsValid())
            return;

        SList<SCellRange> newRanges;
        SPOSITION pos = m_lstSelRanges.GetHeadPosition();

        while (pos)
        {
            SCellRange curRange = m_lstSelRanges.GetNext(pos);
            
            // 如果当前范围与要删除的范围没有交集，则保留
            if (!IsRangesIntersect(curRange, range))
            {
                newRanges.AddTail(curRange);
                continue;
            }

            // 如果当前范围完全包含在要删除的范围内，则删除
            if (range.minRow <= curRange.minRow && range.minCol <= curRange.minCol &&
                range.maxRow >= curRange.maxRow && range.maxCol >= curRange.maxCol)
            {
                continue; // 完全被覆盖，删除该范围
            }

            // 如果要删除的范围完全包含在当前范围内
            if (curRange.minRow <= range.minRow && curRange.minCol <= range.minCol &&
                curRange.maxRow >= range.maxRow && curRange.maxCol >= range.maxCol)
            {
                // 拆分成最多4个新范围
                // 上部分
                if (curRange.minRow < range.minRow)
                {
                    SCellRange topRange(curRange.minRow, curRange.minCol, range.minRow - 1, curRange.maxCol);
                    newRanges.AddTail(topRange);
                }
                // 下部分
                if (curRange.maxRow > range.maxRow)
                {
                    SCellRange bottomRange(range.maxRow + 1, curRange.minCol, curRange.maxRow, curRange.maxCol);
                    newRanges.AddTail(bottomRange);
                }
                // 左部分
                if (curRange.minCol < range.minCol)
                {
                    SCellRange leftRange(
                        smin(curRange.minRow, range.minRow), 
                        curRange.minCol, 
                        smax(curRange.maxRow, range.maxRow), 
                        range.minCol - 1);
                    if (leftRange.IsValid())
                        newRanges.AddTail(leftRange);
                }
                // 右部分
                if (curRange.maxCol > range.maxCol)
                {
                    SCellRange rightRange(
                        smin(curRange.minRow, range.minRow), 
                        range.maxCol + 1, 
                        smax(curRange.maxRow, range.maxRow), 
                        curRange.maxCol);
                    if (rightRange.IsValid())
                        newRanges.AddTail(rightRange);
                }
                continue;
            }

            // 部分重叠的情况
            // 上部分
            if (curRange.minRow < range.minRow)
            {
                SCellRange topRange(curRange.minRow, curRange.minCol, range.minRow - 1, curRange.maxCol);
                if (topRange.IsValid())
                    newRanges.AddTail(topRange);
            }
            // 下部分
            if (curRange.maxRow > range.maxRow)
            {
                SCellRange bottomRange(range.maxRow + 1, curRange.minCol, curRange.maxRow, curRange.maxCol);
                if (bottomRange.IsValid())
                    newRanges.AddTail(bottomRange);
            }
            // 中间部分 - 左边
            if (curRange.minCol < range.minCol)
            {
                SCellRange leftRange(
                    smax(curRange.minRow, range.minRow),
                    curRange.minCol,
                    smin(curRange.maxRow, range.maxRow),
                    range.minCol - 1);
                if (leftRange.IsValid())
                    newRanges.AddTail(leftRange);
            }
            // 中间部分 - 右边
            if (curRange.maxCol > range.maxCol)
            {
                SCellRange rightRange(
                    smax(curRange.minRow, range.minRow),
                    range.maxCol + 1,
                    smin(curRange.maxRow, range.maxRow),
                    curRange.maxCol);
                if (rightRange.IsValid())
                    newRanges.AddTail(rightRange);
            }
        }

        m_lstSelRanges.RemoveAll();
        m_lstSelRanges.AddHeadList(&newRanges);
        MergeRanges();
    }

    BOOL SSelRangeMgr::IsSelected(int row, int col) const
    {
        SPOSITION pos = m_lstSelRanges.GetHeadPosition();
        while (pos)
        {
            const SCellRange &range = m_lstSelRanges.GetNext(pos);
            if (range.InRange(row, col))
                return TRUE;
        }
        return FALSE;
    }

    BOOL SSelRangeMgr::IsSelected(const SCellID &cellID) const
    {
        return IsSelected(cellID.row, cellID.col);
    }
    BOOL SSelRangeMgr::IsEmpty() const
    {
        return m_lstSelRanges.IsEmpty();
    }
    const SList<SCellRange> &SSelRangeMgr::GetRanges() const
    {
        return m_lstSelRanges;
    }

    SCellRange SSelRangeMgr::GetBoundingRange() const
    {
        if (m_lstSelRanges.IsEmpty())
            return SCellRange();

        SPOSITION pos = m_lstSelRanges.GetHeadPosition();
        SCellRange firstRange = m_lstSelRanges.GetNext(pos);
        int minRow = firstRange.minRow;
        int minCol = firstRange.minCol;
        int maxRow = firstRange.maxRow;
        int maxCol = firstRange.maxCol;

        while (pos)
        {
            const SCellRange &range = m_lstSelRanges.GetNext(pos);
            minRow = smin(minRow, range.minRow);
            minCol = smin(minCol, range.minCol);
            maxRow = smax(maxRow, range.maxRow);
            maxCol = smax(maxCol, range.maxCol);
        }

        return SCellRange(minRow, minCol, maxRow, maxCol);
    }

    BOOL SSelRangeMgr::IsRangesIntersect(const SCellRange &range1, const SCellRange &range2) const
    {
        return !(range1.maxRow < range2.minRow || range1.minRow > range2.maxRow ||
                 range1.maxCol < range2.minCol || range1.minCol > range2.maxCol);
    }

    SCellRange SSelRangeMgr::MergeTwoRanges(const SCellRange &range1, const SCellRange &range2) const
    {
        int minRow = smin(range1.minRow, range2.minRow);
        int minCol = smin(range1.minCol, range2.minCol);
        int maxRow = smax(range1.maxRow, range2.maxRow);
        int maxCol = smax(range1.maxCol, range2.maxCol);
        return SCellRange(minRow, minCol, maxRow, maxCol);
    }

    BOOL SSelRangeMgr::CanMergeRanges(const SCellRange &range1, const SCellRange &range2) const
    {
        // 如果两个范围相交或相邻，则可以合并
        SCellRange merged = MergeTwoRanges(range1, range2);
        int area1 = range1.GetRowSpan() * range1.GetColSpan();
        int area2 = range2.GetRowSpan() * range2.GetColSpan();
        int mergedArea = merged.GetRowSpan() * merged.GetColSpan();
        // 如果合并后的面积等于或小于两个原始面积之和，说明可以合并
        // 这里使用小于等于是因为可能存在部分重叠的情况
        return mergedArea <= area1 + area2;
    }

    void SSelRangeMgr::MergeRanges()
    {
        if (m_lstSelRanges.GetCount() <= 1)
            return;

        BOOL merged = TRUE;
        while (merged)
        {
            merged = FALSE;
            SList<SCellRange> newRanges;
            SPOSITION pos1 = m_lstSelRanges.GetHeadPosition();

            // 将第一个元素添加到新列表中
            if (pos1)
            {
                newRanges.AddTail(m_lstSelRanges.GetNext(pos1));
            }

            // 遍历剩余的范围
            while (pos1)
            {
                SCellRange range1 = m_lstSelRanges.GetNext(pos1);
                BOOL foundMerge = FALSE;

                // 检查是否能与新列表中的任何范围合并
                SPOSITION pos2 = newRanges.GetHeadPosition();
                while (pos2)
                {
                    SCellRange &range2 = newRanges.GetAt(pos2);
                    if (CanMergeRanges(range1, range2))
                    {
                        // 合并范围
                        range2 = MergeTwoRanges(range1, range2);
                        foundMerge = TRUE;
                        merged = TRUE;
                        break;
                    }
                    pos2 = newRanges.Next(pos2);
                }

                // 如果没有找到可以合并的范围，则添加到新列表中
                if (!foundMerge)
                {
                    newRanges.AddTail(range1);
                }
            }

            // 更新范围列表
            m_lstSelRanges.RemoveAll();
            m_lstSelRanges.AddHeadList(&newRanges);
        }
    }

SNSEND