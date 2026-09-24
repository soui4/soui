/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       MainDlg_Webkit.cpp
* @brief      CMainDlg 内嵌浏览器(教程页)演示
* @version    v2.0
* @author     SOUI group
* @date       2014/08/15
*
* Describe    "教程"页(page_webkit.xml)的功能实现:
*               - SWkeWebkit 控件:基于 WKE 的网页内核,加载/前进/后退/刷新;
*               - SChromeTab 风格页签的新建事件;
*               - Edit 的 EN_CHANGE 事件响应(地址栏)。
*             WKE 依赖 wke.dll,demo 启动时由 SWkeLoader 初始化(demo.cpp)。
*/

#include "stdafx.h"
#include "MainDlg.h"

#define kLogTag "webkitdlg"

/**
* @brief      "Go"按钮:加载地址栏输入的 URL
*
* Describe    演示运行时用 SetAttribute 修改控件属性:url 属性带 FALSE 参数
*             表示立即生效(不等待布局刷新)。
*/
void CMainDlg::OnBtnWebkitGo()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        SEdit *pEdit=FindChildByName2<SEdit>(L"edit_url");
        SStringT strUrl=pEdit->GetWindowText();
        pWebkit->SetAttribute(L"url",S_CT2W(strUrl),FALSE);
    }
}

/**
* @brief      "后退"按钮:浏览器历史回退
*/
void CMainDlg::OnBtnWebkitBackward()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->goBack();
    }
}

/**
* @brief      "前进"按钮:浏览器历史前进
*/
void CMainDlg::OnBtnWebkitForeward()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->goForward();
    }
}

/**
* @brief      "刷新"按钮:重新加载当前页面
*/
void CMainDlg::OnBtnWebkitRefresh()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->reload();
    }
}

/**
* @brief      Chrome 风格页签控件的"新建页签"事件
*
* Describe    EVT_CHROMETAB_NEW 由 chromeTab 控件在用户点击"+"时发出,
*             事件对象中带 pNewTab 指向新创建的页签窗口,这里为它设置标题与提示。
*/
void CMainDlg::OnChromeTabNew( IEvtArgs *pEvt )
{
    static int iPage = 0;
    EventChromeTabNew *pEvtTabNew = (EventChromeTabNew*)pEvt;

    SStringT strTitle = SStringT().Format(_T("新建窗口 %d"),++iPage);
    pEvtTabNew->pNewTab->SetWindowText(strTitle);
    pEvtTabNew->pNewTab->SetAttribute(L"tip",S_CT2W(strTitle));
}

/**
* @brief      地址栏(SRichEdit/SEdit)的 RE 通知事件
*
* Describe    演示 EVT_RE_NOTIFY 事件:iNotify 中为标准的 EN_* 通知码,
*             此处仅记录 EN_CHANGE 日志,作为 Edit 内容变化响应的示例。
*/
void CMainDlg::OnUrlReNotify(IEvtArgs *pEvt)
{
    EventRENotify *pEvt2 = sobj_cast<EventRENotify>(pEvt);
    SLOGFMTD(_T("OnUrlReNotify,iNotify = %d"),pEvt2->iNotify);
    if(pEvt2->iNotify == EN_CHANGE)
    {
        SLOGFMTD(_T("OnUrlReNotify,iNotify = EN_CHANGE"));
    }
}
