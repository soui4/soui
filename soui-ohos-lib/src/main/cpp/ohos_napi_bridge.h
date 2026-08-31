#ifndef _OHOS_NAPI_BRIDGE_PUBLIC_H_
#define _OHOS_NAPI_BRIDGE_PUBLIC_H_

#include <ctypes.h>

typedef struct _cairo_surface cairo_surface_t;

namespace swinx {
namespace ohos {

struct XComponentState {
    void *component;
    void *nativeWindow;
    int width;
    int height;
    float density;
    float lastTouchX;
    float lastTouchY;
    bool touching;
};

void SetXComponentState(const XComponentState &state);
XComponentState GetXComponentState();
void RequestFrame(HWND hWnd, const RECT *dirty);
bool PresentCairoSurface(cairo_surface_t *surface, const RECT *dirty);
bool DrawDemoFrame();
void NotifyMainWindowMoveDelta(int dx, int dy);
void NotifyMainWindowRect(int x, int y, int width, int height);
void NotifyCursorId(int cursorId);

} // namespace ohos
} // namespace swinx

#endif // _OHOS_NAPI_BRIDGE_PUBLIC_H_
