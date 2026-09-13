/**
 * MessageBeep：系统提示音。
 *
 * 平台实现（同名同签名 swinx_messageBeep，由链接器按平台目录解析，见 swinx/src/SwinxUtils.h）：
 *   Linux  = X11 核心协议 Bell 请求（xcb_bell）
 *   macOS  = NSBeep()
 *   iOS    = AudioServicesPlayAlertSound
 *   移动端 = 转发宿主应用的 g_platformAPI.audio.messageBeep 回调（Android 用
 *            ToneGenerator/RingtoneManager、OHOS 用 AudioRenderer 合成 PCM）；
 *            宿主应用未注册回调时返回 FALSE
 *
 * 与 test_multimon.cpp 的多显示器用例同理：提示音"是否真的响"取决于运行环境
 * （有无声卡、有无 X 连接、X server 的 bell 设置 xset b），因此这里不做
 * "一定成功"的断言，只锁定两件事：
 *   1. 调用安全，且返回值是合法的 BOOL——这条同时回归"跨 TU 的 int/BOOL ABI"：
 *      helper 声明在 swinx/src/SwinxUtils.h（返回 int）、实现在平台目录（.cpp/.mm），
 *      两侧返回类型若不一致，这里会读到非 0/1 的垃圾值；
 *   2. 同一环境下多次调用结果稳定。
 * 若要人工确认听感：fun_test --gtest_filter=swinx_beep.*
 */
#include <gtest/gtest.h>
#include <windows.h>

// Win32 语义：0xFFFFFFFF(-1) 播放"简单提示音"，其余为 MB_ICON* 系列。
// swinx 各平台都只有一种系统提示音，没有与 MB_ICON* 一一对应的提示音集合，
// 因此全部统一映射到系统提示音。
static const UINT kBeepTypes[] = {
    0xFFFFFFFFu,        // -1: SimpleBeep
    MB_OK,              // 0x00000000
    MB_ICONHAND,        // 0x00000010
    MB_ICONQUESTION,    // 0x00000020
    MB_ICONEXCLAMATION, // 0x00000030
    MB_ICONASTERISK,    // 0x00000040
};

TEST(swinx_beep, returns_valid_bool)
{
    const int n = (int)(sizeof(kBeepTypes) / sizeof(kBeepTypes[0]));
    ASSERT_GT(n, 0);
    for (int i = 0; i < n; i++)
    {
        BOOL ret = MessageBeep(kBeepTypes[i]);
#ifdef _WIN32
        // 真机：非零表示已把声音交给系统；无音频会话时也可能返回 FALSE，
        // 故只验证"调用不崩、不阻塞"，不断言具体取值。
        (void)ret;
#else
        // swinx：实现返回 TRUE/FALSE，必须落在 BOOL 的合法取值内
        EXPECT_TRUE(ret == TRUE || ret == FALSE) << "uType=" << kBeepTypes[i];
#endif
    }
}

TEST(swinx_beep, repeated_calls_are_consistent)
{
    // 同一环境下结果应稳定：有显示连接时每次 TRUE、无连接时每次 FALSE。
    // 不应出现"首次能发声、之后失败"（例如连接或请求缓冲被错误复用）。
    BOOL first = MessageBeep(MB_ICONASTERISK);
    for (int i = 0; i < 3; i++)
        EXPECT_EQ(MessageBeep(MB_ICONASTERISK), first);
}
