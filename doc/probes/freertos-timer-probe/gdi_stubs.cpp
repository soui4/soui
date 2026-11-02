/* gdi_stubs.cpp -- probe-only stand-ins for the two font metrics entry points
 * that src/gdi/cairo/fontapi.cpp would provide. That file cannot be compiled
 * for the FreeRTOS port: it needs std::once_flag/std::call_once, which
 * src/platform/freertos/stl/mutex does not implement. Never reached by the
 * timer probe (no text metrics are queried), so returning 0 is enough. */
extern "C"
{
    long GetGlyphIndicesW(void) { return 0; }
    long GetCharWidthI(void) { return 0; }
}
