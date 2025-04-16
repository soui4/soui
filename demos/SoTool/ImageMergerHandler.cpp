#include "stdafx.h"
#include "ImageMergerHandler.h"
#include "droptarget.h"
#include <FileHelper.h>
#include <string>
#include <vector>
#include <algorithm>

CImageMergerHandler::CImageMergerHandler(void):m_pPageRoot(NULL),m_pImgCanvas(NULL)
{
}

CImageMergerHandler::~CImageMergerHandler(void)
{
}

static int SortCmp(const std::wstring &a, const std::wstring &b){
	return a < b;
}

void CImageMergerHandler::OnFileDropdown( HDROP hdrop )
{
    bool success = false;
    WCHAR filename[MAX_PATH];
    UINT files = DragQueryFile(hdrop,(UINT)-1,NULL,0);
	std::vector<std::wstring> fileList;
    for(UINT i=0;i<files;i++)
    {
        success=!!DragQueryFileW(hdrop, i, filename, MAX_PATH);
        if(success)
        {
			fileList.push_back(filename);
        }
    }
	std::sort(fileList.begin(),fileList.end(),SortCmp);
	for(UINT i=0;i<fileList.size();i++){
		AddFile(fileList[i].c_str());
	}

}

void CImageMergerHandler::OnInit(SWindow *pRoot)
{
    m_pPageRoot = pRoot->FindChildByName(L"page_imagemerge");
    m_pImgCanvas = m_pPageRoot->FindChildByName2<SImgCanvas>(L"wnd_canvas");//CDropTarget_Canvas
    IDropTarget *pDT = new CDropTarget(this);
    pRoot->GetContainer()->RegisterDragDrop(m_pImgCanvas->GetSwnd(),pDT);
    pDT->Release();
}

void CImageMergerHandler::AddFile(LPCWSTR pszFileName)
{
    m_pImgCanvas->AddFile(pszFileName);
}

void CImageMergerHandler::OnSave()
{
	if (!m_pImgCanvas->IsCanSave())
		return;
    CFileDialogEx dlgSave(FALSE,_T("png"),0,6,_T("png files(*.png)\0*.png\0All files (*.*)\0*.*\0\0"));
    if(dlgSave.DoModal()==IDOK)
    {
        m_pImgCanvas->Save2File(S_CT2W(dlgSave.m_szFileName));
    }
}

void CImageMergerHandler::OnSaveToIco()
{
	if (!m_pImgCanvas->IsCanSave())
		return;
	CFileDialogEx dlgSave(FALSE, _T("ico"), 0, 6, _T("ico files(*.ico)\0*.ico\0\0"));
	if (dlgSave.DoModal() == IDOK)
	{
		m_pImgCanvas->PushIconSize(16);
		m_pImgCanvas->PushIconSize(20);
		m_pImgCanvas->PushIconSize(24);
		m_pImgCanvas->PushIconSize(32);
		m_pImgCanvas->PushIconSize(40);
		m_pImgCanvas->PushIconSize(48);
		m_pImgCanvas->PushIconSize(64);
		m_pImgCanvas->Save2IconFile(S_CT2W(dlgSave.m_szFileName));
	}
}

void CImageMergerHandler::OnClear()
{
    m_pImgCanvas->Clear();
}

void CImageMergerHandler::OnModeHorz()
{
    if(!m_pImgCanvas) return;
    m_pImgCanvas->SetVertical(FALSE);
}

void CImageMergerHandler::OnModeVert()
{
    if(!m_pImgCanvas) return;
    m_pImgCanvas->SetVertical(TRUE);
}

void CImageMergerHandler::OnSplit()
{
    int nNumber = _ttoi(m_pPageRoot->FindChildByName(L"edit_number")->GetWindowText());
    if(nNumber<2 || nNumber>100)
    {
        SMessageBox(GetActiveWindow(),_T("支持切分为2-100子图"),_T("输入错误"),MB_ICONERROR|MB_OK);
    }else
    {
		m_pImgCanvas->Split(nNumber);
    }
}

void CImageMergerHandler::OnSaveSplits()
{
	CFileDialogEx dlgSave(FALSE,_T("png"),0,6,_T("png files(*.png)\0*.png\0All files (*.*)\0*.*\0\0"));
	if(dlgSave.DoModal() == IDOK)
	{
		m_pImgCanvas->SaveSplits(S_CT2W(dlgSave.m_szFileName));
	}
}

