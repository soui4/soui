#include "stdafx.h"
#include "FolderHander.h"
#include <helper/SplitString.h>
#include "helper/SMenu.h"

//////////////////////////////////////////////////////////////////////////
// CFolderHander
CFolderHander::CFolderHander(void):m_pFolderRoot(NULL)
{
}

CFolderHander::~CFolderHander(void)
{
}

void CFolderHander::OnFileDropdown( HDROP hDrop )
{
    TCHAR filename[MAX_PATH];
    if(DragQueryFile(hDrop, 0, filename, MAX_PATH)!=0) 
    {
        if(GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY)
        {
            InitDir(S_CT2W(filename));
        }
    }

}

void CFolderHander::OnInit( SWindow *pFolderRoot )
{
    m_pFolderRoot = pFolderRoot;
    SASSERT(m_pFolderRoot);

    SWindow *pEditDir = m_pFolderRoot->FindChildByName(L"edit_dir");
    SASSERT(pEditDir);
    IDropTarget *pDT = new CDropTarget(this);
    m_pFolderRoot->GetContainer()->RegisterDragDrop(pEditDir->GetSwnd(),pDT);
    pDT->Release();

    m_pDirTree = m_pFolderRoot->FindChildByName2<STreeCtrl>(L"tree_dir");
}


void CFolderHander::OnDirEnterFinish( EventArgs *pEvt )
{
    SEdit *pEdit = sobj_cast<SEdit>(pEvt->Sender());
    SStringT strDir = pEdit->GetWindowText();
    InitDir(S_CT2W(strDir),TRUE);
}


void CFolderHander::InitDir( const SStringW & strDir,BOOL bInput/*=FALSE */)
{
    DWORD attr = GetFileAttributesW(strDir);
    if(attr==INVALID_FILE_ATTRIBUTES) return ;
    if((attr & FILE_ATTRIBUTE_DIRECTORY) == 0) return;
    if(strDir == m_strDir) return;

    if(!bInput)
    {
        m_pFolderRoot->FindChildByName(L"edit_dir")->SetWindowText(S_CW2T(strDir));
    }
    m_strDir = strDir;
    m_pDirTree->RemoveAllItems();
    HSTREEITEM hRoot = m_pDirTree->InsertItem(_T("root"),0,1);
    InitDirTree(hRoot,m_strDir);
    m_pDirTree->SetCheckState(hRoot,TRUE);
}

void CFolderHander::InitDirTree(HSTREEITEM hTreeItem,const SStringW & strPath)
{
    WIN32_FIND_DATAW fd;
    SStringW strFind = strPath + L"\\*.*";
    HANDLE hFind=FindFirstFileW(strFind,&fd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        while(FindNextFileW(hFind,&fd))
        {
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                && !(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
                SStringW strName= fd.cFileName;
                if(strName != L"." && strName != L"..")
                {
                    HSTREEITEM hItem = m_pDirTree->InsertItem(S_CW2T(strName),0,1,hTreeItem);
                    InitDirTree(hItem,strPath + L"\\" + strName);
                }
            }
        }
        FindClose(hFind);
    }
}

void CFolderHander::OnBtnFileTypes(EventArgs *pEvt)
{
    SWindow *pBtn = sobj_cast<SWindow>(pEvt->Sender());
    CRect rcBtn = pBtn->GetWindowRect();
    HWND hWnd = pBtn->GetContainer()->GetHostHwnd();
    ::ClientToScreen(hWnd,(LPPOINT)&rcBtn);
    ::ClientToScreen(hWnd,(LPPOINT)&rcBtn+1);

    SMenu menu;
    menu.LoadMenu(_T("xml:menu_filetype"));
    for(int i=0;i<(int)m_lstLangExts.GetCount();i++)
    {
        SStringW strDesc = m_lstLangExts[i].strLang+L":"+m_lstLangExts[i].strExts;
        menu.InsertMenu(i,MF_BYPOSITION,i+10,S_CW2T(strDesc),-1);
    }

    int id = menu.TrackPopupMenu(TPM_RIGHTALIGN|TPM_RETURNCMD,rcBtn.right,rcBtn.bottom,hWnd);
    if(id!=0)
    {
        id-=10;
        SEdit *pEdit = m_pFolderRoot->FindChildByName2<SEdit>(L"edit_filetypes");
        SASSERT(pEdit);
        if(pEdit->GetWindowTextLength()!=0)
        {
            pEdit->SetSel((DWORD)-1);
            pEdit->ReplaceSel(_T(";"));            
        }
        pEdit->SetSel((DWORD)-1);
        pEdit->ReplaceSel(S_CW2T(m_lstLangExts[id].strExts));
    }
}

void CFolderHander::InitLang( SXmlNode xmlNode )
{
    SXmlNode xmlLang = xmlNode.child(L"language");
    while(xmlLang)
    {
        LANGEXTS langExts;
        langExts.strLang = xmlLang.attribute(L"name").value();
        langExts.strExts = xmlLang.attribute(L"exts").value();
        m_lstLangExts.Add(langExts);

        xmlLang = xmlLang.next_sibling(L"language");
    }
}
