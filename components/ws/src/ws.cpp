#include "wsClient.h"
#include "wsServer.h"
#include <interface/ws-i.h>
#include <helper/slog.h>
#define kLogTag "Ws"

SNSBEGIN

static WsLogCallback g_logCallback = NULL;

static void lws_log_emit(int level, const char *line)
{
    int soui_level = LOG_LEVEL_INFO;
    switch (level)
    {
    case LLL_ERR:
        soui_level = LOG_LEVEL_ERROR;
        break;
    case LLL_WARN:
        soui_level = LOG_LEVEL_WARN;
        break;
    case LLL_DEBUG:
        soui_level = LOG_LEVEL_DEBUG;
        break;
    case LLL_INFO:
    case LLL_NOTICE:
    default:
        soui_level = LOG_LEVEL_INFO;
        break;
    }
    if(g_logCallback){
        g_logCallback(soui_level,line);
    }
}

class Websocket : public TObjRefImpl<IWebsocket> {
  public:
    Websocket()
    {
        lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG, lws_log_emit);
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