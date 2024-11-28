#include <souistd.h>
#include <SouiFactory.h>
#include <commgr2.h>
#include "../components/resprovider-zip/zipresprovider-param.h"
#include "common.h"
#include <sstream>
#define kLogTag "ipc_svr"

using namespace SOUI;

class CMainDlg : public SHostWnd {
public:
    CMainDlg(LPCSTR pszLayout) :SHostWnd(pszLayout) {}

    void OnClose();
    BOOL OnInitDialog(HWND hWnd,LPARAM lp);
    LRESULT onIpc(UINT msg,WPARAM wp,LPARAM lp);
    LRESULT onIpc2(UINT msg, WPARAM wp, LPARAM lp);
    BOOL OnCopyData(HWND wnd, PCOPYDATASTRUCT pCopyDataStruct);

    EVENT_MAP_BEGIN()
        EVENT_ID_COMMAND(1,OnClose)
    EVENT_MAP_END2(SHostWnd)
    BEGIN_MSG_MAP_EX(CMainDlg)
        MESSAGE_HANDLER_EX(UM_IPC,onIpc)
        MESSAGE_HANDLER_EX(UM_IPC2, onIpc2)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_SIZE(OnSize)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_COPYDATA(OnCopyData)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()
};

void CMainDlg::OnClose() {
    PostMessage(WM_QUIT);
}

BOOL CMainDlg::OnInitDialog(HWND hWnd,LPARAM lp){
    SWindow *pTxtId = FindChildByName("txt_svr_id");
    pTxtId->SetWindowText(SStringT().Format(_T("%ld"),m_hWnd));
    return TRUE;
}

LRESULT CMainDlg::onIpc(UINT msg,WPARAM wp,LPARAM lp){
    SLOGI()<<"onIpc,wp="<<wp<<" lp="<<lp;
    SWindow *pTxtRecv = FindChildByName("txt_recv");
    pTxtRecv->SetWindowText(SStringT().Format(_T("receive UM_IPC wp=%d,lp=%d"),(int)wp,(int)lp));
    return 0;
}

LRESULT CMainDlg::onIpc2(UINT msg, WPARAM wp, LPARAM lp)
{
    return wp+lp;
}

BOOL CMainDlg::OnCopyData(HWND wnd, PCOPYDATASTRUCT pCopyDataStruct)
{
    SLOGI() << "OnCopyData,dwData" << pCopyDataStruct->dwData;
    std::stringstream ss;
    ss << "recv copydata, from " << wnd << " dwData=" << pCopyDataStruct->dwData << " content:" << std::string((char*)pCopyDataStruct->lpData, pCopyDataStruct->cbData);
    SWindow* pTxtRecv = FindChildByName("txt_recv");
    pTxtRecv->SetWindowTextU8(ss.str().c_str());
    return TRUE;
}

int run_app(HINSTANCE hInst){
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
    SAutoRefPtr<IResProvider> sysResouce(sfac.CreateResProvider(RES_FILE), FALSE);
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
			pLogMgr->setLoggerName("ipc_svr");

			//uncomment next line to record info level log.
			pLogMgr->setLoggerLevel(LOG_LEVEL_INFO);
			pLogMgr->start();
		}
        app.SetLogManager(pLogMgr);
    }
    SLOGI()<<"start soui app";
    CMainDlg hostWnd("layout:xml_ipc_svr");
    hostWnd.CreateEx(0,WS_POPUP|WS_VISIBLE,WS_EX_LAYERED,300,100,0,0);
    hostWnd.SendMessage(WM_INITDIALOG);
    int ret = app.Run(hostWnd.m_hWnd);
    SLOGI()<<"soui app end, exit code="<<ret;
    
    return ret;
}

int main(int argc,char **argv){
    //HANDLE h = CreateEventA(NULL, FALSE, FALSE, "test evt");
    //int ret = WaitForSingleObject(h, 50*1000);
    //printf("wait ret=%d\n", ret);
    //CloseHandle(h);
    //return 0;
    return run_app(0);
}