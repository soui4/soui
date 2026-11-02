// wasm 版 ws 组件的入口：只提供客户端，服务端在浏览器里没有意义。

#include "WsClientWasm.h"
#include <interface/ws-i.h>

SNSBEGIN

class CWasmWebsocket : public TObjRefImpl<IWebsocket>
{
  public:
    STDMETHOD_(IWsClient *, CreateWsClient)(THIS_ IConnListener *pListener) OVERRIDE
    {
        return new CWasmWsClient(pListener);
    }

    STDMETHOD_(IWsServer *, CreateWsServer)(THIS_ ISvrListener *) OVERRIDE
    {
        // 浏览器页面不能监听端口；调用方拿到 NULL 应当直接判失败
        return NULL;
    }

    STDMETHOD_(ITimerGenerator *, CreateTimerGenerator)(CTHIS) OVERRIDE
    {
        // 桌面版的定时器生成器靠一条独立定时线程；浏览器里没有线程可给。
        // 需要周期任务时用 ITaskLoop 的 setHeartBeatTask（wasm 版由主循环驱动）。
        return NULL;
    }

    STDMETHOD_(void, SetLogCallback)(THIS_ WsLogCallback cb) OVERRIDE
    {
        WsWasmSetLogCallback(cb);
    }
};

namespace WS
{
SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppRet)
{
    *ppRet = new CWasmWebsocket();
    return TRUE;
}
} // namespace WS

SNSEND

SOUI_COM_C BOOL SOUI_COM_API Ws_CreateInstance(IObjRef **ppRet)
{
    return SOUI::WS::SCreateInstance(ppRet);
}
