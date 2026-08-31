// OhosBridge.cpp
// swinx::ohos 桥接实现：供 SOUI 核心（SOhosApp.cpp）与 N-API 层共享的进程级状态。
// - XComponentState：XComponent 的窗口/尺寸/触点等全局状态，NAPI 回调与 UI 线程并发访问，用互斥锁保护。
// - ImeProxyActive：IME 代理激活标志。
#include "ohos_ime_bridge.h"
#include "ohos_napi_bridge.h"

#include <mutex>

namespace swinx {
namespace ohos {

namespace {

std::mutex g_stateMutex;
XComponentState g_xcompState = {};
bool g_imeProxyActive = false;

} // namespace

void SetImeProxyActive(bool active)
{
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_imeProxyActive = active;
}

bool IsImeProxyActive()
{
    std::lock_guard<std::mutex> lock(g_stateMutex);
    return g_imeProxyActive;
}

void SetXComponentState(const XComponentState &state)
{
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_xcompState = state;
}

XComponentState GetXComponentState()
{
    std::lock_guard<std::mutex> lock(g_stateMutex);
    return g_xcompState;
}

} // namespace ohos
} // namespace swinx
