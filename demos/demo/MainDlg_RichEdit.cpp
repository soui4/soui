/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg_RichEdit.cpp
* @brief      CMainDlg 富文本与动态创建窗口演示
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    "基础控件"页富文本相关演示,以及"杂项"页中窗口创建类演示:
*               - RichEdit 插入 GIF 表情(依赖 smiley OLE 组件,仅 Windows)
*               - 格式化消息追加 / RTF 打开与保存
*               - 自定义富文本光标(EventGetCaret)
*               - 从文件创建窗口(SHostDialog "file:" 协议)
*               - CreateChildrenFromXml / 模板创建子窗口
*/

#include "stdafx.h"
#include "MainDlg.h"
#include <controls.extend/FileHelper.h> //CFileDialogEx 文件对话框
#include "FormatMsgDlg.h"
#include <controls.extend/SChatEdit.h>
#include <controls.extend/SScintillaView.h>
#include <controls.extend/reole/RichEditOle.h>
#include "SmileyCreateHook.h"

#ifdef _WIN32
/**
* @brief      GIF 表情数据源:按表情 ID 映射到 demo 源码目录下的 gif/N.gif
*
* Describe    CSmileySource 负责表情图片的加载,ImageID2Path 决定 ID 到文件
*             的映射规则,这里指向源码树的 gif 目录便于离线演示。
*/
class CSmileySource2 : public CSmileySource
{
public:
    CSmileySource2(){}

protected:
    //获对ID对应的图片路径
    virtual SStringW ImageID2Path(UINT nID)
    {
        SStringA filePath = __FILE__;
        filePath = filePath.Left(filePath.ReverseFind('\\'));
        filePath += SStringA().Format("/gif/%d.gif", nID);
        return S_CA2W(filePath);
    }
};

//Richedit中插入表情使用的回调函数。
ISmileySource * CreateSource2()
{
    return  new CSmileySource2;
}
#endif

/**
* @brief      "insert gif"按钮:向富文本插入 GIF 表情(仅 Windows)
*
* Describe    完整演示表情 OLE 组件的插入流程:
*               1. LoadFromFile 加载表情数据源(ISmileySource);
*               2. CoCreateInstance 创建 ISmileyCtrl COM 对象;
*               3. EM_GETOLEINTERFACE 取得 IRichEditOle 接口;
*               4. Insert2Richedit 插入。
*             若系统未注册 sosmiley.dll,提示用户现场注册。
*/
void CMainDlg::OnBtnInsertGif2RE()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFileDialogEx openDlg(TRUE,_T("gif"),0,6,_T("gif files(*.gif)\0*.gif\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
        {
#if defined(_WIN32) && !defined(_ARM64_) && !defined(_ARM_) && !defined(__MINGW32__)
            ISmileySource *pSource = new CSmileySource2;
            HRESULT hr=pSource->LoadFromFile(S_CT2W(openDlg.m_szFileName));
            if(SUCCEEDED(hr))
            {
                SComPtr<ISmileyCtrl> pSmiley;
                hr=::CoCreateInstance(CLSID_SSmileyCtrl,NULL,CLSCTX_INPROC,__uuidof(ISmileyCtrl),(LPVOID*)&pSmiley);
                if(SUCCEEDED(hr))
                {
                    pSmiley->SetSource(pSource);
                    SComPtr<IRichEditOle> ole;
                    pEdit->SSendMessage(EM_GETOLEINTERFACE,0,(LPARAM)&ole);
                    pSmiley->Insert2Richedit(ole);
                }else
                {
                    //表情 COM 未注册时,尝试现场注册 sosmiley.dll
                    UINT uRet = SMessageBox(m_hWnd,_T("可能是因为没有向系统注册表情COM模块。\n现在注册吗?"),_T("创建表情OLE对象失败"),MB_YESNO|MB_ICONSTOP);
                    if(uRet == IDYES)
                    {
                        HMODULE hMod = LoadLibrary(_T("sosmiley.dll"));
                        if(hMod)
                        {
                            typedef HRESULT (STDAPICALLTYPE *DllRegisterServerPtr)();
                            DllRegisterServerPtr funRegDll = (DllRegisterServerPtr)GetProcAddress(hMod,"DllRegisterServer");
                            if(funRegDll)
                            {
                                HRESULT hr=funRegDll();
                                if(FAILED(hr))
                                {
                                    SMessageBox(m_hWnd,_T("请使用管理员权限运行模块注册程序"),_T("注册表情COM失败"),MB_OK|MB_ICONSTOP);
                                }else
                                {
                                    SMessageBox(m_hWnd,_T("请重试"),_T("注册成功"),MB_OK|MB_ICONINFORMATION);
                                }
                            }
                            FreeLibrary(hMod);
                        }else
                        {
                            SMessageBox(m_hWnd,_T("没有找到表情COM模块[sosmiley.dll]。\n现在注册吗"),_T("错误"),MB_OK|MB_ICONSTOP);
                        }
                    }
                }
            }else
            {
                SMessageBox(m_hWnd,_T("加载表情失败"),_T("错误"),MB_OK|MB_ICONSTOP);
            }
            pSource->Release();
#endif
        }
    }
}

/**
* @brief      "append format msg"按钮:弹出参数对话框并向富文本追加格式化消息
*
* Describe    演示模式对话框(SHostDialog 派生的 CFormatMsgDlg)与
*             SChatEdit::AppendFormatText 的配合。
*/
void CMainDlg::OnBtnAppendMsg()
{
    SChatEdit *pEdit = FindChildByName2<SChatEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFormatMsgDlg formatMsgDlg;
        if(formatMsgDlg.DoModal()==IDOK)
        {
            for(int i=0;i<formatMsgDlg.m_nRepeat;i++)
                pEdit->AppendFormatText(S_CT2W(SStringT().Format(_T("line:%d "),i) + formatMsgDlg.m_strMsg));
        }
    }
}

/**
* @brief      "save to rtf"按钮:把富文本内容保存为 RTF 文件
*/
void CMainDlg::OnBtnRtfSave()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFileDialogEx openDlg(FALSE,_T("rtf"),_T("soui_richedit"),6,_T("rtf files(*.rtf)\0*.rtf\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
        {
            pEdit->SaveRtf(openDlg.m_szFileName);
        }
    }
}

/**
* @brief      "open from rtf"按钮:从 RTF 文件加载富文本内容
*/
void CMainDlg::OnBtnRtfOpen()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        CFileDialogEx openDlg(TRUE,_T("rtf"),0,6,_T("rtf files(*.rtf)\0*.rtf\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
        {
            pEdit->LoadRtf(openDlg.m_szFileName);
        }
    }
}

/**
* @brief      富文本光标定制事件
*
* Describe    EventGetCaret 允许应用在创建光标前提供自定义的 <caret> XML,
*             这里返回一个蓝色、带淡入淡出动画的光标描述。
*             (全局默认光标在 init.xml 的 <caret> 中定义。)
*/
void CMainDlg::OnGetCaret(IEvtArgs* e)
{
	EventGetCaret *e2=sobj_cast<EventGetCaret>(e);
	e2->strCaret->Assign(L"<caret color=\"rgb(0,0,255)\" animate=\"true\" fadeTime=\"20\" showTime=\"10\" interpolator=\"Accelerate\"/>");
}

/**
* @brief      "从文件创建窗口"按钮
*
* Describe    演示 SHostDialog 的 "file:" 协议:直接从磁盘加载布局文件,
*             不经过资源提供器。由于该布局使用相对路径引用图片,
*             需先把当前目录切换到资源所在位置。
*/
void CMainDlg::OnBtnFileWnd()
{
    //由于资源中使用了相对路径，需要将当前路径指定到资源所在位置
    SStringT strCurDir = SApplication::getSingleton().GetAppDir();
	#ifdef _WIN32
    strCurDir += _T("\\filewnd");
	#else
    strCurDir += _T("/filewnd");
	#endif //_WIN32
    SetCurrentDirectory(strCurDir);
    if(GetFileAttributes(_T("test.xml"))==INVALID_FILE_ATTRIBUTES)
    {
        SMessageBox(m_hWnd,_T("没有找到资源文件！"),_T("错误"),MB_OK|MB_ICONSTOP);
        return ;
    }
    SHostDialog fileDlg(_T("file:test.xml"));
    fileDlg.DoModal(m_hWnd);
}

/**
* @brief      "CreateChildren"按钮:把输入框中的 XML 字符串实例化为子窗口
*
* Describe    演示 SWindow::CreateChildrenFromXml 的动态创建能力:
*             先清空容器原有子窗口(注意边遍历边销毁要预先取 next 兄弟),
*             再把 XML 字符串解析为控件树挂到容器上。
*/
void CMainDlg::OnBtnCreateChildren()
{
    SScintillaView *pEdit = FindChildByID2<SScintillaView>(R.id.edit_xml);
    SASSERT(pEdit);
    SStringW strXml = S_CA2W(pEdit->GetEditorText(), CP_UTF8);
    SWindow *pContainer = FindChildByID(R.id.wnd_container);
    SASSERT(pContainer);
    //remove all children at first.
    SWindow *pChild = pContainer->GetWindow(GSW_FIRSTCHILD);
    while(pChild)
    {
        SWindow *pNext = pChild->GetWindow(GSW_NEXTSIBLING);
        pChild->Destroy();
        pChild = pNext;
    }
    //using SWindow::CreateChildren to Create Children described in the input xml string.
    pContainer->CreateChildrenFromXml(strXml);
}

/**
* @brief      模板创建子窗口:把输入的 XML 片段挂到 wnd_temp_host 下
*
* Describe    与 OnBtnCreateChildren 的区别:输入可以是 <include> 引用
*             或任何合法的 XML 片段,便于演示布局复用。
*/
void CMainDlg::OnBtnCreateByTemp()
{
	SWindow *pContainer = FindChildByName(L"wnd_temp_host");
	SWindow *pInput = FindChildByName(L"re_temp_input");
	if(pContainer && pInput)
	{
		SStringT strInput = pInput->GetWindowText();
		pContainer->CreateChildrenFromXml(S_CT2W(strInput));
	}
}

/**
* @brief      wrap_content 布局演示窗口
*
* Describe    打开一个以内容自适应(viewSize 根据子窗口计算)为特性的演示对话框。
*/
void CMainDlg::OnBtnOpenWrapContent()
{
	SHostDialog dlgWrapContent(_T("layout:dlg_wrap_content"));
	dlgWrapContent.DoModal(m_hWnd);
}
