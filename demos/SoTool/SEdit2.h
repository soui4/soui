#ifndef _SEDIT2_H_
#define _SEDIT2_H_

#include <control/SRichEdit.h>
SNSBEGIN

	DEF_EVT_EXT(EventKeyEnter,EVT_EXTERNAL_BEGIN+500,{int fake;});
    
    class SEdit2 : public SEdit
    {
    DEF_SOBJECT(SEdit,L"edit2")
    public:
        SEdit2(void);
        ~SEdit2(void);
        
    protected:
        void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_KEYDOWN(OnKeyDown)
        SOUI_MSG_MAP_END()
        
    };
SNSEND
#endif//_SEDIT2_H_
