#pragma once
#include <control/SEdit.h>
#include <event/SNotifyCenter.h>
#include <helper/SFunctor.hpp>
#include <interface/STaskLoop-i.h>
#include <commgr2.h>
#include <vector>
#include "events.h"
#include "fund_eastmoney.h"

using namespace SOUI;

// 后台抓取完成后，由 SNotifyCenter 回投到 UI 线程的事件
// list / err 按值存放，事件对象本身在堆上分配，随事件释放一并析构
DEF_EVT_EXT(EventQueryResult, EVT_QUERY_RESULT, {
    BOOL bOk;
    std::vector<FundHolding> list;
    std::string err;
})

class CMainDlg : public SHostWnd, public TAutoEventMapReg<CMainDlg> {
public:
    CMainDlg();
    ~CMainDlg();

    int OnCreate(LPCREATESTRUCT lpCreateStruct);
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    void OnClose();
    void OnMaximize();
    void OnRestore();
    void OnMinimize();
    void OnSize(UINT nType, CSize size);

protected:
    void OnQuery();
    // 在 UI 线程处理后台回投的结果事件
    void OnQueryResult(EventArgs *e);
    // 在后台 TaskLoop 工作线程中执行：网络抓取 + 解析
    void doFetch(const std::string &code, const std::string &year);

    void FillGrid(const std::vector<HoldingChange> &list);
    void SetStatus(const SStringT &s);
    void SetBusy(BOOL bBusy);

    EVENT_MAP_BEGIN()
        EVENT_HANDLER(EventQueryResult::EventID, OnQueryResult)
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_NAME_COMMAND(L"btn_query", OnQuery)
    EVENT_MAP_END2(SHostWnd)

    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    SComMgr2 m_comMgr;                   // 用于创建后台 TaskLoop 与 IHttpClient
    SAutoRefPtr<ITaskLoop> m_taskLoop;   // 后台工作线程（抓取数据）
    BOOL m_bBusy = FALSE;
};
