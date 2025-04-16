#pragma once

#include "droptarget.h"
#include "SEdit2.h"

class CFolderHander : public IFileDropHandler
{
    struct LANGEXTS
    {
        SOUI::SStringW strLang;
        SOUI::SStringW strExts;
    };

public:
    CFolderHander(void);
    ~CFolderHander(void);

    void OnInit(SOUI::SWindow *pFolderRoot);
    void InitDir(const SOUI::SStringW & strPath,BOOL bInput=FALSE);
protected:
    virtual void OnFileDropdown(HDROP hDrop);

    void OnBtnFileTypes(SOUI::EventArgs *pEvt);
    void OnDirEnterFinish(SOUI::EventArgs *pEvt);
    
    void InitLang(SXmlNode xmlNode);
    void InitDirTree(HSTREEITEM hTreeItem,const SOUI::SStringW & strPath);

    EVENT_MAP_BEGIN()
        EVENT_NAME_HANDLER(L"btn_dropdown_filetypes",SOUI::EventCmd::EventID, OnBtnFileTypes)
        EVENT_NAME_HANDLER(L"edit_dir",SOUI::EventKeyEnter::EventID,OnDirEnterFinish)
    EVENT_MAP_BREAK()

    SOUI::SWindow *   m_pFolderRoot;
    SOUI::SStringW    m_strDir;
    SOUI::STreeCtrl * m_pDirTree;

protected:

    typedef SOUI::SArray<LANGEXTS> LANGEXTLIST;
    LANGEXTLIST m_lstLangExts;
};
