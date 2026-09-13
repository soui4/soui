#ifndef __SOUILAYOUTPARAMSTRUCT__H__
#define __SOUILAYOUTPARAMSTRUCT__H__

#include <core/SLayoutSize.h>

#define POSFLAG_REFCENTER    '|' /**< Reference parent window center */
#define POSFLAG_REFPREV_NEAR '[' /**< Reference the near edge of the previous sibling window */
#define POSFLAG_REFNEXT_NEAR ']' /**< Reference the near edge of the next sibling window */
#define POSFLAG_REFPREV_FAR  '{' /**< Reference the far edge of the previous sibling window */
#define POSFLAG_REFNEXT_FAR  '}' /**< Reference the far edge of the next sibling window */
#define POSFLAG_PERCENT      '%' /**< Define coordinate as percentage of parent window */
#define POSFLAG_SIZE         '@' /**< Define window size in pos attribute, only valid when x2,y2 are defined */

SNSBEGIN

/** Coordinate type */
enum PIT
{

    PIT_NULL = 0,        /**< Invalid definition */
    PIT_NORMAL,          /**< Anchor coordinate */
    PIT_CENTER,          /**< Reference parent window center point, starts with "|" */
    PIT_PERCENT,         /**< Specify percentage of parent window coordinates, starts with "%" */
    PIT_PREV_NEAR,       /**< Reference near edge of previous sibling window, starts with "[" */
    PIT_NEXT_NEAR,       /**< Reference near edge of next sibling window, starts with "]" */
    PIT_PREV_FAR,        /**< Reference far edge of previous sibling window, starts with "{" */
    PIT_NEXT_FAR,        /**< Reference far edge of next sibling window, starts with "}" */
    PIT_SIZE,            /**< Specify window width or height, starts with "@" */
    PIT_SIB_LEFT = 10,   /**< Sibling node's left, used for X */
    PIT_SIB_TOP = 10,    /**< Sibling node's top, same as left, used for Y */
    PIT_SIB_RIGHT = 11,  /**< Sibling node's right, used for X */
    PIT_SIB_BOTTOM = 11, /**< Sibling node's bottom, same as right, used for Y */
};

struct POS_INFO
{
    PIT pit;          /**<Coordinate type */
    int nRefID;       /**<ID used when referencing sibling window by ID, -1 means not referencing a sibling of a specific ID, the ID reference format is like "sib.left@200:10"
                       */
    bool bMinus;      /**<The defined value contains "-". Since -0 cannot be directly represented as nPos, a separate flag bit is needed */
    SLayoutSize nPos; /**<Coordinate value*/
};

struct SouiLayoutParamStruct
{
    int nCount;                 /**< Number of defined coordinates */
    POS_INFO posLeft, posRight; /**< Value defined by pos attribute, valid when nCount >0 */
    POS_INFO posTop, posBottom; /**< Value defined by pos attribute, valid when nCount >0 */

    float fOffsetX, fOffsetY; /**< Window coordinate offset, x += fOffsetX * width, y += fOffsetY * height  */

    SLayoutSize width;  /**<Width defined by the width attribute, valid when nCount==0*/
    SLayoutSize height; /**<Height defined by the height attribute, valid when nCount==0*/
};
SNSEND

#endif /**< __SOUILAYOUTPARAMSTRUCT__H__ */