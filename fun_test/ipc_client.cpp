#include <souistd.h>
#include <SouiFactory.h>
#include <commgr2.h>
#include "../components/resprovider-zip/zipresprovider-param.h"
#include "common.h"

#define kLogTag "ipc_client"

using namespace SOUI;

class CMainDlg : public SHostWnd {
    HWND m_svrId;
public:
    CMainDlg(LPCSTR pszLayout) :SHostWnd(pszLayout),m_svrId(0) {}

    void OnClose();
    BOOL OnInitDialog(HWND hWnd,LPARAM lp);
    void OnBtnSendMsg();
    void OnBtnPostMsg();
    void OnBtnSendCopyData();
    void OnBtnSendMsgCallback();

    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND(1,OnClose)
        EVENT_NAME_COMMAND(L"btn_send_msg",OnBtnSendMsg)
        EVENT_NAME_COMMAND(L"btn_post_msg",OnBtnPostMsg)
        EVENT_NAME_COMMAND(L"btn_send_copydata", OnBtnSendCopyData)
        EVENT_NAME_COMMAND(L"btn_send_msgcallback",OnBtnSendMsgCallback)
    EVENT_MAP_END2(SHostWnd)
    BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_SIZE(OnSize)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()
};

void CMainDlg::OnClose() {
    PostMessage(WM_QUIT);
}

BOOL CMainDlg::OnInitDialog(HWND hWnd,LPARAM lp){
    SWindow *pTxtId = FindChildByName("txt_client_id");
    pTxtId->SetWindowText(SStringT().Format(_T("%ld"),m_hWnd));
    m_svrId = (HWND)lp;
    {
        SWindow *pTxtId = FindChildByName("txt_svr_id");
        pTxtId->SetWindowText(SStringT().Format(_T("%ld"),m_svrId));
    }
    return TRUE;
}

void CMainDlg::OnBtnSendMsg(){
    if(::IsWindow(m_svrId)){
        int ret = ::SendMessage(m_svrId,UM_IPC,100,200);
        SLOGW()<<"send UM_IPC ret="<<ret;
    }else{
        SLOGW()<<"svr id is not valid window, svr id="<<m_svrId;
    }
}

void CMainDlg::OnBtnPostMsg(){
    if(::IsWindow(m_svrId)){
        int ret = ::PostMessage(m_svrId,UM_IPC,300,400);
        SLOGW()<<"post UM_IPC ret="<<ret;
    }else{
        SLOGW()<<"svr id is not valid window, svr id="<<m_svrId;
    }
}

void CMainDlg::OnBtnSendCopyData()
{
    if (::IsWindow(m_svrId)) {
        std::string strBuf = "soui on linux";
        COPYDATASTRUCT cbs;
        cbs.dwData = 100;
        cbs.cbData = strBuf.length();
        cbs.lpData = (PVOID)strBuf.c_str();

        int ret = ::PostMessage(m_svrId, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cbs);
        SLOGW() << "post UM_IPC ret=" << ret;
    }
    else {
        SLOGW() << "svr id is not valid window, svr id=" << m_svrId;
    }
}

static VOID CALLBACK CbTest(HWND hWnd, UINT msg, ULONG_PTR dwData, LRESULT res) {
    CMainDlg* _this = (CMainDlg*)dwData;
    SLOGI() << "cbtest, res=" << res;
    SWindow* pTxt = _this->FindChildByName("txt_svr_ret");
    pTxt->SetWindowText(SStringT().Format(_T("cbtest ret=%d"), res));
}

void CMainDlg::OnBtnSendMsgCallback()
{
    ::SendMessageCallback(m_svrId, UM_IPC2, 100, 200, CbTest, (ULONG_PTR)this);
}

int run_app(HINSTANCE hInst,HWND hSvrId){
    std::tstring srcDir = getSourceDir();

    SComMgr2 comMgr;
    SAutoRefPtr<IRenderFactory> renderFac;
    BOOL useGdi=1;
    if(useGdi)
        comMgr.CreateRender_GDI((IObjRef**)&renderFac);
    else
        comMgr.CreateRender_Skia((IObjRef**)&renderFac);
    SAutoRefPtr<IImgDecoderFactory> imgDecoder;
    comMgr.CreateImgDecoder((IObjRef**)&imgDecoder);
    renderFac->SetImgDecoderFactory(imgDecoder);
    
    SApplication app(renderFac,hInst);
    SouiFactory sfac;
    SAutoRefPtr<IResProvider> sysResouce(sfac.CreateResProvider(RES_FILE),FALSE);
    std::tstring sysRes = srcDir + kPath_SysRes;
    sysResouce->Init((LPARAM)sysRes.c_str(), 0);
    app.LoadSystemNamedResource(sysResouce);
    BOOL bLoadFileRes=TRUE;
    if(bLoadFileRes){
        SAutoRefPtr<IResProvider> testResouce(sfac.CreateResProvider(RES_FILE), FALSE);
        std::tstring appRes = srcDir + kPath_TestRes;
        testResouce->Init((LPARAM)appRes.c_str(), 0);
        app.AddResProvider(testResouce);
    }else
    {
        SAutoRefPtr<IResProvider> testResouce;
        BOOL bLoad=comMgr.CreateResProvider_ZIP((IObjRef**)&testResouce);
        SASSERT(bLoad);
        ZIPRES_PARAM param;
        std::tstring strZip = srcDir + kPath_ZipRes;
        ZipFile(&param, renderFac, strZip.c_str(), kZip_Psw);
        BOOL initDone = testResouce->Init((WPARAM)&param,0);
        app.AddResProvider(testResouce);
    }
    if(1){//enable log
        SAutoRefPtr<ILogMgr> pLogMgr;
        if (comMgr.CreateLog4z((IObjRef**)&pLogMgr) && pLogMgr)
		{
			//uncomment next line to disable log mgr to output debug string.
			pLogMgr->setLoggerName(kLogTag);

			//uncomment next line to record info level log.
			pLogMgr->setLoggerLevel(LOG_LEVEL_INFO);
			pLogMgr->start();
		}
        app.SetLogManager(pLogMgr);
    }
    SLOGI()<<"start soui app";
    CMainDlg hostWnd("layout:xml_ipc_client");
    hostWnd.CreateEx(0,WS_POPUP|WS_VISIBLE,WS_EX_LAYERED,300,100,0,0);
    hostWnd.SendMessage(WM_INITDIALOG,0,(LPARAM)hSvrId);
    int ret = app.Run(hostWnd.m_hWnd);
    SLOGI()<<"soui app end, exit code="<<ret;
    
    return ret;
}

int main(int argc,char **argv){
    //HANDLE h = CreateEventA(NULL, FALSE, FALSE, "test evt");
    //int ret = SetEvent(h);
    //printf("wait ret=%d\n", ret);
    //CloseHandle(h);
    //return 0;

    if(argc<2)
    {
        printf("usage: ipc_client {svrid}\n");
        return 0;
    }
    HWND hSvrId = (HWND)atol(argv[1]);
    return run_app(0,hSvrId);
}