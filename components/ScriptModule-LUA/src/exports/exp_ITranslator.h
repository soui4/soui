#ifndef __EXP_ITRANSLATOR_H__
#define __EXP_ITRANSLATOR_H__

#include <interface/STranslator-i.h>
#include "toobj.h"

// tr(src, ctx) -> utf8 string
static int L_ITranslator_tr(lua_State *L)
{
    ITranslator *_this = lua_tinker_toobj<ITranslator>(L, 1);
    if (!_this)
        return luaL_error(L, "expect ITranslator at arg 1");
    const char *src = luaL_checkstring(L, 2);
    const char *ctx = (const char *)lua_tostring(L, 3);
    SStringW strSrc = S_CA2W(src, CP_UTF8);
    SStringW strCtx = ctx ? S_CA2W(ctx, CP_UTF8) : SStringW();
    int nLen = _this->tr(&strSrc, &strCtx, NULL, 0);
    if (nLen <= 0)
    {
        lua_pushstring(L, "");
        return 1;
    }
    std::vector<wchar_t> buf(nLen + 1);
    _this->tr(&strSrc, &strCtx, &buf[0], nLen + 1);
    SStringA strOut = S_CW2A(&buf[0], CP_UTF8);
    lua_pushstring(L, strOut.c_str());
    return 1;
}

static int L_ITranslator_GetName(lua_State *L)
{
    ITranslator *_this = lua_tinker_toobj<ITranslator>(L, 1);
    if (!_this)
        return luaL_error(L, "expect ITranslator at arg 1");
    SStringA strName;
    _this->GetNameA(&strName);
    lua_pushstring(L, strName.c_str());
    return 1;
}

static int L_ITranslator_NameEqual(lua_State *L)
{
    ITranslator *_this = lua_tinker_toobj<ITranslator>(L, 1);
    if (!_this)
        return luaL_error(L, "expect ITranslator at arg 1");
    const char *name = luaL_checkstring(L, 2);
    SStringA strName(name);
    lua_pushboolean(L, _this->NameEqualA(&strName));
    return 1;
}

static int L_ITranslator_getFontInfo(lua_State *L)
{
    ITranslator *_this = lua_tinker_toobj<ITranslator>(L, 1);
    if (!_this)
        return luaL_error(L, "expect ITranslator at arg 1");
    SStringW strFont;
    _this->getFontInfo(&strFont);
    SStringA strOut = S_CW2A(strFont.c_str(), CP_UTF8);
    lua_pushstring(L, strOut.c_str());
    return 1;
}

static void ExpLua_ITranslator_Inner(lua_State *L)
{
    lua_tinker::class_add<ITranslator>(L, "ITranslator");
    lua_tinker::class_inh<ITranslator, IObjRef>(L);

    // ITranslator::Load needs a binary translation file body, use
    // ITranslatorMgr + STranslator-based loaders from the host app instead.
    class_set_cfun<ITranslator>(L, "tr", L_ITranslator_tr);
    class_set_cfun<ITranslator>(L, "GetName", L_ITranslator_GetName);
    class_set_cfun<ITranslator>(L, "NameEqual", L_ITranslator_NameEqual);
    class_set_cfun<ITranslator>(L, "getFontInfo", L_ITranslator_getFontInfo);

    DEF_CAST_OBJREF(L, ITranslator);
}

// ---------------------------------------------------------------------------
// ITranslatorMgr
// ---------------------------------------------------------------------------
static int L_ITranslatorMgr_CreateTranslator(lua_State *L)
{
    ITranslatorMgr *_this = lua_tinker_toobj<ITranslatorMgr>(L, 1);
    if (!_this)
        return luaL_error(L, "expect ITranslatorMgr at arg 1");
    ITranslator *pTranslator = NULL;
    BOOL bRet = _this->CreateTranslator(&pTranslator);
    if (!bRet || !pTranslator)
    {
        lua_pushnil(L);
        return 1;
    }
    lua_tinker::push<ITranslator *>(L, pTranslator);
    return 1;
}

static int L_ITranslatorMgr_GetLanguage(lua_State *L)
{
    ITranslatorMgr *_this = lua_tinker_toobj<ITranslatorMgr>(L, 1);
    if (!_this)
        return luaL_error(L, "expect ITranslatorMgr at arg 1");
    SStringA strLang;
    _this->GetLanguageA(&strLang);
    lua_pushstring(L, strLang.c_str());
    return 1;
}

static int L_ITranslatorMgr_tr(lua_State *L)
{
    ITranslatorMgr *_this = lua_tinker_toobj<ITranslatorMgr>(L, 1);
    if (!_this)
        return luaL_error(L, "expect ITranslatorMgr at arg 1");
    const char *src = luaL_checkstring(L, 2);
    const char *ctx = (const char *)lua_tostring(L, 3);
    SStringW strSrc = S_CA2W(src, CP_UTF8);
    SStringW strCtx = ctx ? S_CA2W(ctx, CP_UTF8) : SStringW();
    int nLen = _this->tr(&strSrc, &strCtx, NULL, 0);
    if (nLen <= 0)
    {
        lua_pushstring(L, "");
        return 1;
    }
    std::vector<wchar_t> buf(nLen + 1);
    _this->tr(&strSrc, &strCtx, &buf[0], nLen + 1);
    SStringA strOut = S_CW2A(&buf[0], CP_UTF8);
    lua_pushstring(L, strOut.c_str());
    return 1;
}

static void ExpLua_ITranslatorMgr_Inner(lua_State *L)
{
    lua_tinker::class_add<ITranslatorMgr>(L, "ITranslatorMgr");
    lua_tinker::class_inh<ITranslatorMgr, IObjRef>(L);

    lua_tinker::class_def<ITranslatorMgr>(L, "IsValid", &ITranslatorMgr::IsValid);
    lua_tinker::class_def<ITranslatorMgr>(L, "SetLanguage", &ITranslatorMgr::SetLanguageA);
    lua_tinker::class_def<ITranslatorMgr>(L, "InstallTranslator", &ITranslatorMgr::InstallTranslator);

    class_set_cfun<ITranslatorMgr>(L, "GetLanguage", L_ITranslatorMgr_GetLanguage);
    class_set_cfun<ITranslatorMgr>(L, "CreateTranslator", L_ITranslatorMgr_CreateTranslator);
    class_set_cfun<ITranslatorMgr>(L, "tr", L_ITranslatorMgr_tr);

    DEF_CAST_OBJREF(L, ITranslatorMgr);
}

#endif // __EXP_ITRANSLATOR_H__
