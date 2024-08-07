#include "SConnection.h"
#include <helper/SCriticalSection.h>
#include <assert.h>
#include <functional>
#include <xcb/xcb_icccm.h>
#include <xcb/render.h>
#include <xcb/xcb_renderutil.h>
#include <xcb/xcb_image.h>

#include "uimsg.h"
#include <algorithm>
#include "cursors.hpp"

#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <X11/keysym.h>

#include <debug.h>

#define qWarning WARN
#define qWarning0 WARN0

SNSBEGIN

static SConnMgr *s_connMgr = NULL;
static SCriticalSection s_cs;

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

struct KeyMap {
    uint32_t scanCode;
    uint32_t vk;
};
// keyboard mapping table
static const KeyMap KeyTbl[] = {

    // misc keys

    XK_Escape,                  VK_ESCAPE,
    XK_Tab,                     VK_TAB,
    //XK_ISO_Left_Tab,            Qt::Key_Backtab,
    XK_BackSpace,               VK_BACK,
    XK_Return,                  VK_RETURN,
    XK_Insert,                  VK_INSERT,
    XK_Delete,                  VK_DELETE,
    XK_Clear,                   VK_DELETE,
    XK_Pause,                   VK_PAUSE,
    XK_Print,                   VK_PRINT,
//    0x1005FF60,                 Qt::Key_SysReq,         // hardcoded Sun SysReq
//    0x1007ff00,                 Qt::Key_SysReq,         // hardcoded X386 SysReq

    // cursor movement

    XK_Home,                    VK_HOME,
    XK_End,                     VK_END,
    XK_Left,                    VK_LEFT,
    XK_Up,                      VK_UP,
    XK_Right,                   VK_RIGHT,
    XK_Down,                    VK_DOWN,
    XK_Prior,                   VK_PRIOR,
    XK_Next,                    VK_NEXT,

    // modifiers

    XK_Shift_L,                 VK_SHIFT,
    XK_Shift_R,                 VK_SHIFT,
    XK_Shift_Lock,              VK_SHIFT,
    XK_Control_L,               VK_CONTROL,
    XK_Control_R,               VK_CONTROL,
//    XK_Meta_L,                  Qt::Key_Meta,
//    XK_Meta_R,                  Qt::Key_Meta,
    XK_Alt_L,                   VK_MENU,
    XK_Alt_R,                   VK_MENU,
    XK_Caps_Lock,               VK_CAPITAL,
    XK_Num_Lock,                VK_NUMLOCK,
    XK_Scroll_Lock,             VK_SCROLL,
//    XK_Super_L,                 Qt::Key_Super_L,
//    XK_Super_R,                 Qt::Key_Super_R,
    XK_Menu,                    VK_MENU,
//    XK_Hyper_L,                 Qt::Key_Hyper_L,
//    XK_Hyper_R,                 Qt::Key_Hyper_R,
    XK_Help,                    VK_HELP,
//    0x1000FF74,                 Qt::Key_Backtab,        // hardcoded HP backtab
    0x1005FF10,                 VK_F11,            // hardcoded Sun F36 (labeled F11)
    0x1005FF11,                 VK_F12,            // hardcoded Sun F37 (labeled F12)

    // numeric and function keypad keys

    XK_KP_Space,                VK_SPACE,
    XK_KP_Tab,                  VK_TAB,
    XK_KP_Enter,                VK_RETURN,
    //XK_KP_F1,                 Qt::Key_F1,
    //XK_KP_F2,                 Qt::Key_F2,
    //XK_KP_F3,                 Qt::Key_F3,
    //XK_KP_F4,                 Qt::Key_F4,
    XK_KP_Home,                 VK_HOME,
    XK_KP_Left,                 VK_LEFT,
    XK_KP_Up,                   VK_UP,
    XK_KP_Right,                VK_RIGHT,
    XK_KP_Down,                 VK_DOWN,
    XK_KP_Prior,                VK_PRIOR,
    XK_KP_Next,                 VK_NEXT,
    XK_KP_End,                  VK_END,
    XK_KP_Begin,                VK_CLEAR,
    XK_KP_Insert,               VK_INSERT,
    XK_KP_Delete,               VK_DELETE,
    XK_KP_Equal,                VK_OEM_NEC_EQUAL,
    XK_KP_Multiply,             VK_MULTIPLY,
    XK_KP_Add,                  VK_ADD,
    XK_KP_Separator,            VK_SEPARATOR,
    XK_KP_Subtract,             VK_SUBTRACT,
    XK_KP_Decimal,              VK_DECIMAL,
    XK_KP_Divide,               VK_DIVIDE,

    // International input method support keys

    // International & multi-key character composition
//    XK_ISO_Level3_Shift,        Qt::Key_AltGr,
//    XK_Multi_key,               Qt::Key_Multi_key,
//    XK_Codeinput,               Qt::Key_Codeinput,
//    XK_SingleCandidate,         Qt::Key_SingleCandidate,
//    XK_MultipleCandidate,       Qt::Key_MultipleCandidate,
//    XK_PreviousCandidate,       Qt::Key_PreviousCandidate,

    // Misc Functions
    XK_Mode_switch,             VK_MODECHANGE,
//    XK_script_switch,           Qt::Key_Mode_switch,

    // Japanese keyboard support
    XK_Kanji,                   VK_KANJI,

//    XK_Muhenkan,                Qt::Key_Muhenkan,
    //XK_Henkan_Mode,           Qt::Key_Henkan_Mode,
    //XK_Henkan_Mode,             Qt::Key_Henkan,
    //XK_Henkan,                  Qt::Key_Henkan,
    //XK_Romaji,                  Qt::Key_Romaji,
    //XK_Hiragana,                Qt::Key_Hiragana,
    //XK_Katakana,                Qt::Key_Katakana,
    //XK_Hiragana_Katakana,       Qt::Key_Hiragana_Katakana,
    //XK_Zenkaku,                 Qt::Key_Zenkaku,
    //XK_Hankaku,                 Qt::Key_Hankaku,
    //XK_Zenkaku_Hankaku,         Qt::Key_Zenkaku_Hankaku,
    //XK_Touroku,                 Qt::Key_Touroku,
    //XK_Massyo,                  Qt::Key_Massyo,
    //XK_Kana_Lock,               Qt::Key_Kana_Lock,
    //XK_Kana_Shift,              Qt::Key_Kana_Shift,
    //XK_Eisu_Shift,              Qt::Key_Eisu_Shift,
    //XK_Eisu_toggle,             Qt::Key_Eisu_toggle,
    //XK_Kanji_Bangou,          Qt::Key_Kanji_Bangou,
    //XK_Zen_Koho,              Qt::Key_Zen_Koho,
    //XK_Mae_Koho,              Qt::Key_Mae_Koho,
    //XK_Kanji_Bangou,            Qt::Key_Codeinput,
    //XK_Zen_Koho,                Qt::Key_MultipleCandidate,
    //XK_Mae_Koho,                Qt::Key_PreviousCandidate,

#ifdef XK_KOREAN
    // Korean keyboard support
    XK_Hangul,                  VK_HANGUL,
    //XK_Hangul_Start,            Qt::Key_Hangul_Start,
    //XK_Hangul_End,              Qt::Key_Hangul_End,
    //XK_Hangul_Hanja,            Qt::Key_Hangul_Hanja,
    //XK_Hangul_Jamo,             Qt::Key_Hangul_Jamo,
    //XK_Hangul_Romaja,           Qt::Key_Hangul_Romaja,
    //XK_Hangul_Codeinput,      Qt::Key_Hangul_Codeinput,
    //XK_Hangul_Codeinput,        Qt::Key_Codeinput,
    //XK_Hangul_Jeonja,           Qt::Key_Hangul_Jeonja,
    //XK_Hangul_Banja,            Qt::Key_Hangul_Banja,
    //XK_Hangul_PreHanja,         Qt::Key_Hangul_PreHanja,
    //XK_Hangul_PostHanja,        Qt::Key_Hangul_PostHanja,
    //XK_Hangul_SingleCandidate,Qt::Key_Hangul_SingleCandidate,
    //XK_Hangul_MultipleCandidate,Qt::Key_Hangul_MultipleCandidate,
    //XK_Hangul_PreviousCandidate,Qt::Key_Hangul_PreviousCandidate,
    //XK_Hangul_SingleCandidate,  Qt::Key_SingleCandidate,
    //XK_Hangul_MultipleCandidate,Qt::Key_MultipleCandidate,
    //XK_Hangul_PreviousCandidate,Qt::Key_PreviousCandidate,
    //XK_Hangul_Special,          Qt::Key_Hangul_Special,
    //XK_Hangul_switch,         Qt::Key_Hangul_switch,
    XK_Hangul_switch,           VK_MODECHANGE,
#endif  // XK_KOREAN

    // dead keys
    //XK_dead_grave,              Qt::Key_Dead_Grave,
    //XK_dead_acute,              Qt::Key_Dead_Acute,
    //XK_dead_circumflex,         Qt::Key_Dead_Circumflex,
    //XK_dead_tilde,              Qt::Key_Dead_Tilde,
    //XK_dead_macron,             Qt::Key_Dead_Macron,
    //XK_dead_breve,              Qt::Key_Dead_Breve,
    //XK_dead_abovedot,           Qt::Key_Dead_Abovedot,
    //XK_dead_diaeresis,          Qt::Key_Dead_Diaeresis,
    //XK_dead_abovering,          Qt::Key_Dead_Abovering,
    //XK_dead_doubleacute,        Qt::Key_Dead_Doubleacute,
    //XK_dead_caron,              Qt::Key_Dead_Caron,
    //XK_dead_cedilla,            Qt::Key_Dead_Cedilla,
    //XK_dead_ogonek,             Qt::Key_Dead_Ogonek,
    //XK_dead_iota,               Qt::Key_Dead_Iota,
    //XK_dead_voiced_sound,       Qt::Key_Dead_Voiced_Sound,
    //XK_dead_semivoiced_sound,   Qt::Key_Dead_Semivoiced_Sound,
    //XK_dead_belowdot,           Qt::Key_Dead_Belowdot,
    //XK_dead_hook,               Qt::Key_Dead_Hook,
    //XK_dead_horn,               Qt::Key_Dead_Horn,
};

class SXcbKeyboard {
public:
    SXcbKeyboard(SConnection* conn)
        :m_conn(conn) 
        , m_autorepeat_code(0)
        , xkb_context(0)
        , xkb_keymap(0)
        , xkb_state(0)
        , latin_keymap(0)
        , m_hasLatinLayout(false)
        , m_config(false)
    {
        memset(&xkb_names, 0, sizeof(xkb_names));
        m_key_symbols = xcb_key_symbols_alloc(m_conn->connection);
        updateModifiers();
    }

    ~SXcbKeyboard() {
        xkb_state_unref(xkb_state);
        xkb_keymap_unref(xkb_keymap);
        xkb_context_unref(xkb_context);
        xkb_keymap_unref(latin_keymap);

        xcb_key_symbols_free(m_key_symbols);
    }

    void readXKBConfig()
    {
        clearXKBConfig();
        xcb_generic_error_t* error;
        xcb_get_property_cookie_t cookie;
        xcb_get_property_reply_t* config_reply;

        xcb_connection_t* c = xcb_connection();
        xcb_window_t rootWindow = m_conn->screen->root;

        cookie = xcb_get_property(c, 0, rootWindow,
            m_conn->_XKB_RULES_NAMES, XCB_ATOM_STRING, 0, 1024);

        config_reply = xcb_get_property_reply(c, cookie, &error);
        if (!config_reply) {
            qWarning0("Qt: Couldn't interpret the _XKB_RULES_NAMES property");
            return;
        }
        char* xkb_config = (char*)xcb_get_property_value(config_reply);
        int length = xcb_get_property_value_length(config_reply);

        // on old X servers xkb_config can be 0 even if config_reply indicates a succesfull read
        if (!xkb_config || length == 0)
            return;
        // ### TODO some X servers don't set _XKB_RULES_NAMES at all, in these cases it is filled
        // with gibberish, we would need to do some kind of sanity check

        char* names[5] = { 0, 0, 0, 0, 0 };
        char* p = xkb_config, * end = p + length;
        int i = 0;
        // The result from xcb_get_property_value() is not necessarily \0-terminated,
        // we need to make sure that too many or missing '\0' symbols are handled safely.
        do {
            uint len = strnlen(p, length);
            names[i++] = p;
            p += len + 1;
            length -= len + 1;
        } while (p < end || i < 5);

        xkb_names.rules = strdup(names[0]);
        xkb_names.model = strdup(names[1]);
        xkb_names.layout = strdup(names[2]);
        xkb_names.variant = strdup(names[3]);
        xkb_names.options = strdup(names[4]);

        free(config_reply);
    }

    void clearXKBConfig()
    {
        if (xkb_names.rules)
            delete[] xkb_names.rules;
        if (xkb_names.model)
            delete[] xkb_names.model;
        if (xkb_names.layout)
            delete[] xkb_names.layout;
        if (xkb_names.variant)
            delete[] xkb_names.variant;
        if (xkb_names.options)
            delete[] xkb_names.options;
        memset(&xkb_names, 0, sizeof(xkb_names));
    }
    void updateKeymap() {
        m_config = true;
        // set xkb context object
        if (!xkb_context) {
            xkb_context = xkb_context_new((xkb_context_flags)0);
            // log only critical errors, we do our own error logging from printKeymapError()
            xkb_context_set_log_level(xkb_context, (xkb_log_level)XKB_LOG_LEVEL_CRITICAL);
        }
        // update xkb keymap object
        xkb_keymap_unref(xkb_keymap);
        xkb_keymap = 0;

        struct xkb_state* new_state = 0;
            xkb_keymap = xkb_x11_keymap_new_from_device(xkb_context, xcb_connection(), core_device_id, (xkb_keymap_compile_flags)0);
            if (xkb_keymap) {
                // Create a new keyboard state object for a keymap
                new_state = xkb_x11_state_new_from_device(xkb_keymap, xcb_connection(), core_device_id);
            }
        if (!xkb_keymap) {
            // Compile a keymap from RMLVO (rules, models, layouts, variants and options) names
            readXKBConfig();
            xkb_keymap = xkb_keymap_new_from_names(xkb_context, &xkb_names, (xkb_keymap_compile_flags)0);
            if (!xkb_keymap) {
                // last fallback is to used hard-coded keymap name, see DEFAULT_XKB_* in xkbcommon.pri
                qWarning0("Qt: Could not determine keyboard configuration data"
                    " from X server, will use hard-coded keymap configuration.");
                clearXKBConfig();
                xkb_keymap = xkb_keymap_new_from_names(xkb_context, &xkb_names, (xkb_keymap_compile_flags)0);
            }
            if (xkb_keymap) {
                new_state = xkb_state_new(xkb_keymap);
            }
            else {
                qWarning0("Qt: Failed to compile a keymap!");
                m_config = false;
                return;
            }

        }
        if (!new_state) {
            qWarning0("Qt: Failed to create xkb state!");
            m_config = false;
            return;
        }
        // update xkb state object
        xkb_state_unref(xkb_state);
        xkb_state = new_state;
        updateXKBMods();

        checkForLatinLayout();
    }
    
    void checkForLatinLayout()
    {
        m_hasLatinLayout = false;
        const xkb_layout_index_t layoutCount = xkb_keymap_num_layouts(xkb_keymap);
        const xcb_keycode_t minKeycode = m_conn->m_setup->min_keycode;
        const xcb_keycode_t maxKeycode = m_conn->m_setup->max_keycode;
        struct xkb_state* kb_state = xkb_state_new(xkb_keymap);
        for (xkb_layout_index_t layout = 0; layout < layoutCount; ++layout) {
            xkb_state_update_mask(kb_state, 0, 0, 0, 0, 0, layout);
            for (xcb_keycode_t code = minKeycode; code < maxKeycode; ++code) {
                xkb_keysym_t sym = xkb_state_key_get_one_sym(kb_state, code);
                // if layout can produce any of these latin letters (chosen
                // arbitrarily) then it must be a latin key based layout
                if (sym == XK_q || sym == XK_a || sym == XK_e) {
                    m_hasLatinLayout = true;
                    xkb_state_unref(kb_state);
                    return;
                }
            }
        }
        xkb_state_unref(kb_state);
    }


    void updateXKBMods()
    {
        xkb_mods.shift = xkb_keymap_mod_get_index(xkb_keymap, XKB_MOD_NAME_SHIFT);
        xkb_mods.lock = xkb_keymap_mod_get_index(xkb_keymap, XKB_MOD_NAME_CAPS);
        xkb_mods.control = xkb_keymap_mod_get_index(xkb_keymap, XKB_MOD_NAME_CTRL);
        xkb_mods.mod1 = xkb_keymap_mod_get_index(xkb_keymap, "Mod1");
        xkb_mods.mod2 = xkb_keymap_mod_get_index(xkb_keymap, "Mod2");
        xkb_mods.mod3 = xkb_keymap_mod_get_index(xkb_keymap, "Mod3");
        xkb_mods.mod4 = xkb_keymap_mod_get_index(xkb_keymap, "Mod4");
        xkb_mods.mod5 = xkb_keymap_mod_get_index(xkb_keymap, "Mod5");
    }


    uint32_t xkbModMask(uint16_t state)
    {
        uint32_t xkb_mask = 0;

        if ((state & XCB_MOD_MASK_SHIFT) && xkb_mods.shift != XKB_MOD_INVALID)
            xkb_mask |= (1 << xkb_mods.shift);
        if ((state & XCB_MOD_MASK_LOCK) && xkb_mods.lock != XKB_MOD_INVALID)
            xkb_mask |= (1 << xkb_mods.lock);
        if ((state & XCB_MOD_MASK_CONTROL) && xkb_mods.control != XKB_MOD_INVALID)
            xkb_mask |= (1 << xkb_mods.control);
        if ((state & XCB_MOD_MASK_1) && xkb_mods.mod1 != XKB_MOD_INVALID)
            xkb_mask |= (1 << xkb_mods.mod1);
        if ((state & XCB_MOD_MASK_2) && xkb_mods.mod2 != XKB_MOD_INVALID)
            xkb_mask |= (1 << xkb_mods.mod2);
        if ((state & XCB_MOD_MASK_3) && xkb_mods.mod3 != XKB_MOD_INVALID)
            xkb_mask |= (1 << xkb_mods.mod3);
        if ((state & XCB_MOD_MASK_4) && xkb_mods.mod4 != XKB_MOD_INVALID)
            xkb_mask |= (1 << xkb_mods.mod4);
        if ((state & XCB_MOD_MASK_5) && xkb_mods.mod5 != XKB_MOD_INVALID)
            xkb_mask |= (1 << xkb_mods.mod5);

        return xkb_mask;
    }

    void updateXKBStateFromState(struct xkb_state* kb_state, uint16_t state)
    {
        const uint32_t modsDepressed = xkb_state_serialize_mods(kb_state, XKB_STATE_MODS_DEPRESSED);
        const uint32_t modsLatched = xkb_state_serialize_mods(kb_state, XKB_STATE_MODS_LATCHED);
        const uint32_t modsLocked = xkb_state_serialize_mods(kb_state, XKB_STATE_MODS_LOCKED);
        const uint32_t xkbMask = xkbModMask(state);

        const uint32_t latched = modsLatched & xkbMask;
        const uint32_t locked = modsLocked & xkbMask;
        uint32_t depressed = modsDepressed & xkbMask;
        // set modifiers in depressed if they don't appear in any of the final masks
        depressed |= ~(depressed | latched | locked) & xkbMask;

        const xkb_state_component newState
            = xkb_state_update_mask(kb_state,
                depressed,
                latched,
                locked,
                0,
                0,
                (state >> 13) & 3); // bits 13 and 14 report the state keyboard group

        if ((newState & XKB_STATE_LAYOUT_EFFECTIVE) == XKB_STATE_LAYOUT_EFFECTIVE) {
            //qWarning("TODO: Support KeyboardLayoutChange on QPA (QTBUG-27681)");
        }
    }

    uint32_t translateModifiers(int s) const
    {
        UINT ret = 0;
        if (s & XCB_MOD_MASK_SHIFT)
            ret |= XCB_MOD_MASK_SHIFT;
        if (s & XCB_MOD_MASK_CONTROL)
            ret |= XCB_MOD_MASK_CONTROL;
        if (s & rmod_masks.alt)
            ret |= XCB_MOD_MASK_1;
        return ret;
    }


    int lookVkMap(xcb_keysym_t key) const
    {
        for (int i = 0; i < ARRAYSIZE(KeyTbl); i++) {
            if (key == KeyTbl[i].scanCode)
                return KeyTbl[i].vk;
        }
        return 0;
    }

    int scanCodeToVk(xcb_keysym_t keysym, UINT  modifiers) const
    {
        int code = 0;

        // Commentary in X11/keysymdef says that X codes match ASCII, so it
        // is safe to use the locale functions to process X codes in ISO8859-1.
        // This is mainly for compatibility - applications should not use the
        // Qt keycodes between 128 and 255 (extended ACSII codes), but should
        // rather use the QKeyEvent::text().
        if (keysym < 128) {
            // upper-case key, if known
            code = isprint((int)keysym) ? toupper((int)keysym) : 0;
        }
        else if (keysym >= XK_F1 && keysym <= XK_F35) {
            // function keys
            code = VK_F1 + ((int)keysym - XK_F1);
        }
        else if (keysym >= XK_KP_Space && keysym <= XK_KP_9) {
            if (keysym >= XK_KP_0) {
                // numeric keypad keys
                code = VK_NUMPAD0 + ((int)keysym - XK_KP_0);
            }
            else {
                code = lookVkMap(keysym);
            }
        }
        else {
            // any other keys
            code = lookVkMap(keysym);
        }

        return code;
    }

    uint32_t translateVk(xcb_keycode_t code, uint16_t state, xcb_timestamp_t ts) {
        struct xkb_state* kb_state = xkb_state_new(xkb_keymap);
        if (!kb_state)
            return 0;
        updateXKBStateFromState(kb_state, state);
        xcb_keysym_t sym = xkb_state_key_get_one_sym(kb_state, code);
        uint32_t modifiers = translateModifiers(state);
        uint32_t vk = scanCodeToVk(sym, modifiers);
        //todo: handle auto repeat
        xkb_state_unref(kb_state);
        return vk;
    }

protected:
    xcb_connection_t* xcb_connection() {
        return m_conn->connection;
    }

    void updateModifiers()
    {
        // The core protocol does not provide a convenient way to determine the mapping
        // of modifier bits. Clients must retrieve and search the modifier map to determine
        // the keycodes bound to each modifier, and then retrieve and search the keyboard
        // mapping to determine the keysyms bound to the keycodes. They must repeat this
        // process for all modifiers whenever any part of the modifier mapping is changed.
        memset(&rmod_masks, 0, sizeof(rmod_masks));

        xcb_generic_error_t* error = 0;
        xcb_connection_t* conn = xcb_connection();
        xcb_get_modifier_mapping_cookie_t modMapCookie = xcb_get_modifier_mapping(conn);
        xcb_get_modifier_mapping_reply_t* modMapReply =
            xcb_get_modifier_mapping_reply(conn, modMapCookie, &error);
        if (error) {
            DBG_LOG0("Qt: failed to get modifier mapping");
            free(error);
            return;
        }

        // for Alt and Meta L and R are the same
        static const xcb_keysym_t symbols[] = {
            XK_Alt_L, XK_Meta_L, XK_Mode_switch, XK_Super_L, XK_Super_R,
            XK_Hyper_L, XK_Hyper_R
        };
        static const size_t numSymbols = sizeof symbols / sizeof * symbols;

        // Figure out the modifier mapping, ICCCM 6.6
        xcb_keycode_t* modKeyCodes[numSymbols];
        for (size_t i = 0; i < numSymbols; ++i)
            modKeyCodes[i] = xcb_key_symbols_get_keycode(m_key_symbols, symbols[i]);

        xcb_keycode_t* modMap = xcb_get_modifier_mapping_keycodes(modMapReply);
        const int w = modMapReply->keycodes_per_modifier;
        for (size_t i = 0; i < numSymbols; ++i) {
            for (int bit = 0; bit < 8; ++bit) {
                uint mask = 1 << bit;
                for (int x = 0; x < w; ++x) {
                    xcb_keycode_t keyCode = modMap[x + bit * w];
                    xcb_keycode_t* itk = modKeyCodes[i];
                    while (itk && *itk != XCB_NO_SYMBOL)
                        if (*itk++ == keyCode) {
                            uint sym = symbols[i];
                            if ((sym == XK_Alt_L || sym == XK_Alt_R))
                                rmod_masks.alt = mask;
                            if ((sym == XK_Meta_L || sym == XK_Meta_R))
                                rmod_masks.meta = mask;
                            if (sym == XK_Mode_switch)
                                rmod_masks.altgr = mask;
                            if ((sym == XK_Super_L) || (sym == XK_Super_R))
                                rmod_masks.super = mask;
                            if ((sym == XK_Hyper_L) || (sym == XK_Hyper_R))
                                rmod_masks.hyper = mask;
                        }
                }
            }
        }

        for (size_t i = 0; i < numSymbols; ++i)
            free(modKeyCodes[i]);
        free(modMapReply);
        resolveMaskConflicts();
    }


    void resolveMaskConflicts()
    {
        // if we don't have a meta key (or it's hidden behind alt), use super or hyper to generate
        // Qt::Key_Meta and Qt::MetaModifier, since most newer XFree86/Xorg installations map the Windows
        // key to Super
        if (rmod_masks.alt == rmod_masks.meta)
            rmod_masks.meta = 0;

        if (rmod_masks.meta == 0) {
            // no meta keys... s/meta/super,
            rmod_masks.meta = rmod_masks.super;
            if (rmod_masks.meta == 0) {
                // no super keys either? guess we'll use hyper then
                rmod_masks.meta = rmod_masks.hyper;
            }
        }
    }
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
};

SConnMgr *SConnMgr::instance()
{
    if (!s_connMgr)
    {
        SAutoLock lock(s_cs);
        if (!s_connMgr)
        {
            s_connMgr = new SConnMgr();
        }
    }
    return s_connMgr;
}

void SConnMgr::free()
{
    SAutoLock lock(s_cs);
    if (s_connMgr)
    {
        delete s_connMgr;
        s_connMgr = NULL;
    }
}

//----------------------------------------------------------
SConnMgr::~SConnMgr()
{
    SAutoWriteLock autoLock(&m_rwLock);
    auto it = m_conns.begin();
    while(it!=m_conns.end()){
        delete it->second;
        it++;
    }
    m_conns.clear();
}

void SConnMgr::removeConn(SConnection *pObj)
{
    SAutoWriteLock autoLock(&m_rwLock);
    pthread_t tid = pthread_self();
    auto it = m_conns.find(tid);
    if (it == m_conns.end())
    {
        return;
    }
    assert(it->second == pObj);
    delete it->second;
    m_conns.erase(it);
}

SConnection * SConnMgr::getConnection(pthread_t tid_,int screenNum){
   pthread_t tid = tid_!=0? tid_:pthread_self();   
    {
        SAutoReadLock autoLock(&m_rwLock);
        auto it = m_conns.find(tid);
        if (it != m_conns.end())
        {
            return it->second;
        }
    }
    {
        //not found
        SAutoWriteLock autoLock(&m_rwLock);
        SConnection * state = new SConnection(screenNum);        
        m_conns[tid]=state;
        return state;
    }
}

xcb_atom_t SConnMgr::atom(const char *name,bool onlyIfExist){
    SConnection *trdUiState = getConnection();
    if(!trdUiState)
        return 0;
    return internAtom(trdUiState->connection,onlyIfExist?1:0,name);
}

//---------------------------------------------------------------------
xcb_atom_t SConnMgr::internAtom(xcb_connection_t *connection, uint8_t onlyIfExist, const char *atomName)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, onlyIfExist, strlen(atomName), atomName);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, NULL);
    xcb_atom_t atom = XCB_ATOM_NONE;

    if (reply)
    {
        atom = reply->atom;
        ::free(reply);
    }

    return atom;
}

//todo: 
static uint32_t GetDoubleClickSpan(xcb_connection_t* connection,xcb_screen_t * screen) {
    uint32_t ret = 200;
    xcb_window_t root_window = screen->root;

    xcb_atom_t atom = SConnMgr::internAtom(connection, 0, "_NET_DOUBLE_CLICK_TIME");
    xcb_get_property_cookie_t cookie = xcb_get_property(connection, 0, root_window, atom, XCB_ATOM_CARDINAL, 0, 1024);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(connection, cookie, NULL);
    if (reply == NULL) {
        fprintf(stderr, "Failed to get property value\n");
        return ret;
    }
    
    if (reply->value_len == 1) {
        ret = *((uint32_t*)xcb_get_property_value(reply));
    }
    free(reply);
    return ret;
}

SConnection::SConnection(int screenNum):m_keyboard(nullptr)
{
    connection = xcb_connect(nullptr, &screenNum);
    if (xcb_connection_has_error(connection) > 0)
    {
        connection = NULL;
        return;
    }
    
    /* Get the screen whose number is screenNum */

    m_setup = xcb_get_setup(connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(m_setup);

    // we want the screen at index screenNum of the iterator
    for (int i = 0; i < screenNum; ++i)
    {
        xcb_screen_next(&iter);
    }
    if (!iter.data)
    {
        // get the first screen
        iter = xcb_setup_roots_iterator(m_setup);
    }
    screen = iter.data;

    m_keyboard = new SXcbKeyboard(this);

    m_tsDoubleSpan = GetDoubleClickSpan(connection, screen);

    WM_PROTOCOLS_ATOM = SConnMgr::internAtom(connection, 1, "WM_PROTOCOLS");
    WM_DELETE_WINDOW_ATOM = SConnMgr::internAtom(connection, 1, "WM_DELETE_WINDOW");
    WM_STATE_ATOM = SConnMgr::internAtom(connection, 1, "WM_STATE");
    WM_CLASS_ATOM = SConnMgr::internAtom(connection, 1, "WM_CLASS");
    WM_NAME_ATOM = SConnMgr::internAtom(connection, 1, "WM_NAME");
    WM_CHANGE_STATE_ATOM = SConnMgr::internAtom(connection, 1, "WM_CHANGE_STATE");
    _NET_WM_STATE_ATOM = SConnMgr::internAtom(connection, 1, "_NET_WM_STATE");
    _NET_WM_STATE_HIDDEN_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_HIDDEN");
    _NET_WM_STATE_DEMANDS_ATTENTION_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_DEMANDS_ATTENTION");

    _NET_WM_STATE_ABOVE_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_ABOVE");
    _NET_WM_STATE_BELOW_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_BELOW");
    _NET_WM_STATE_FULLSCREEN_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_FULLSCREEN");
    _NET_WM_STATE_STAYS_ON_TOP_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_STAYS_ON_TOP");

    _NET_WM_STATE_MAXIMIZED_HORZ_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_MAXIMIZED_HORZ");
    _NET_WM_STATE_MAXIMIZED_VERT_ATOM = SConnMgr::internAtom(connection,1,"_NET_WM_STATE_MAXIMIZED_VERT");
    _NET_WM_ICON = SConnMgr::internAtom(connection, 1, "_NET_WM_ICON");

    _NET_WM_ICON_NAME = SConnMgr::internAtom(connection,1,"_NET_WM_ICON_NAME");
    _NET_WM_ICON = SConnMgr::internAtom(connection,1,"_NET_WM_ICON");
    _XKB_RULES_NAMES = SConnMgr::internAtom(connection, 1, "_XKB_RULES_NAMES");
    m_bQuit=false;
    m_msgPeek = nullptr;
    m_bMsgNeedFree = false;
    m_hWndCapture = 0;
    m_hWndActive = 0;
    m_hCursor = 0;
    m_bBlockTimer = false;

    m_trdEvtReader = std::move(std::thread(std::bind(&readProc, this)));

    m_deskDC = new _SDC(screen->root);
    m_deskBmp = CreateCompatibleBitmap(m_deskDC, 10, 10);
    SelectObject(m_deskDC, m_deskBmp);
}

SConnection::~SConnection()
{
    if (!connection)
    {
        return;
    }

    for(auto it : m_sysCursor){
        xcb_free_cursor(connection,it.second);
    }
    m_sysCursor.clear();
    for(auto it:m_stdCursor){
        DestroyIcon(it.second);
    }
    m_stdCursor.clear();

    m_bQuit = true;
    xcb_disconnect(connection);

    m_trdEvtReader.join();

    delete m_keyboard;

    for(auto it:m_msgQueue){
        delete it;
    }
    m_msgQueue.clear();
    if(m_msgPeek && m_bMsgNeedFree){
        delete m_msgPeek;
        m_msgPeek = nullptr;
        m_bMsgNeedFree = false;
    }
    delete m_deskDC;
    DeleteObject(m_deskBmp);
}


bool SConnection::update(){
    UINT timeOut = -1;
    if(!m_bBlockTimer){
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        for (auto& it : m_lstTimer) {
            timeOut = std::min(timeOut, it.fireRemain);
        }
    }
    std::unique_lock<std::mutex> lock(m_mutex4Evt);
    uint64_t ts1 = GetTickCount64();
    m_varCondition.wait_for(lock, std::chrono::milliseconds(timeOut), [&]{return !m_evtQueue.empty(); });

    for(auto it:m_evtQueue){
        pushEvent(it);
        free(it);
    }
    bool bRet = !m_evtQueue.empty();
    m_evtQueue.clear();
    uint64_t ts2 = GetTickCount64();
    if(!m_bBlockTimer){
        UINT elapse = ts2 - ts1;
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        for (auto& it : m_lstTimer) {
            if (it.fireRemain <= elapse) {
                //fire timer event
                Msg* pMsg = new Msg;
                pMsg->hwnd = it.hWnd;
                pMsg->message = WM_TIMER;
                pMsg->wParam = it.id;
                pMsg->lParam = (LPARAM)it.proc;
                pMsg->time = ts2;
                m_msgQueue.push_back(pMsg);
                it.fireRemain = it.elapse;
                bRet = true;
            }
            else {
                it.fireRemain -= elapse;
            }
        }
    }
    return bRet;
}

BOOL SConnection::TranslateMessage(const MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN&& isprint(pMsg->wParam)) {
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);

        Msg *msg = new Msg;
        msg->message = WM_CHAR;
        msg->hwnd = pMsg->hwnd;
        msg->wParam = pMsg->wParam;
        msg->lParam = pMsg->lParam;

        SHORT shiftState = GetKeyState(VK_SHIFT);
        if (!(shiftState & 0x8000) && pMsg->wParam>='A' && pMsg->wParam<='Z') {
            msg->wParam += 0x20;//tolower
        }
        m_msgQueue.push_back(msg);
        return TRUE;
    }
    return FALSE;
}

BOOL SConnection::peekMsg(THIS_ LPMSG pMsg, HWND  hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT  wRemoveMsg) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
	auto it = m_msgQueue.begin();
	for (; it != m_msgQueue.end(); it++) {
		BOOL bMatch = TRUE;
		Msg* msg = (*it);
		do {
			if (msg->message == WM_QUIT)
				break;
			if (msg->message == WM_TIMER && msg->lParam == 0)
				break;
			if (msg->hwnd != hWnd && hWnd != 0)
			{
				bMatch = FALSE;
				break;
			}
			if (wMsgFilterMin == 0 && wMsgFilterMax == 0)
				break;
			if (wMsgFilterMin <= msg->message && wMsgFilterMax >= msg->message)
				break;
			bMatch = FALSE;
		} while (false);
        if (bMatch) break;
	}
	if (it != m_msgQueue.end()) {
		Msg* msg = (*it);
		if (m_msgPeek && m_bMsgNeedFree) {
			delete m_msgPeek;
			m_msgPeek = nullptr;
			m_bMsgNeedFree=false;
		}
		if (msg->message == WM_TIMER && msg->lParam) {
			TIMERPROC proc = (TIMERPROC)msg->lParam;
			proc(msg->hwnd, WM_TIMER, msg->wParam, msg->time);
			m_msgQueue.erase(it);
			return PeekMessage(pMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
		}
		m_msgPeek = msg;
		if (wRemoveMsg & PM_NOREMOVE)
			m_bMsgNeedFree = false;
		else
		{
			m_msgQueue.erase(it);
			m_bMsgNeedFree = true;
		}
		memcpy(pMsg, (MSG*)m_msgPeek, sizeof(MSG));

		return TRUE;
	}

	return FALSE;
}

void SConnection::postMsg(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
    Msg *pMsg = new Msg;
    pMsg->hwnd = hWnd;
    pMsg->message = message;
    pMsg->wParam = wp;
    pMsg->lParam = lp;
    m_msgQueue.push_back(pMsg);
}

UINT_PTR SConnection::SetTimer(HWND hWnd, UINT_PTR id, UINT uElapse,
    TIMERPROC proc)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
    if (hWnd) {
        //find exist timer.
        for (auto& it : m_lstTimer) {
            if (it.hWnd != hWnd)
                continue;
            if (it.id == id) {
                it.fireRemain = uElapse;
                it.proc = proc;
                return id;
            }
        }
        TimerInfo timer;
        timer.id = id;
        timer.fireRemain = uElapse;
        timer.hWnd = hWnd;
        timer.proc = proc;
        timer.elapse = uElapse;
        m_lstTimer.push_back(timer);
        return id;
    }
    else {
        UINT_PTR newId = 0;
        for (auto& it : m_lstTimer) {
            if (it.hWnd)
                continue;
            newId = std::max(it.id, newId);
        }
        TimerInfo timer;
        timer.id = newId+1;
        timer.fireRemain = uElapse;
        timer.hWnd = hWnd;
        timer.proc = proc;
        timer.elapse = uElapse;
        m_lstTimer.push_back(timer);
        return newId;
    }
}

BOOL SConnection::KillTimer(HWND hWnd,
    UINT_PTR id)
{

    std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
    for (auto it = m_lstTimer.begin(); it != m_lstTimer.end(); it++) {
        if (it->hWnd == hWnd && it->id == id) {
            m_lstTimer.erase(it);
            return TRUE;
        }
    }
    return FALSE;
}

HDC SConnection::GetDC()
{
    return m_deskDC;
}

BOOL SConnection::ReleaseDC(HDC hdc)
{
    //todo:hjx
    return TRUE;
}

HWND SConnection::SetCapture(HWND hCapture)
{
    xcb_grab_pointer(
        connection,
        1, // 这个标志位表示不使用对子窗口的事件
        hCapture,
        XCB_EVENT_MASK_BUTTON_PRESS|XCB_EVENT_MASK_BUTTON_RELEASE|XCB_EVENT_MASK_POINTER_MOTION, 
        XCB_GRAB_MODE_ASYNC, // 异步捕获
        XCB_GRAB_MODE_ASYNC,
        XCB_NONE, // 捕获事件的窗口
        XCB_NONE, // 使用默认光标
        XCB_CURRENT_TIME // 立即开始捕获
    );

    HWND ret = m_hWndCapture;
    m_hWndCapture = hCapture;
    return ret;
}

BOOL SConnection::ReleaseCapture()
{
    if(!m_hWndCapture)
        return FALSE;
    m_hWndCapture = 0;
    xcb_ungrab_pointer(connection,XCB_CURRENT_TIME);
    return TRUE;
}

HWND SConnection::GetCapture() const{
    return m_hWndCapture;
}

xcb_cursor_t SConnection::createXcbCursor(HCURSOR cursor)
{
        xcb_generic_error_t *error = 0;
        xcb_render_query_pict_formats_cookie_t formatsCookie = xcb_render_query_pict_formats(connection);
        xcb_render_query_pict_formats_reply_t *formatsReply = xcb_render_query_pict_formats_reply(connection,
                                                                                                formatsCookie,
                                                                                                &error);
        if (!formatsReply || error) {
            //qWarning("qt_xcb_createCursorXRender: query_pict_formats failed");
            free(formatsReply);
            free(error);
            return XCB_NONE;
        }
        xcb_render_pictforminfo_t *fmt = xcb_render_util_find_standard_format(formatsReply,
                                                                            XCB_PICT_STANDARD_ARGB_32);
        if (!fmt) {
            //qWarning("qt_xcb_createCursorXRender: Failed to find format PICT_STANDARD_ARGB_32");
            free(formatsReply);
            return XCB_NONE;
        }
    xcb_cursor_t xcb_cursor = XCB_NONE;
        ICONINFO info={0};
        GetIconInfo(cursor,&info);
        assert(info.fIcon == 0);
        assert(info.hbmColor);
        BITMAP bm;
        GetObject(info.hbmColor,sizeof(bm),&bm);
        if(bm.bmBitsPixel!=32){
            goto end;
        }

    do{
        xcb_pixmap_t pix = xcb_generate_id(connection);
        xcb_create_pixmap(connection, 32, pix, screen->root, bm.bmWidth, bm.bmHeight);

        xcb_render_picture_t pic = xcb_generate_id(connection);
        xcb_render_create_picture(connection, pic, pix, fmt->id, 0, 0);

        xcb_gcontext_t gc = xcb_generate_id(connection);

        xcb_image_t *xi = xcb_image_create(bm.bmWidth, bm.bmHeight, XCB_IMAGE_FORMAT_Z_PIXMAP,
                                       32, 32, 32, 32,
                                       XCB_IMAGE_ORDER_LSB_FIRST,//todo:hjx
                                       XCB_IMAGE_ORDER_MSB_FIRST,
                                       0, 0, 0);
        if (!xi) {
            //qWarning("qt_xcb_createCursorXRender: xcb_image_create failed");
            break;
        }
        xi->data = (uint8_t *) malloc(xi->stride * bm.bmHeight);
        if (!xi->data) {
            //qWarning("qt_xcb_createCursorXRender: Failed to malloc() image data");
            xcb_image_destroy(xi);
            break;
        }
        memcpy(xi->data, bm.bmBits,bm.bmWidth*4*bm.bmHeight);

        xcb_create_gc(connection, gc, pix, 0, 0);
        xcb_image_put(connection, pix, gc, xi, 0, 0, 0);
        xcb_free_gc(connection, gc);

        xcb_cursor = xcb_generate_id(connection);
        xcb_render_create_cursor(connection, xcb_cursor, pic, info.xHotspot, info.yHotspot);

        free(xi->data);
        xcb_image_destroy(xi);
        xcb_render_free_picture(connection, pic);
        xcb_free_pixmap(connection, pix);

    }while(false);
 
    free(formatsReply);
end:
    if(info.hbmColor)
        DeleteObject(info.hbmColor);
    if(info.hbmMask)
        DeleteObject(info.hbmMask);

    return xcb_cursor;
}

HCURSOR SConnection::SetCursor(HCURSOR cursor)
{
    if(!cursor)
        cursor = LoadCursor(IDC_ARROW);
    assert(cursor);
    if(cursor == m_hCursor)
        return cursor;
    xcb_cursor_t xcbCursor = 0;
    if(m_sysCursor.find(cursor)!=m_sysCursor.end()){
        xcbCursor = m_sysCursor[cursor];
    }else{
        xcbCursor = createXcbCursor(cursor);
        if(!xcbCursor)
        {
            //todo: print error
            return 0;
        }
    }
    HCURSOR ret = m_hCursor;
    uint32_t val[]={xcbCursor};
    xcb_change_window_attributes(connection, m_hWndActive?m_hWndActive: screen->root, XCB_CW_CURSOR,val);
    m_hCursor= cursor;
    return ret;
}


#define CIDC_ARROW           (32512)
#define CIDC_IBEAM           (32513)
#define CIDC_WAIT            (32514)
#define CIDC_CROSS           (32515)
#define CIDC_UPARROW         (32516)
#define CIDC_SIZE            (32640)  
#define CIDC_ICON            (32641)  
#define CIDC_SIZENWSE        (32642)
#define CIDC_SIZENESW        (32643)
#define CIDC_SIZEWE          (32644)
#define CIDC_SIZENS          (32645)
#define CIDC_SIZEALL         (32646)
#define CIDC_NO              (32648) 
#define CIDC_HAND            (32649)
#define CIDC_APPSTARTING     (32650) 
#define CIDC_HELP            (32651)

struct CData {
    const unsigned char* buf;
    UINT   length;
};

static bool getStdCursorCData(WORD wId, CData & data)
{
    bool ret = true;
    switch (wId) {
    case CIDC_ARROW: data.buf = ocr_normal_cur; data.length = sizeof(ocr_normal_cur); break;;
    case CIDC_IBEAM: data.buf = ocr_ibeam_cur; data.length = sizeof(ocr_ibeam_cur); break;;
    case CIDC_WAIT:  data.buf = ocr_wait_cur; data.length = sizeof(ocr_wait_cur); break;
    case CIDC_CROSS:  data.buf = ocr_cross_cur; data.length = sizeof(ocr_cross_cur); break;
    case CIDC_UPARROW:  data.buf = ocr_up_cur; data.length = sizeof(ocr_up_cur); break;
    case CIDC_SIZE:  data.buf = ocr_size_cur; data.length = sizeof(ocr_size_cur); break;
    case CIDC_SIZEALL:  data.buf = ocr_sizeall_cur; data.length = sizeof(ocr_sizeall_cur); break;
    case CIDC_ICON:  data.buf = ocr_icon_cur; data.length = sizeof(ocr_icon_cur); break;
    case CIDC_SIZENWSE:  data.buf = ocr_sizenwse_cur; data.length = sizeof(ocr_sizenwse_cur); break;
    case CIDC_SIZENESW:  data.buf = ocr_sizenesw_cur; data.length = sizeof(ocr_sizenesw_cur); break;
    case CIDC_SIZEWE: data.buf = ocr_sizewe_cur; data.length = sizeof(ocr_sizewe_cur); break;
    case CIDC_SIZENS:  data.buf = ocr_sizens_cur; data.length = sizeof(ocr_sizens_cur); break;
    case CIDC_HAND:  data.buf = ocr_hand_cur; data.length = sizeof(ocr_hand_cur); break;
    case CIDC_HELP:  data.buf = ocr_help_cur; data.length = sizeof(ocr_help_cur); break;
    case CIDC_APPSTARTING: data.buf = ocr_appstarting_cur; data.length = sizeof(ocr_appstarting_cur); break;
    default:
        ret = false;
        break;
    }
    return ret;
}

HCURSOR SConnection::LoadCursor(LPCSTR lpCursorName)
{
    HCURSOR ret = 0;
    if(IS_INTRESOURCE(lpCursorName)){
        WORD wId = (WORD)(ULONG_PTR)lpCursorName;
        if(m_stdCursor.find(wId)!=m_stdCursor.end())
            return m_stdCursor[wId];
        CData data;
        if (!getStdCursorCData(wId, data)) {
            getStdCursorCData(CIDC_ARROW, data);
        }
        ret = (HCURSOR)LoadImageBuf((PBYTE)data.buf,data.length, IMAGE_CURSOR,0,0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR);
        assert(ret);
        m_stdCursor.insert(std::make_pair(wId,ret));
    }else{
        ret = (HCURSOR)LoadImage(0,lpCursorName,IMAGE_CURSOR,0,0,LR_LOADFROMFILE|LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
    }
    return ret;
}

BOOL SConnection::DestroyCursor(HCURSOR cursor)
{
    //look for sys cursor
    auto it = m_sysCursor.find(cursor);
    if(it==m_sysCursor.end())
        return FALSE;
    for(auto it : m_stdCursor){
        if(cursor == it.second)
            return TRUE;
    }
    m_sysCursor.erase(cursor);
    DestroyIcon((HICON)cursor);
    return TRUE;
}

static uint32_t TsSpan(uint32_t t1, uint32_t t2) {
    if (t1 > t2) {
        return t1 - t2;
    }
    else {
        return t1 + (0xffffffff - t2);
    }
}

static WPARAM ButtonState2Mask(uint16_t state) {
    WPARAM wp = 0;
    if (state & XCB_KEY_BUT_MASK_SHIFT)
        wp |= MK_SHIFT;
    if (state & XCB_KEY_BUT_MASK_CONTROL)
        wp |= MK_CONTROL;
    if (state & XCB_BUTTON_MASK_1)
        wp |= MK_LBUTTON;
    if (state & XCB_BUTTON_MASK_2)
        wp |= MK_RBUTTON;
    if (state & XCB_BUTTON_MASK_3)
        wp |= MK_RBUTTON;
    return wp;
}

HWND SConnection::SetActiveWindow(HWND hWnd)
{
    HWND ret = m_hWndActive;
    xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT, hWnd, XCB_CURRENT_TIME);
    m_hWndActive = hWnd;
    return ret;
}

HWND SConnection::GetParentWnd(HWND hWnd) const
{
    return GetWindow(hWnd, GW_PARENT);

}

HWND SConnection::GetWindow(HWND hWnd, int code) const
{
    xcb_query_tree_cookie_t tree_cookie = xcb_query_tree(connection, hWnd);
    xcb_query_tree_reply_t* tree_reply = xcb_query_tree_reply(connection, tree_cookie, NULL);
    if (!tree_reply)
        return 0;
    HWND ret = 0;
    switch(code){
    case GW_CHILDFIRST:
    if(tree_reply->children_len>0){
        xcb_window_t* children = xcb_query_tree_children(tree_reply);
        ret = children[0];
    }
        break;
    case GW_CHILDLAST:
        if (tree_reply->children_len > 0) {
            xcb_window_t* children = xcb_query_tree_children(tree_reply);
            ret = children[tree_reply->children_len-1];
        }
        break;
    case GW_HWNDFIRST: 
        if(tree_reply->parent){
            ret = GetWindow(tree_reply->parent, GW_CHILDFIRST);
        }
        break;
    case GW_HWNDLAST:
        if (tree_reply->parent) {
            ret = GetWindow(tree_reply->parent, GW_CHILDLAST);
        }
        break;
    case GW_PARENT:
        ret = tree_reply->parent;
        break;
    }
    free(tree_reply);
    return ret;
}

HWND SConnection::WindowFromPoint(POINT pt,HWND hWnd) const
{
    if (!hWnd)
        hWnd = screen->root;
    xcb_query_tree_reply_t* reply = xcb_query_tree_reply(
        connection,
        xcb_query_tree(connection, hWnd),
        0
    );

    xcb_window_t* children = xcb_query_tree_children(reply);
    int num_children = xcb_query_tree_children_length(reply);
    xcb_window_t result = XCB_WINDOW_NONE;

    for (int i = 0; i < num_children; i++) {
        xcb_get_geometry_reply_t* geometry = xcb_get_geometry_reply(connection, xcb_get_geometry(connection, children[i]), NULL);
        if (!geometry) continue;

        if (pt.x >= geometry->x && pt.x < (geometry->x + geometry->width) &&
            pt.y >= geometry->y && pt.y < (geometry->y + geometry->height)) {
            result = children[i];
        }
        free(geometry);
        if(result != XCB_WINDOW_NONE)
            break;
    }
    free(reply);
    if (!result)
        return hWnd;
    else
        return WindowFromPoint(pt, result);
}

BOOL SConnection::GetCursorPos(LPPOINT ppt) const {
    // 获取当前鼠标位置的请求
    xcb_query_pointer_cookie_t pointer_cookie = xcb_query_pointer(connection, screen->root);
    xcb_query_pointer_reply_t* pointer_reply = xcb_query_pointer_reply(connection, pointer_cookie, NULL);
    if (!pointer_reply) {
        fprintf(stderr, "Failed to get mouse position\n");
        return FALSE;
    }
    ppt->x = pointer_reply->root_x;
    ppt->y = pointer_reply->root_y;
    // 释放资源
    free(pointer_reply);
    return TRUE;
}


uint32_t SConnection::netWmStates(HWND hWnd)
{
    uint32_t result(0);

    xcb_get_property_cookie_t get_cookie =
        xcb_get_property_unchecked(connection, 0, hWnd, _NET_WM_STATE_ATOM,
                         XCB_ATOM_ATOM, 0, 1024);

    xcb_get_property_reply_t *reply =
        xcb_get_property_reply(connection, get_cookie, NULL);

    if (reply && reply->format == 32 && reply->type == XCB_ATOM_ATOM) {
        const xcb_atom_t *states = static_cast<const xcb_atom_t *>(xcb_get_property_value(reply));
        const xcb_atom_t *statesEnd = states + reply->length;
        if (statesEnd != std::find(states, statesEnd, _NET_WM_STATE_ABOVE_ATOM))
            result |= NetWmStateAbove;
        if (statesEnd != std::find(states, statesEnd, _NET_WM_STATE_BELOW_ATOM))
            result |= NetWmStateBelow;
        if (statesEnd != std::find(states, statesEnd, _NET_WM_STATE_FULLSCREEN_ATOM))
            result |= NetWmStateFullScreen;
        if (statesEnd != std::find(states, statesEnd, _NET_WM_STATE_MAXIMIZED_HORZ_ATOM))
            result |= NetWmStateMaximizedHorz;
        if (statesEnd != std::find(states, statesEnd, _NET_WM_STATE_MAXIMIZED_VERT_ATOM))
            result |= NetWmStateMaximizedVert;
        if (statesEnd != std::find(states, statesEnd, _NET_WM_STATE_STAYS_ON_TOP_ATOM))
            result |= NetWmStateStaysOnTop;
        if (statesEnd != std::find(states, statesEnd, _NET_WM_STATE_DEMANDS_ATTENTION_ATOM))
            result |= NetWmStateDemandsAttention;
        free(reply);
    } else {
#ifdef NET_WM_STATE_DEBUG
        printf("getting net wm state (%x), empty\n", m_window);
#endif
    }

    return result;
}

bool SConnection::pushEvent(xcb_generic_event_t *event){
    uint8_t event_code = event->response_type & 0x7f;
    Msg *pMsg = nullptr;
    bool ret = false;
    switch (event_code)
    {
    case XCB_KEY_PRESS:
    {
        //todo: impl auto repeat logic
        xcb_key_press_event_t* e2 = (xcb_key_press_event_t*)event;
        pMsg = new Msg;
        pMsg->hwnd = e2->child == XCB_NONE ? e2->event : e2->child;
        UINT vk = m_keyboard->translateVk(e2->detail, e2->state, e2->time);
        pMsg->message = vk< VK_NUMLOCK? WM_KEYDOWN:WM_SYSKEYDOWN;
        pMsg->wParam = vk;
        BYTE scanCode = (BYTE)e2->detail;
        pMsg->lParam = scanCode<<16;
        printf("onkeydown, detail=%c,vk=%d\n", e2->detail,vk);
        break;
    }
    case XCB_KEY_RELEASE:
    {
        xcb_key_release_event_t* e2 = (xcb_key_release_event_t*)event;
        pMsg = new Msg;
        pMsg->hwnd = e2->child == XCB_NONE ? e2->event : e2->child;
        UINT vk = m_keyboard->translateVk(e2->detail, e2->state, e2->time);
        pMsg->message = vk < VK_NUMLOCK ? WM_KEYUP : WM_SYSKEYUP;
        pMsg->wParam = vk;
        BYTE scanCode = (BYTE)e2->detail;
        pMsg->lParam = scanCode << 16;
        printf("onkeyup, detail=%c,vk=%d\n", e2->detail, vk);
        break;
    }
    case XCB_EXPOSE:
    {
        xcb_expose_event_t* expose = (xcb_expose_event_t*)event;
        RECT rc = {expose->x,expose->y,expose->x+expose->width,expose->y+expose->height};
        HRGN hrgn = CreateRectRgnIndirect(&rc);
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
                if((*it)->message == WM_PAINT && (*it)->hwnd == expose->window)
                {
                    MsgPaint *oldMsg = (MsgPaint*)(*it);
                    RECT rcOld,rcNew;
                    GetRgnBox(oldMsg->rgn, &rcOld);
                    CombineRgn(oldMsg->rgn, oldMsg->rgn, hrgn, RGN_OR);
                    GetRgnBox(oldMsg->rgn, &rcNew);
                    DeleteObject(hrgn);
                    return true;
                }
        }
        MsgPaint *pMsgPaint = new MsgPaint(hrgn);
        pMsgPaint->hwnd = expose->window;
        pMsgPaint->message = WM_PAINT;
        pMsgPaint->wParam = 0;
        pMsgPaint->lParam = (LPARAM)pMsgPaint->rgn;
        pMsg = pMsgPaint;
        break;
    }
    case XCB_PROPERTY_NOTIFY:
    {
        xcb_property_notify_event_t* e2 = (xcb_property_notify_event_t*)event;
        if (e2->atom == _NET_WM_STATE_ATOM || e2->atom == WM_STATE_ATOM) {
            uint32_t newState = -1;
            if (e2->atom == WM_STATE_ATOM) {
                const xcb_get_property_cookie_t get_cookie = xcb_get_property(connection, 0, e2->window, WM_STATE_ATOM, XCB_ATOM_ANY, 0, 1024);
                xcb_get_property_reply_t * reply = xcb_get_property_reply(connection, get_cookie, nullptr);
                if (reply && reply->format == 32 && reply->type == WM_STATE_ATOM && reply->length!=0) {
                    const uint32_t* data = (const uint32_t*)xcb_get_property_value(reply);
                    if (data[0] == XCB_ICCCM_WM_STATE_ICONIC/* || data[0]==XCB_ICCCM_WM_STATE_WITHDRAWN*/) {
                        newState = SIZE_MINIMIZED;                       
                    }
                    else if (data[0] == XCB_ICCCM_WM_STATE_NORMAL) {
                        newState = SIZE_RESTORED;
                    }
                }
                free(reply);
            }
            if (newState != SIZE_MINIMIZED) {
                uint32_t state = netWmStates(e2->window);
                printf("state changed, state=0x%08x\n",state);
                if((state & (NetWmStateMaximizedHorz |NetWmStateMaximizedVert) )== (NetWmStateMaximizedHorz |NetWmStateMaximizedVert)){
                    newState = SIZE_MAXIMIZED;
                }else if((state & (NetWmStateMaximizedHorz |NetWmStateMaximizedVert) )== 0){
                    newState = SIZE_RESTORED;
                }
            }
            if (newState != -1) {
                pMsg = new Msg;
                pMsg->hwnd = e2->window;
                pMsg->message = UM_STATE;
                pMsg->wParam = newState;
            }
        }
        break;
    }
    case XCB_CONFIGURE_NOTIFY:
    {
        xcb_configure_notify_event_t *e2 = (xcb_configure_notify_event_t*)event;
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
                if((*it)->message == WM_MOVE && (*it)->hwnd == e2->window)
                {
                    m_msgQueue.erase(it);
                    break;
                }
        }
        for(auto it = m_msgQueue.begin();it!=m_msgQueue.end();it++){
                if((*it)->message == WM_SIZE && (*it)->hwnd == e2->window)
                {
                    m_msgQueue.erase(it);
                    break;
                }
        }
        pMsg = new Msg;
        pMsg->hwnd = e2->window;
        pMsg->message = WM_MOVE;
        pMsg->wParam = 0;
        pMsg->lParam = MAKELPARAM(e2->x,e2->y);
        m_msgQueue.push_back(pMsg);
        pMsg = new Msg;
        pMsg->hwnd = e2->window;
        pMsg->message = WM_SIZE;
        pMsg->wParam = 0;
        pMsg->lParam = MAKELPARAM(e2->width,e2->height);
        m_msgQueue.push_back(pMsg);
        pMsg = nullptr;
        break;
    }
    case XCB_CLIENT_MESSAGE:
    {
        xcb_client_message_event_t *client_message_event = (xcb_client_message_event_t *) event;
        if(client_message_event->data.data32[0] == WM_DELETE_WINDOW_ATOM){
            pMsg = new Msg;
            pMsg->message = WM_CLOSE;
            pMsg->hwnd = client_message_event->window;
            pMsg->wParam = pMsg->lParam = 0;
        }else if(client_message_event->type == _NET_WM_STATE_HIDDEN_ATOM){
            pMsg = new Msg;
            pMsg->message = WM_SHOWWINDOW;
            pMsg->hwnd =client_message_event->window;
            pMsg->wParam = client_message_event->data.data32[0];
            pMsg->lParam = 0;
        }
        else if(client_message_event->type == _NET_WM_STATE_DEMANDS_ATTENTION_ATOM){
            pMsg = new Msg;
            pMsg->message = WM_ENABLE;
            pMsg->hwnd =client_message_event->window;
            pMsg->wParam = client_message_event->data.data32[0];
            pMsg->lParam = 0;
        }
        break;
    }
    break;
    case XCB_BUTTON_PRESS:
    {
        xcb_button_press_event_t* e2 = (xcb_button_press_event_t*)event;
        if (e2->detail >= XCB_BUTTON_INDEX_1 && e2->detail <= XCB_BUTTON_INDEX_3) {
            pMsg = new Msg;
            pMsg->hwnd = e2->event;
            pMsg->pt.x = e2->event_x;
            pMsg->pt.y = e2->event_y;
            pMsg->lParam = MAKELPARAM(pMsg->pt.x, pMsg->pt.y);
            switch (e2->detail) {
            case XCB_BUTTON_INDEX_1://left button
                pMsg->message = TsSpan(e2->time, m_tsPrevPress) > m_tsDoubleSpan ? WM_LBUTTONDOWN : WM_LBUTTONDBLCLK;
                break;
            case XCB_BUTTON_INDEX_2:
                pMsg->message = TsSpan(e2->time, m_tsPrevPress) < m_tsDoubleSpan ? WM_RBUTTONDOWN : WM_RBUTTONDBLCLK;
                break;
            case XCB_BUTTON_INDEX_3:
                pMsg->message = TsSpan(e2->time, m_tsPrevPress) < m_tsDoubleSpan ? WM_MBUTTONDOWN : WM_MBUTTONDBLCLK;
                break;
            }
            pMsg->wParam = ButtonState2Mask(e2->state);
            m_tsPrevPress = e2->time;
//            printf("on button down, msg=%d,pt=(%d,%d)\n", pMsg->message, pMsg->pt.x, pMsg->pt.y);
        }
        break;
    }
    case XCB_BUTTON_RELEASE:
    {
        xcb_button_release_event_t* e2 = (xcb_button_release_event_t*)event;
        if (e2->detail >= XCB_BUTTON_INDEX_1 && e2->detail <= XCB_BUTTON_INDEX_3) {
            pMsg = new Msg;
            pMsg->hwnd = e2->event;
            pMsg->pt.x = e2->event_x;
            pMsg->pt.y = e2->event_y;
            pMsg->lParam = MAKELPARAM(pMsg->pt.x, pMsg->pt.y);
            switch (e2->detail) {
            case XCB_BUTTON_INDEX_1://left button
                pMsg->message = WM_LBUTTONUP;
                break;
            case XCB_BUTTON_INDEX_2:
                pMsg->message = WM_RBUTTONUP;
                break;
            case XCB_BUTTON_INDEX_3:
                pMsg->message = WM_MBUTTONUP;
                break;
            }
            pMsg->wParam = ButtonState2Mask(e2->state);
//            printf("on button up!!, msg=%d,pt=(%d,%d)\n", pMsg->message, pMsg->pt.x, pMsg->pt.y);
        }
        break;
    }
    case XCB_MOTION_NOTIFY:
    {
        xcb_motion_notify_event_t* e2 = (xcb_motion_notify_event_t*)event;
        pMsg = new Msg;
        pMsg->hwnd = e2->event;
        pMsg->message = WM_MOUSEMOVE;
        pMsg->pt.x = e2->event_x;
        pMsg->pt.y = e2->event_y;
        pMsg->lParam = MAKELPARAM(pMsg->pt.x, pMsg->pt.y);
        pMsg->wParam = ButtonState2Mask(e2->state);
        pMsg->time = e2->time;
//        printf("XCB_MOTION_NOTIFY!!, msg=%d,pt=(%d,%d)\n", pMsg->message, pMsg->pt.x, pMsg->pt.y);
        break;
    }
    default:
        break;
    }
    if(pMsg){
        std::unique_lock<std::recursive_mutex> lock(m_mutex4Msg);
        m_msgQueue.push_back(pMsg);
    }
    return ret;
}

void* SConnection::readProc(void *p)
{
    SConnection * _this = static_cast<SConnection*>(p);
    _this->_readProc();
    return p;
}

void SConnection::_readProc()
{
    while (!m_bQuit){
        xcb_generic_event_t * event = xcb_wait_for_event(connection);
        if(!event){
            m_bQuit = true;
            break;
        }
        {
            std::unique_lock<std::mutex> lock(m_mutex4Evt);
            m_evtQueue.push_back(event);
            m_varCondition.notify_one();
        }
    }

    m_mutex4Evt.lock();
    for (auto it : m_evtQueue)
    {
        free(it);
    }
    m_evtQueue.clear();
    m_mutex4Evt.unlock();
}

SNSEND
