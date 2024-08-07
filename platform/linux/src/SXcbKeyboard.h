#ifndef _SXCB_KEYBOARD_H_
#define _SXCB_KEYBOARD_H_

#include <platform.h>
#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <X11/keysym.h>


struct _xkb_mods {
    xkb_mod_index_t shift;
    xkb_mod_index_t lock;
    xkb_mod_index_t control;
    xkb_mod_index_t mod1;
    xkb_mod_index_t mod2;
    xkb_mod_index_t mod3;
    xkb_mod_index_t mod4;
    xkb_mod_index_t mod5;
};

struct _mod_masks {
    uint alt;
    uint altgr;
    uint meta;
    uint super;
    uint hyper;
};

class SConnection;
class SXcbKeyboard {
public:
    SXcbKeyboard(SConnection* conn);

    ~SXcbKeyboard();

    SHORT getKeyState(uint8_t vk) {
        return m_byKeyboardState[vk] << 8;
    }

    void readXKBConfig();

    void clearXKBConfig();
    void updateKeymap();

    void checkForLatinLayout();


    void updateXKBMods();


    uint32_t xkbModMask(uint16_t state);

    void updateXKBStateFromState(struct xkb_state* kb_state, uint16_t state);

    uint32_t translateModifiers(int s) const;


    int lookVkMap(xcb_keysym_t key) const;

    int scanCodeToVk(xcb_keysym_t keysym, UINT  modifiers) const;

    uint32_t onKeyEvent(bool bPress, xcb_keycode_t code, uint16_t state, xcb_timestamp_t ts);

protected:
    xcb_connection_t* xcb_connection();
    void updateModifiers();
    void resolveMaskConflicts();
private:
    SConnection* m_conn;

    xcb_keycode_t m_autorepeat_code;

    struct xkb_context* xkb_context;
    struct xkb_keymap* xkb_keymap;
    struct xkb_state* xkb_state;
    struct xkb_rule_names xkb_names;
    mutable struct xkb_keymap* latin_keymap;
    bool m_hasLatinLayout;
    xcb_key_symbols_t* m_key_symbols;

    _xkb_mods xkb_mods;
    _mod_masks rmod_masks;

    _mod_masks vmod_masks;
    int core_device_id;

    bool m_config;

    BYTE m_byKeyboardState[256];
};

#endif//_SXCB_KEYBOARD_H_