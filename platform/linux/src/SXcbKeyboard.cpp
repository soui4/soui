#include "SXcbKeyboard.h"
#include "SConnection.h"
#include <debug.h>

#define qWarning WARN
#define qWarning0 WARN0


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

SXcbKeyboard::SXcbKeyboard(SConnection* conn)
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
    memset(m_byKeyboardState, 0, sizeof(m_byKeyboardState));
    m_key_symbols = xcb_key_symbols_alloc(m_conn->connection);
    updateModifiers();
    updateKeymap();
}

SXcbKeyboard::~SXcbKeyboard() {
    xkb_state_unref(xkb_state);
    xkb_keymap_unref(xkb_keymap);
    xkb_context_unref(xkb_context);
    xkb_keymap_unref(latin_keymap);

    xcb_key_symbols_free(m_key_symbols);
}

xcb_connection_t* SXcbKeyboard::xcb_connection() {
    return m_conn->connection;
}

void SXcbKeyboard::readXKBConfig()
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

void SXcbKeyboard::clearXKBConfig()
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

void SXcbKeyboard::updateKeymap() {
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

void SXcbKeyboard::checkForLatinLayout()
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

void SXcbKeyboard::updateXKBMods()
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

uint32_t SXcbKeyboard::xkbModMask(uint16_t state)
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

void SXcbKeyboard::updateXKBStateFromState(struct xkb_state* kb_state, uint16_t state)
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

uint32_t SXcbKeyboard::translateModifiers(int s) const
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

int SXcbKeyboard::lookVkMap(xcb_keysym_t key) const
{
    for (int i = 0; i < ARRAYSIZE(KeyTbl); i++) {
        if (key == KeyTbl[i].scanCode)
            return KeyTbl[i].vk;
    }
    return 0;
}

int SXcbKeyboard::scanCodeToVk(xcb_keysym_t keysym, UINT modifiers) const
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

uint32_t SXcbKeyboard::onKeyEvent(bool bPress, xcb_keycode_t code, uint16_t state, xcb_timestamp_t ts) {
    xkb_state_update_key(xkb_state, code, bPress ? XKB_KEY_DOWN : XKB_KEY_UP);
    updateXKBStateFromState(xkb_state, state);
    xcb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state, code);
    uint32_t modifiers = translateModifiers(state);
    uint32_t vk = scanCodeToVk(sym, modifiers);
    //todo: handle auto repeat
    //todo:hjx, keyboard state is recorded by myself, it may loss state before we started.
    if (bPress)
        m_byKeyboardState[vk] = 0x80;
    else
        m_byKeyboardState[vk] = 0x00;
    if (vk == VK_SHIFT) {
        printf("shift value=0x%02x\n", m_byKeyboardState[VK_SHIFT]);
    }
    return vk;
}

void SXcbKeyboard::updateModifiers()
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

void SXcbKeyboard::resolveMaskConflicts()
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
