#include <interface/ws-i.h>
#include "wsClient.h"
#include "wsServer.h"
#include <helper/slog.h>


SNSBEGIN

static WsLogCallback g_logCallback = NULL;

static void lws_log_emit(int level, const char *line)
{
    int sLevel = LOG_LEVEL_DEBUG;
    switch (level)
    {
        case LLL_ERR:
            sLevel = LOG_LEVEL_ERROR;
            break;
        case LLL_WARN:
            sLevel = LOG_LEVEL_WARN;
            break;
        case LLL_NOTICE:
            sLevel = LOG_LEVEL_INFO;
            break;
    }
    if(g_logCallback){
        g_logCallback(sLevel,line);
    }
}

class Websocket : public TObjRefImpl<IWebsocket> {
  public:
    Websocket()
    {
        lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE, lws_log_emit);
    }
    // IWebsocket methods
    STDMETHODIMP_(IWsClient *) CreateWsClient(THIS_ IConnListener *pListener) OVERRIDE
    {
        return new WsClient(pListener);
    }
    STDMETHODIMP_(IWsServer *) CreateWsServer(THIS_ ISvrListener *pListener) OVERRIDE
    {
        return new WsServer(pListener);
    }
    STDMETHODIMP_(void) SetLogCallback(THIS_ WsLogCallback cb) OVERRIDE
    {
        g_logCallback = cb;
    }
};

#undef SOUI_COM_API
#ifdef _WIN32
#define SOUI_COM_API __declspec(dllexport)
#else
#define SOUI_COM_API
#endif
#undef SOUI_COM_C
#define SOUI_COM_C EXTERN_C

namespace WS
{
SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppRet)
{
    *ppRet = new Websocket();
    return TRUE;
}
}

SNSEND

SOUI_COM_C BOOL SOUI_COM_API Ws_CreateInstance(IObjRef **ppRet)
{
    return SOUI::WS::SCreateInstance(ppRet);
}