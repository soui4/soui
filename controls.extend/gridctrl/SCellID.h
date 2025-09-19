#ifndef __SCELLID__H__
#define __SCELLID__H__
#include "SGridCtrl-i.h"

SNSBEGIN

enum{
    kMaxRow = 0x7FFFFFFF,
    kMaxCol=  0x7FFFFFFF,
};

/**
 * @struct SCellID
 * @brief Cell identifier structure
 * @details Structure to identify a specific cell in the grid by row and column
 */
struct SCellID : SCELLID
{
    
    SCellID(int nRow = -1, int nCol = -1) 
    {
        row = nRow;
        col = nCol;
    }
    SCellID(const SCELLID& rhs)
    {
        row = rhs.row;
        col = rhs.col;
    }
    
    BOOL IsValid() const { return (row >= 0 && row!= kMaxRow && col >= 0 && col!=kMaxCol); }
    BOOL operator==(const SCellID& rhs) const { return (row == rhs.row && col == rhs.col); }
    BOOL operator!=(const SCellID& rhs) const { return !(*this == rhs); }

    void operator=(const SCELLID& rhs)
    {
        row = rhs.row;
        col = rhs.col;
    }
};

/**
 * @struct SCellRange
 * @brief Cell range structure
 * @details Structure to define a range of cells in the grid
 */
struct SCellRange : SCELLRANGE
{
    
    SCellRange(int nMinRow = -1, int nMinCol = -1, int nMaxRow = -1, int nMaxCol = -1)
    {
        minRow = nMinRow;
        minCol = nMinCol;
        maxRow = nMaxRow;
        maxCol = nMaxCol;
    }
    
    BOOL IsValid() const { return (minRow >= 0 && minCol >= 0 && maxRow >= minRow && maxCol >= minCol); }
    BOOL InRange(int row, int col) const { return (row >= minRow && row <= maxRow && col >= minCol && col <= maxCol); }
    BOOL InRange(const SCellID& cellID) const { return InRange(cellID.row, cellID.col); }
    
    SCellID GetTopLeft() const { return SCellID(minRow, minCol); }
    int GetRowSpan() const { return maxRow - minRow + 1; }
    int GetColSpan() const { return maxCol - minCol + 1; }
};

SNSEND

#endif//__SCELLID__H__