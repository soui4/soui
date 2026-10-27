/**
 * swinx 的公共句柄形状必须与 Windows SDK 一致：通用句柄 HANDLE 和 GDI 句柄
 * HGDIOBJ 都是 void*，具体 GDI 句柄（HFONT/HBITMAP/...）才是各自独立的结构体
 * 指针。
 *
 * 这不是纯编译期细节：ScriptModule-LUA 的 lua_tinker 按 HANDLE 做模板特化。
 * 一旦 HANDLE 退回 struct _Handle*，HGDIOBJ 与 HANDLE 就成了两个不同类型，
 * 非 Windows 平台必须再补一份 HGDIOBJ 的 read/push 特化才能编译（本次改动删除
 * 的正是那两块）。这里把形状钉住，改动重演时至少有一条用例能指出原因。
 */
#include <gtest/gtest.h>
#include <windows.h>

namespace
{

template <typename A, typename B>
struct IsSameType
{
    enum
    {
        value = 0
    };
};

template <typename A>
struct IsSameType<A, A>
{
    enum
    {
        value = 1
    };
};

} // namespace

TEST(swinx_handle_type, handle_and_hgdiobj_are_void_pointers)
{
    const bool handleIsVoid = IsSameType<HANDLE, void *>::value != 0;
    const bool hgdiobjIsVoid = IsSameType<HGDIOBJ, void *>::value != 0;
    const bool handleIsHgdiobj = IsSameType<HANDLE, HGDIOBJ>::value != 0;
    EXPECT_TRUE(handleIsVoid);
    EXPECT_TRUE(hgdiobjIsVoid);
    EXPECT_TRUE(handleIsHgdiobj);

    // lua_tinker 依赖的正是这种免强转互赋；两个类型不同时这里直接编译失败。
    HGDIOBJ obj = NULL;
    HANDLE h = obj;
    EXPECT_TRUE(h == NULL);
}
